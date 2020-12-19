#include "match.h"
#include "usermanager.h"
#include "filesystem.h"
#include <algorithm>

Match::Match(const std::string& token, const std::string& whitePlayerToken, const std::string& blackPlayerToken, const std::string& fenString, const std::string& ruleType, VisionRules& visionRules)
{
    _matchToken = token;
    _whitePlayer = whitePlayerToken;
    _blackPlayer = blackPlayerToken;
    _players[whitePlayerToken].usertoken = whitePlayerToken;
    _players[whitePlayerToken].type = PlayerTypes::White;
    _players[whitePlayerToken].askingForDrawTimstamp = std::chrono::system_clock::now();
    _players[blackPlayerToken].usertoken = blackPlayerToken;
    _players[blackPlayerToken].type = PlayerTypes::Black; 
    _players[blackPlayerToken].askingForDrawTimstamp = std::chrono::system_clock::now();
    _ruleType = ruleType;
    clock = std::make_shared<ChessClock>();
    game = std::make_shared<SlugChess>(fenString, visionRules);
    std::cout  << "Creating match: " << _matchToken << " white: " << whitePlayerToken << " black:" << blackPlayerToken  << std::endl << std::flush;
}

bool Match::DoMove(const std::string& usertoken, std::shared_ptr<chesscom::Move> move) 
{
    //std::cout  << usertoken << " iswhite: " << std::to_string(usertoken == _whitePlayer) << " iswhitesmove" << std::to_string(IsWhitesMove())  << std::endl << std::flush;
    if(_matchFinished){ 
        Messenger::Log(usertoken + " ERROR: tried a move when match is finished");
        return false;
    }
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
        if(expectedmatch_event != chesscom::Non) nl_MatchCompleted(expectedmatch_event);
        nl_MatchEventAll(expectedmatch_event, true);
        if(expectedmatch_event != chesscom::Non) nl_TerminateMatch();
        
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
         nl_MatchCompleted(matchEventType);
         nl_MatchEventAll(matchEventType, false);
         nl_TerminateMatch();
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
        nl_MatchCompleted(chesscom::MatchEvent::Draw);
        nl_MatchEventAll(chesscom::MatchEvent::Draw, false);
        nl_TerminateMatch();
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
        nl_MatchCompleted(chesscom::MatchEvent::Draw);
        nl_MatchEventAll(chesscom::MatchEvent::Draw, false);
        nl_TerminateMatch();
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
    ss << "[Date \""<< 1900 + local_time->tm_year << "." << std::setw(2) << std::setfill('0')
        << 1 + local_time->tm_mon << "." << local_time->tm_mday << "\"]" << std::setfill(' ') << std::endl;
    ss << "[Round \"1\"]" << std::endl;
    ss << "[White \"" << UserManager::Get()->GetUserName(_whitePlayer) << "\"]" << std::endl;
    ss << "[Black \"" << UserManager::Get()->GetUserName(_blackPlayer) << "\"]" << std::endl;
    ss << "[Result \"" << game->ResultString() << "\"]" << std::endl;
    ss << "[Time \"" << local_time->tm_hour << ":" << local_time->tm_min << ":" << local_time->tm_sec << "\"]" << std::endl;
    ss << "[Mode \"ICS\"]" << std::endl;
    ss << "[FEN \"" << game->GetFenString() << "\"]" << std::endl;
    ss << "[SetUp \"1\"]" << std::endl;
    if(_ruleType != "custom")
    {
        ss << "[Variant \"SlugChess." << _ruleType << "\"]" << std::endl;
    } else {
        ss << "[Variant \"SlugChess." << "custom." << "{}" << "\"]" << std::endl;
    }
    
    ss << std::endl;
    game->PrintSanMoves(ss);
    return ss.str();
}

/**
 *  ttime is official end of match time TODO: move to better location. Dossent fitt in match
 */
std::map<std::string, std::string> Match::ReadSlugChessPgnString(const std::string& pgn)
{
    std::map<std::string, std::string> map;
    std::istringstream istream(pgn);
    while (!istream.eof())
    {
        std::string line;
        std::getline(istream, line);
        if(istream.fail())return map;
        if(line[0] == '['){
            auto nameEndPos = line.find(' ', 1);
            auto startValue =  1 + line.find('"', nameEndPos);
            auto endValue =   line.find('"', startValue);
            map[line.substr(1, nameEndPos-1)] = line.substr(startValue, endValue-startValue);
        }
        else if(line[0] == '1')
        {
            std::string more_line;
            while (!istream.eof())
            {
                std::getline(istream, more_line);
                line = line + more_line;
            }
            map["San"] = line;            
        }
    }
    // for (auto& [name, value] : map){
    //         Messenger::Log( name + " " + value);
    // }
    return map;
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

void Match::nl_MatchCompleted(chesscom::MatchEvent result)
{
    time_t endOfMatch = time(nullptr);
    _pgn = GetPgnString(endOfMatch);
    Filesystem::WriteMatchPgn(
        UserManager::Get()->GetUserName(_whitePlayer), 
        UserManager::Get()->GetUserName(_blackPlayer),
        _pgn,
        endOfMatch);
    UserManager::Get()->UpdateElo(_whitePlayer, _blackPlayer, result);
    _matchFinished = true;
}

void Match::nl_TerminateMatch()
{
    Messenger::Log(_matchToken + " terminating match");
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
        SlugChessConverter::SetGameState(game, &whiteGS, PlayerTypes::White);
        SlugChessConverter::SetGameState(game, &blackGS, PlayerTypes::Black);
        SlugChessConverter::SetGameState(game, &whiteGS, PlayerTypes::Observer);
    }else{
        mrPkt.set_move_happned(false);
    }
    if(_matchFinished){
        mrPkt.mutable_game_result()->set_pgn(_pgn);

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