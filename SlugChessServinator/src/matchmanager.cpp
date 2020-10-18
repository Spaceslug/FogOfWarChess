#include "matchmanager.h"
#include "slugchesscoverter.h"
#include <random>
#include <exception>
#include <string>

MatchManager* MatchManager::_instance = 0;

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
    match->clock->is_ticking = false;
    _matches[matchToken] = match;
    std::cout << "  checing match " << " black sec left " << std::to_string(match->clock->blackSecLeft) << " white sec left " << std::to_string(match->clock->whiteSecLeft) << std::endl << std::flush;
    auto matPtr = _matches[matchToken];
    //std::cout << "  white player " <<  matPtr->getWhitePlayer() << std::endl << std::flush;
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
    match->clock->is_ticking = false;
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
        return std::shared_ptr<Match>(nullptr);
    }
}

void MatchManager::EraseMatch(const std::string& matchId)
{
    std::unique_lock<std::mutex> lk(_matchesMutex);
    _matches.erase(matchId);
}

void MatchManager::UserLoggedOut(const std::string& token, std::shared_ptr<chesscom::UserData> userData)
{
    std::unique_lock<std::mutex> lk(_matchesMutex);
    for(auto& [matchId, matchPtr] : _matches)
    {
        if(matchPtr->getWhitePlayer() == token)
        {
            std::cout << "Ended match " << matchId << " because " << token << " got logged out" << std::endl << std::flush;
            matchPtr->PlayerDisconnected(token, chesscom::MatchEvent::BlackWin);
        }
        else if(matchPtr->getBlackPlayer() == token)
        {
            std::cout << "Ended match " << matchId << " because " << token << " got logged out" << std::endl << std::flush;
            matchPtr->PlayerDisconnected(token, chesscom::MatchEvent::WhiteWin);
        }
    }
    
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
    //bool playerIsWhite = matchPtr->getWhitePlayer() == listenerUsertoken;
    PlayerTypes playerType = matchPtr->getPlayerType(listenerUsertoken);
    //bool loop = true;
    //uint lastEventNum = 0;
    std::mutex mutex;
    std::unique_lock<std::mutex> lk(mutex);
    //Sending init
    std::cout  << " Sending init gamestate to  " << listenerUsertoken << std::endl << std::flush;
    moveResultPkt.set_move_happned(false);
    SlugChessConverter::SetGameState(matchPtr->game, &state, playerType);
    moveResultPkt.set_allocated_game_state(&state);
    moveResultPkt.set_match_event(chesscom::MatchEvent::Non);
    moveResultPkt.mutable_chess_clock()->set_white_seconds_left(matchPtr->clock->whiteSecLeft);
    moveResultPkt.mutable_chess_clock()->set_black_seconds_left(matchPtr->clock->blackSecLeft);
    moveResultPkt.mutable_chess_clock()->set_timer_ticking(matchPtr->clock->is_ticking);
    writerPtr->Write(moveResultPkt);
    moveResultPkt.release_game_state();
    std::shared_ptr<MoveResultStream> resultStream = std::make_shared<MoveResultStream>();
    resultStream->streamPtr = writerPtr;
    resultStream->alive = true;
    matchPtr->PlayerListenerJoin(listenerUsertoken, resultStream);
    while(matchPtr->Ongoing() && !contextPtr->IsCancelled())
    {
        matchPtr->matchDoneCV.wait_for(lk, std::chrono::milliseconds(MAX_SLEEP_MS));
    }
    resultStream->alive = false;
    matchPtr->PlayerListenerDisconnected(listenerUsertoken);
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
        bool didMove = matchPtr->DoMove(usertoken, movePtr);
        if(didMove){
            //std::cout << "adding ChessMove2" << std::endl << std::flush;
            //std::cout << "FEN: " << matchPtr->game->GetCurrentFenString() << std::endl << std::flush;
        }
    }
        break;
    case chesscom::MatchEvent::UnexpectedClosing:
    {
        //std::cout << request->match_token() << " " <<  request->usertoken()<< " Got UnexpectedClosing" << std::endl << std::flush;
        if(matchPtr->getWhitePlayer() == usertoken)
        {
            matchPtr->PlayerDisconnected(usertoken, chesscom::MatchEvent::BlackWin);
        }
        else if(matchPtr->getBlackPlayer() == usertoken)
        {
            matchPtr->PlayerDisconnected(usertoken, chesscom::MatchEvent::WhiteWin);
        }else{
            matchPtr->PlayerDisconnected(usertoken, event);
        }
        
    }
        break;
    case chesscom::MatchEvent::ExpectedClosing:
    {
        if(matchPtr->getWhitePlayer() == usertoken)
        {
            matchPtr->PlayerDisconnected(usertoken, chesscom::MatchEvent::BlackWin);
        }
        else if(matchPtr->getBlackPlayer() == usertoken)
        {
            matchPtr->PlayerDisconnected(usertoken, chesscom::MatchEvent::WhiteWin);
        }else{
            matchPtr->PlayerDisconnected(usertoken, event);
        }
    }
        break;
    case chesscom::MatchEvent::AskingForDraw:
    {
        matchPtr->PlayerAskingForDraw(usertoken);
    }
        break;
    case chesscom::MatchEvent::AcceptingDraw:
    {
        matchPtr->PlayerAcceptingDraw(usertoken);
    }
        break;
    default:
        throw "fuck fwafdw";
        break;
    }
}
