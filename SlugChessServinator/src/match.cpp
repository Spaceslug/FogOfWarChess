#include "match.h"
#include "messenger.h"
#include "usermanager.h"
#include "filesystem.h"

bool Match::DoMove(const std::string& usertoken, std::shared_ptr<chesscom::Move> move) 
{
    //std::cout  << usertoken << " iswhite: " << std::to_string(usertoken == _whitePlayer) << " iswhitesmove" << std::to_string(IsWhitesMove())  << std::endl << std::flush;
    if((IsWhitesMove() && _whitePlayer == usertoken)  
        || ((!IsWhitesMove()) && _blackPlayer == usertoken))
    {
        std::unique_lock<std::mutex> scopeLock (_mutex);
        if(!game->LegalMove(move->from(), move->to())){ 
            std::cout  << usertoken << " ERROR: tried a move that is not possible" << std::endl << std::flush;
            return false;
        }
        game->DoMove(move->from(), move->to());
        chesscom::MatchEvent expectedmatch_event = chesscom::MatchEvent::Non;
        if(game->Result() != SlugChess::EndResult::StillPlaying){
            switch (game->Result())
            {
            case SlugChess::EndResult::Draw:
                expectedmatch_event = chesscom::MatchEvent::Draw;
                break;
            case SlugChess::EndResult::WhiteWin:
                expectedmatch_event = chesscom::MatchEvent::WhiteWin;
                break;
            case SlugChess::EndResult::BlackWin:
                expectedmatch_event = chesscom::MatchEvent::BlackWin;
                break;
            default:
                break;
            }
        }
        if(moves.size() != 0){ //Avoid changing time first move
            int* secToChange = IsWhitesMove()? &clock->whiteSecLeft: &clock->blackSecLeft;
            *secToChange -= move->sec_spent();
            if(*secToChange <= 0){
                expectedmatch_event = IsWhitesMove()?chesscom::MatchEvent::BlackWin:chesscom::MatchEvent::WhiteWin;
            }
            else {
                *secToChange += clock->secsPerMove;
            }
            
        }
        else
        {
            clock->is_ticking = true;
        }
        
        matchEvents.push_back(expectedmatch_event);
        moves.push_back(move);
        cv.notify_all();
        //TORM newSystem
        nl_MatchEventAll(expectedmatch_event, true);
        if(expectedmatch_event != chesscom::Non) nl_MatchFinished();
        
        return true;
    }
    else
    {
        std::cout  <<  usertoken << " ERROR: not this players turn to move" << std::endl << std::flush;
        return false;
    }
}

void Match::PlayerDisconnected(const std::string& usertoken, chesscom::MatchEvent matchEventType)
{
    SendMessageAllPlayers(UserManager::Get()->GetUserName(usertoken) + " left the game");

    std::unique_lock<std::mutex> scopeLock (_mutex);
    matchEvents.push_back(matchEventType);
    cv.notify_all();
    //TORM newSystem
    if(matchEventType == chesscom::MatchEvent::WhiteWin || matchEventType == chesscom::MatchEvent::BlackWin ||
     matchEventType == chesscom::MatchEvent::Draw )
     {
         nl_MatchEventAll(matchEventType, false);
         nl_MatchFinished();
     }
}

void Match::PlayerListenerJoin(const std::string& usertoken, std::shared_ptr<MoveResultStream> resultStream )
{
    if(usertoken == _whitePlayer || usertoken == _blackPlayer)
    {
        //Allready have token and type
        std::unique_lock<std::mutex> scopeLock (_mutex);
        _players.at(usertoken).resultStream = resultStream;
    }
    else
    {
        //Is observer
        std::unique_lock<std::mutex> scopeLock (_mutex);
        _players[usertoken].usertoken = usertoken;
        _players[usertoken].type = PlayerTypes::Observer;
        _players[usertoken].resultStream = resultStream;
    }
}

void Match::PlayerListenerDisconnected(const std::string& usertoken)
{
    std::unique_lock<std::mutex> scopeLock (_mutex);
    if(_players.count(usertoken) > 0)
    {
        _players.at(usertoken).resultStream = nullptr;
    }
}

void Match::PlayerAskingForDraw(const std::string& usertoken)
{
    std::unique_lock<std::mutex> scopeLock (_mutex);
    std::string playerUsertoken, opponentUsertoken;
    if(_whitePlayer == usertoken){
        if(!IsWhitesMove())return;
        playerUsertoken = usertoken;
        opponentUsertoken = _blackPlayer;
    }else if(_blackPlayer == usertoken){
        if(IsWhitesMove())return;
        playerUsertoken = usertoken;
        opponentUsertoken = _whitePlayer;
    }else{
        std::cout  <<  usertoken << " ERROR: asked for draw but is not a player" << std::endl << std::flush;
        return;
    }
    if(_players.at(opponentUsertoken).SecSinceAskedForDraw() < TIMEOUT_FOR_DRAW)
    {
        //Opponent allready asked
        matchEvents.push_back(chesscom::MatchEvent::Draw);
        cv.notify_all();
        //TORM newSystem
        nl_MatchEventAll(chesscom::MatchEvent::Draw, false);
        nl_MatchFinished();
        std::cout  <<  usertoken << " op allready asking for draw. SecSinceAsked " << std::to_string(_players[opponentUsertoken].SecSinceAskedForDraw()) << std::endl << std::flush;
    }
    else if(_players.at(playerUsertoken).SecSinceAskedForDraw() > TIMEOUT_FOR_DRAW)
    {
        _players.at(playerUsertoken).askingForDrawTimstamp = std::chrono::system_clock::now();
        matchEvents.push_back(chesscom::MatchEvent::AskingForDraw);
        cv.notify_all();
        //TORM newSystem
        nl_MatchEventAskingForDraw(chesscom::MatchEvent::AskingForDraw, opponentUsertoken);
        std::cout  <<  usertoken << " asking for draw" << std::endl << std::flush;
        _players.at(playerUsertoken).SecSinceAskedForDraw();
    }else{
        std::cout  <<  usertoken << " allready asked for draw" << std::endl << std::flush;

    }
}

void Match::PlayerAcceptingDraw(const std::string& usertoken)
{
    std::unique_lock<std::mutex> scopeLock (_mutex);
    std::string playerUsertoken, opponentUsertoken;
    if(_whitePlayer == usertoken){
        playerUsertoken = usertoken;
        opponentUsertoken = _blackPlayer;
    }else if(_blackPlayer == usertoken){
        playerUsertoken = usertoken;
        opponentUsertoken = _whitePlayer;
    }else{
        std::cout  <<  usertoken << " ERROR: accepting draw but is not a player" << std::endl << std::flush;
        return;
    }
    if(_players.at(opponentUsertoken).SecSinceAskedForDraw() < TIMEOUT_FOR_DRAW + EXTRATIME_FOR_DRAW)
    {
        matchEvents.push_back(chesscom::MatchEvent::Draw);
        cv.notify_all();
        //TORM newSystem
        nl_MatchEventAll(chesscom::MatchEvent::Draw, false);
        nl_MatchFinished();
        std::cout  <<  usertoken << " accepted draw for draw" << std::endl << std::flush;
    }else{
        std::cout <<  usertoken << "acept draw failed. To many Sec since opp asked draw: " << std::to_string(_players.at(opponentUsertoken).SecSinceAskedForDraw()) << std::endl << std::flush;
    }
}
/**
 *  ttime is official end of match time 
 */
std::string Match::GetPgnString(time_t& ttime)
{
    std::stringstream ss;
    ss << "[Event \"Custom SlugChess game\"]" << std::endl; //TODO: Games can not be custom when ladder is up
    ss << "[Site \"REDACTED, REDACTED NOR\"]" << std::endl;
    tm *local_time = gmtime(&ttime);
    ss << "[Date \""<< 1900 + local_time->tm_year << "." 
        << 1 + local_time->tm_mon << "." << local_time->tm_mday << "\"]" << std::endl;
    ss << "[Round \"1\"]" << std::endl;
    ss << "[White \"" << UserManager::Get()->GetUserName(_whitePlayer) << "\"]" << std::endl;
    ss << "[Black \"" << UserManager::Get()->GetUserName(_blackPlayer) << "\"]" << std::endl;
    ss << "[Result \"" << game->ResultString() << "\"]" << std::endl;
    ss << "[Time \"" << local_time->tm_hour << ":" << local_time->tm_min << ":" << local_time->tm_sec << "\"]" << std::endl;
    ss << "[Mode \"ICS\"]" << std::endl;
    ss << "[FEN \"" << game->GetCurrentFenString() << "\"]" << std::endl;
    ss << "[SetUp \"1\"]" << std::endl;
    ss << "[Variant \"SlugChess.Torch\"]" << std::endl;
    ss << std::endl;
    game->PrintSanMoves(ss);
    return ss.str();
}

void Match::SendMessageAllPlayers(const std::string& message)
{
    std::unique_lock<std::mutex> scopeLock (_mutex);
    nl_SendMessageAllPlayers(message);
}
void Match::nl_SendMessageAllPlayers(const std::string& message)
{
    for(auto& [usertoken, player]  : _players)
    {
        if(player.resultStream != nullptr) //Indicates player is connected to match and wants to listen
        {
            Messenger::SendServerMessage(usertoken, message);
        }
    }
}

void Match::nl_MatchFinished()
{
    Messenger::Log(_matchToken + " match finished");
    std::stringstream ss;
    ss << "PGN of the game:" << std::endl;
    nl_SendMessageAllPlayers(_pgn);
    _matchFinished = true;
    matchDoneCV.notify_all();
}
void Match::nl_MatchEventAskingForDraw(chesscom::MatchEvent matchEvent, std::string& usertoken)
{
    chesscom::MoveResult mrPkt;
    mrPkt.set_move_happned(false);
    mrPkt.set_match_event(matchEvent);

    if(_players.count(usertoken) > 0)
    {
        if(_players.at(usertoken).resultStream != nullptr && _players.at(usertoken).resultStream->alive ) 
        {
            _players.at(usertoken).resultStream->streamPtr->Write(mrPkt);
        }
    }
}

void Match::nl_MatchEventAll(chesscom::MatchEvent matchEvent, bool moveHappened)
{
    Messenger::Log("MatchEvent " + std::to_string(matchEvent));
    //Messenger::Log(GetPgnString());
    chesscom::MoveResult mrPkt;

    chesscom::GameState whiteGS;
    chesscom::GameState blackGS;
    chesscom::GameState observerGS;
    if(moveHappened){
        mrPkt.set_move_happned(true);
        SlugChessConverter::SetGameState(game, &whiteGS, true);
        SlugChessConverter::SetGameState(game, &blackGS, false);
        //SlugChessConverter::SetGameState(game, &whiteGS, true); TODO implement oberserver gamestate
    }else{
        mrPkt.set_move_happned(false);
    }

    if(matchEvent != chesscom::Non){
        time_t endOfMatch = time(nullptr);
        _pgn = GetPgnString(endOfMatch);
        mrPkt.mutable_game_result()->set_png(_pgn);
        Filesystem::WriteMatchPgn(
            UserManager::Get()->GetUserName(_whitePlayer), 
            UserManager::Get()->GetUserName(_blackPlayer),
            _pgn,
            endOfMatch);
    }
    
    mrPkt.set_match_event(matchEvent);
    mrPkt.mutable_chess_clock()->set_white_seconds_left(clock->whiteSecLeft);
    mrPkt.mutable_chess_clock()->set_black_seconds_left(clock->blackSecLeft);
    mrPkt.mutable_chess_clock()->set_timer_ticking(clock->is_ticking);

    for(auto& [usertoken, player]  : _players)
    {
        (void)usertoken;//To suppress unused variable warning
        if(player.resultStream != nullptr && player.resultStream->alive) 
        {
            if(moveHappened) mrPkt.set_allocated_game_state(&(player.type == PlayerTypes::White?whiteGS:
                                            player.type == PlayerTypes::Black?blackGS:observerGS));
            player.resultStream->streamPtr->Write(mrPkt);
            if(moveHappened) mrPkt.release_game_state();
        }
    }

}