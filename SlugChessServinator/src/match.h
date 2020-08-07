#pragma once
#include <condition_variable>
#include <map>
#include <chrono>
#include <memory>
#include "messenger.h"
#include "slugchesscoverter.h"
#include "../../SlugChessCore/src/slugchess.h"

enum PlayerTypes{
    White,
    Black,
    Observer
};
struct MoveResultStream {
    bool alive;
    grpc::internal::WriterInterface<chesscom::MoveResult>* streamPtr;
};
struct ChessClock {
    int blackSecLeft;
    int whiteSecLeft;
    int secsPerMove;
    bool is_ticking;
    std::chrono::time_point<std::chrono::system_clock> lastMove; //TODO start using 
//    std::chrono::time_point<std::chrono::system_clock> lastBlackMove;
};
struct ChessMove {
    std::string from;
    std::string to;
};
struct Player
{
    PlayerTypes type;
    std::string usertoken;
    std::shared_ptr<MoveResultStream> resultStream;
    bool askingForDraw = false;
    std::chrono::time_point<std::chrono::system_clock> askingForDrawTimstamp;
    int64_t SecSinceAskedForDraw() { 
        //std::cout << usertoken << "Now: " << std::to_string(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()) << "Last: " << std::to_string(std::chrono::duration_cast<std::chrono::seconds>(askingForDrawTimstamp.time_since_epoch()).count()) << std::endl << std::flush;
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - askingForDrawTimstamp).count(); }
};
class Match {
public:
#define TIMEOUT_FOR_DRAW 5
#define EXTRATIME_FOR_DRAW 1
#define MOVE_TIME_DELAY_ALLOWED_MILLISEC 1000
    Match(const std::string& token, const std::string& whitePlayerToken, const std::string& blackPlayerToken, const std::string& fenString, VisionRules& visionRules)
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
        clock = std::make_shared<ChessClock>();
        game = std::make_shared<SlugChess>(fenString, visionRules);
        std::cout  << "Creating match: " << _matchToken << " white: " << whitePlayerToken << " black:" << blackPlayerToken  << std::endl << std::flush;
    }

    bool IsWhitesMove() { return moves.size()%2 == 0; }
    bool DoMove(const std::string& usertoken, std::shared_ptr<chesscom::Move> move) 
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
            nl_MatchEventAllWithMove(expectedmatch_event);
            if(expectedmatch_event != chesscom::Non) nl_MatchFinished();
            
            return true;
        }
        else
        {
            std::cout  <<  usertoken << " ERROR: not this players turn to move" << std::endl << std::flush;
            return false;
        }
    }

    void PlayerDisconnected(const std::string& usertoken, chesscom::MatchEvent matchEventType);
    void PlayerListenerJoin(const std::string& usertoken, std::shared_ptr<MoveResultStream> resultStream);
    void PlayerListenerDisconnected(const std::string& usertoken);

    void PlayerAskingForDraw(const std::string& usertoken)
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
            nl_MatchEventAll(chesscom::MatchEvent::Draw);
            nl_MatchFinished();
            std::cout  <<  usertoken << " op allready asking for draw. SecSinceAsked " << std::to_string(_players[opponentUsertoken].SecSinceAskedForDraw()) << std::endl << std::flush;
        }
        else if(_players.at(playerUsertoken).SecSinceAskedForDraw() > TIMEOUT_FOR_DRAW)
        {
            _players.at(playerUsertoken).askingForDrawTimstamp = std::chrono::system_clock::now();
            matchEvents.push_back(chesscom::MatchEvent::AskingForDraw);
            cv.notify_all();
            //TORM newSystem
            nl_MatchEvent(chesscom::MatchEvent::AskingForDraw, opponentUsertoken);
            std::cout  <<  usertoken << " asking for draw" << std::endl << std::flush;
            _players.at(playerUsertoken).SecSinceAskedForDraw();
        }else{
            std::cout  <<  usertoken << " allready asked for draw" << std::endl << std::flush;

        }
    }

    void PlayerAcceptingDraw(const std::string& usertoken)
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
            nl_MatchEventAll(chesscom::MatchEvent::Draw);
            nl_MatchFinished();
            std::cout  <<  usertoken << " accepted draw for draw" << std::endl << std::flush;
        }else{
            std::cout <<  usertoken << "acept draw failed. To many Sec since opp asked draw: " << std::to_string(_players.at(opponentUsertoken).SecSinceAskedForDraw()) << std::endl << std::flush;
        }
    }

    void SendMessageAllPlayers(const std::string& message);
    bool Ongoing(){ return !_matchFinished; }
	
    std::vector<chesscom::MatchEvent> matchEvents;

    std::vector<std::shared_ptr<chesscom::Move>> moves;
    std::string& getWhitePlayer(){return _whitePlayer;}
    std::string& getBlackPlayer(){return _blackPlayer;}
    std::string& getMatchToken(){return _matchToken;}
    std::shared_ptr<ChessClock> clock;
    std::shared_ptr<SlugChess> game;
    std::condition_variable cv;
    std::condition_variable matchDoneCV;
 

    private:
    bool _matchFinished = false;
    std::string _whitePlayer;
    std::string _blackPlayer;
    std::string _matchToken;
    std::map<std::string, Player> _players;
    std::mutex _mutex;

    void nl_SendMessageAllPlayers(const std::string& message);
    void nl_MatchFinished()
    {
        Messenger::Log(_matchToken + " match finished");
        std::stringstream ss;
        ss << "Moves of the game in SAN:" << std::endl;
        game->PrintSanMoves(ss);
        nl_SendMessageAllPlayers(ss.str());
        _matchFinished = true;
        matchDoneCV.notify_all();
    }
    void nl_MatchEvent(chesscom::MatchEvent matchEvent, std::string& usertoken)
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
    void nl_MatchEventAll(chesscom::MatchEvent matchEvent)
    {
        Messenger::Log("MatchEventAll " + std::to_string(matchEvent));
        chesscom::MoveResult mrPkt;
        mrPkt.set_move_happned(false);
        mrPkt.set_match_event(matchEvent);

        for(auto& [usertoken, player]  : _players)
        {
            (void)usertoken;//To suppress unused variable warning
            if(player.resultStream != nullptr && player.resultStream->alive) 
            {
                player.resultStream->streamPtr->Write(mrPkt);
            }
        }
    }
    void nl_MatchEventAllWithMove(chesscom::MatchEvent matchEvent)
    {
        chesscom::MoveResult mrPkt;
        chesscom::GameState whiteGS;
        SlugChessConverter::SetGameState(game, &whiteGS, true);
        chesscom::GameState blackGS;
        SlugChessConverter::SetGameState(game, &blackGS, false);
        chesscom::GameState observerGS;
        //SlugChessConverter::SetGameState(game, &whiteGS, true); TODO implement oberserver gamestate

        mrPkt.set_move_happned(true);
        mrPkt.set_opponent_asking_for_draw(false);
        mrPkt.set_match_event(matchEvent);
        mrPkt.mutable_chess_clock()->set_white_seconds_left(clock->whiteSecLeft);
        mrPkt.mutable_chess_clock()->set_black_seconds_left(clock->blackSecLeft);
        mrPkt.mutable_chess_clock()->set_timer_ticking(clock->is_ticking);

        for(auto& [usertoken, player]  : _players)
        {
            (void)usertoken;//To suppress unused variable warning
            if(player.resultStream != nullptr && player.resultStream->alive) 
            {
                mrPkt.set_allocated_game_state(&(player.type == PlayerTypes::White?whiteGS:
                                                player.type == PlayerTypes::Black?blackGS:observerGS));
                player.resultStream->streamPtr->Write(mrPkt);
                mrPkt.release_game_state();
            }
        }

    }

};
