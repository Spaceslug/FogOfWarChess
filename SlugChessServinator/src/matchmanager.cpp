#include "matchmanager.h"
#include "slugchesscoverter.h"
#include <random>
#include <exception>

std::string MatchManager::CreateMatch(std::string& player1Token, std::string& player2Token)
{
    std::string matchToken = "match"+ std::to_string(_tokenCounter++);
    std::string white;
    std::string black; 
    std::tie(white, black) = RandomSort(player1Token, player2Token);
    std::shared_ptr<::Match> match = std::make_shared<::Match>(matchToken, white, black,	 
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w AHah - 0 1", defaultVisionRules);

    match->clock->blackSecLeft = defaultTimeRules.player_time().minutes() * 60 + defaultTimeRules.player_time().seconds();
    match->clock->whiteSecLeft = match->clock->blackSecLeft;
    match->clock->secsPerMove = defaultTimeRules.seconds_per_move();
    _matches[matchToken] = match;
    std::cout << "  checing match " << " black sec left " << std::to_string(match->clock->blackSecLeft) << " white sec left " << std::to_string(match->clock->whiteSecLeft) << std::endl << std::flush;
    auto matPtr = _matches[matchToken];
    std::cout << "  white player " <<  matPtr->whitePlayer << std::endl << std::flush;
    return matchToken;
}
std::string MatchManager::CreateMatch(chesscom::HostedGame& hostedGame)
{
    std::string matchToken = "match"+ std::to_string(_tokenCounter++);
    std::string white;
    std::string black;
    if(hostedGame.game_rules().side_type() == chesscom::SideType::HostIsWhite)
    {
        white = hostedGame.host().usertoken();black = hostedGame.joiner().usertoken();
    }
    else if(hostedGame.game_rules().side_type() == chesscom::SideType::HostIsBlack)
    {
        black = hostedGame.host().usertoken();white = hostedGame.joiner().usertoken();
    }
    else
    {
        std::tie(white, black) = RandomSort(hostedGame.host().usertoken(), hostedGame.joiner().usertoken());
    }
    std::string fenString;
    if(hostedGame.game_rules().chess_type() == chesscom::ChessType::Classic)
    {
        fenString = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w AHah - 0 1";
    }
    else if(hostedGame.game_rules().chess_type() == chesscom::ChessType::FisherRandom)
    {
        //throw std::exception("not implemten FisherRandom chess");
    }
    else if(hostedGame.game_rules().chess_type() == chesscom::ChessType::SlugRandom)
    {
        fenString = Sfen::GenSlugRandom();
    }
    
    VisionRules visionRules = FromChesscomVisionRules(hostedGame.game_rules().vision_rules());
    std::shared_ptr<::Match> match = std::make_shared<::Match>(matchToken, white, black, 
       fenString, visionRules);
    match->clock->blackSecLeft = hostedGame.game_rules().time_rules().player_time().minutes() * 60 + hostedGame.game_rules().time_rules().player_time().seconds();
    match->clock->whiteSecLeft = match->clock->blackSecLeft;
    match->clock->secsPerMove = hostedGame.game_rules().time_rules().seconds_per_move();
    {
        std::unique_lock<std::mutex> lk(_matchesMutex);
        _matches[matchToken] = match;
    }

    return matchToken;
}

std::shared_ptr<Match> MatchManager::GetMatch(const std::string& matchId)
{
    std::unique_lock<std::mutex> lk(_matchesMutex);
    if(_matches.count(matchId) > 0)
    {
        return _matches[matchId];
    }else{
        return std::shared_ptr<Match>();
    }
}

void MatchManager::EraseMatch(const std::string& matchId)
{
    std::unique_lock<std::mutex> lk(_matchesMutex);
    _matches.erase(matchId);
}

VisionRules MatchManager::FromChesscomVisionRules(const chesscom::VisionRules& chesscomVision)
{
    VisionRules visionRules;
    visionRules.enabled = chesscomVision.enabled();
    visionRules.globalRules.ViewMoveFields = chesscomVision.view_move_fields();
    visionRules.globalRules.ViewRange = chesscomVision.view_range();
    visionRules.globalRules.ViewCaptureField = chesscomVision.view_capture_field();
    //std::cout << " Vision rules" << std::endl << std::flush;
    for (auto&& piceRulesPar : chesscomVision.pice_overwriter()) {
        ChessPice pice = (ChessPice)piceRulesPar.first;
        visionRules.overWriteRules[pice].ViewRange = piceRulesPar.second.view_range();
        visionRules.overWriteRules[pice].ViewMoveFields = piceRulesPar.second.view_move_fields();
        visionRules.overWriteRules[pice].ViewCaptureField = piceRulesPar.second.view_capture_field();
    }

    return visionRules;
}

std::pair<std::string,std::string> MatchManager::RandomSort(const std::string& first,const std::string& second)
{
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(0, 1);
    if(dist(mt) == 1)
    {
        return {first, second};
    }
    else
    {
        return {second, first};
    }
}

void MatchManager::MatchListenLoop(
        std::string listenerUsertoken, 
        std::shared_ptr<Match> matchPtr,
        grpc::ServerContext* contextPtr,
        grpc::internal::WriterInterface<chesscom::MoveResult>* writerPtr)
{
    chesscom::MoveResult moveResultPkt;
    chesscom::GameState state;
    bool playerIsWhite = matchPtr->whitePlayer == listenerUsertoken;
    bool loop = true;
    uint lastEventNum = 0;
    std::unique_lock<std::mutex> lk(matchPtr->mutex);
    //Sending init
    std::cout  << " Sending init gamestate to  " << listenerUsertoken << std::endl << std::flush;
    moveResultPkt.set_move_happned(false);
    moveResultPkt.set_opponent_asking_for_draw(false);
    SlugChessConverter::SetGameState(matchPtr->game, &state, playerIsWhite);
    moveResultPkt.set_allocated_game_state(&state);
    moveResultPkt.set_match_event(chesscom::MatchEvent::Non);
    moveResultPkt.mutable_chess_clock()->set_white_seconds_left(matchPtr->clock->whiteSecLeft);
    moveResultPkt.mutable_chess_clock()->set_black_seconds_left(matchPtr->clock->blackSecLeft);
    writerPtr->Write(moveResultPkt);
    moveResultPkt.release_game_state();

    while (loop)
    {
        if(contextPtr->IsCancelled()) {
            return;
        }
        bool isUpdate;
        {
            isUpdate = matchPtr->matchEvents.size() > lastEventNum;
            if(isUpdate){
                if(matchPtr->matchEvents[lastEventNum] == chesscom::MatchEvent::UnexpectedClosing)
                {
                    std::cout  << matchPtr->matchToken << " " <<  listenerUsertoken << " Opponent UnexpectedClosing" << std::endl << std::flush;
                    moveResultPkt.set_move_happned(false);
                    moveResultPkt.set_opponent_asking_for_draw(false);
                    //moveResultPkt.set_allocated_move(matchPtr->moves.back().get());
                    moveResultPkt.set_match_event(chesscom::MatchEvent::UnexpectedClosing);
                    writerPtr->Write(moveResultPkt);
                    loop = false;
                    //continue;
                }
                else if(matchPtr->matchEvents[lastEventNum] == chesscom::MatchEvent::WhiteWin || matchPtr->matchEvents[lastEventNum] == chesscom::MatchEvent::BlackWin)
                {
                    std::cout  << " Going to send other move " << std::endl << std::flush;
                    std::cout  << matchPtr->matchToken << " " <<  listenerUsertoken << " Someone won!! " << matchPtr->matchEvents[lastEventNum] << std::endl << std::flush;
                    moveResultPkt.set_move_happned(true);
                    moveResultPkt.set_opponent_asking_for_draw(false);
                    SlugChessConverter::SetGameState(matchPtr->game, &state, playerIsWhite);
                    moveResultPkt.set_allocated_game_state(&state);
                    moveResultPkt.set_match_event(matchPtr->matchEvents[lastEventNum]);
                    moveResultPkt.mutable_chess_clock()->set_white_seconds_left(matchPtr->clock->whiteSecLeft);
                    moveResultPkt.mutable_chess_clock()->set_black_seconds_left(matchPtr->clock->blackSecLeft);

                    std::stringstream ss;
                    ss << "Moves of the game in SAN:" << std::endl;
                    matchPtr->game->PrintSanMoves(ss);
                    std::string message = ss.str();
                    //TODO this chat message to user should be handles somewhere else
                    //messenger.SendServerMessage(listenerUsertoken, message);
                    writerPtr->Write(moveResultPkt);
                    moveResultPkt.release_game_state();
                    loop = false;
                    continue;
                }
                else
                {
                    std::cout  << " Going to send move " << std::endl << std::flush;
                    moveResultPkt.set_move_happned(true);
                    moveResultPkt.set_opponent_asking_for_draw(false);
                    SlugChessConverter::SetGameState(matchPtr->game, &state, playerIsWhite);
                    moveResultPkt.set_allocated_game_state(&state);
                    moveResultPkt.set_match_event(chesscom::MatchEvent::Non);
                    moveResultPkt.mutable_chess_clock()->set_white_seconds_left(matchPtr->clock->whiteSecLeft);
                    moveResultPkt.mutable_chess_clock()->set_black_seconds_left(matchPtr->clock->blackSecLeft);
                    writerPtr->Write(moveResultPkt);
                    moveResultPkt.release_game_state();
                    std::cout  << matchPtr->matchToken << " " <<  listenerUsertoken << " SentMoveResult " << std::endl << std::flush;
                }
                lastEventNum++;
            }
        }
        matchPtr->cv.wait(lk);

    }
}

void MatchManager::DoMoveInMatch(
        chesscom::MatchEvent event,
        std::string usertoken, 
        std::shared_ptr<Match> matchPtr, 
        std::shared_ptr<chesscom::Move> movePtr)
{
    
    switch (event)
    {
    case chesscom::MatchEvent::Non:
    {
        //std::cout  << request->match_token() << " " <<  request->usertoken()<< " Got move " << request->move().from() << " " << request->move().to() << " secspent " << std::to_string(movePtr->sec_spent()) << std::endl << std::flush;
        //TODO fix time spent to somehow match timepoint. This is why sec_spent is sent separeatly from the move
        bool didMove = matchPtr->DoMove(usertoken, movePtr, movePtr->sec_spent());
        if(didMove){
            std::cout << "adding ChessMove2" << std::endl << std::flush;
        }
    }
        break;
    case chesscom::MatchEvent::UnexpectedClosing:
    {
        //std::cout << request->match_token() << " " <<  request->usertoken()<< " Got UnexpectedClosing" << std::endl << std::flush;
        matchPtr->PlayerDisconnected(usertoken);
    }
        break;
    case chesscom::MatchEvent::ExpectedClosing:
    {
        //std::cout << request->match_token() << " " <<  request->usertoken()<< " Got ExpectedClosing" << std::endl << std::flush;
        matchPtr->PlayerDisconnected(usertoken);
    }
        break;
    default:
        throw "fuck fwafdw";
        break;
    }
}