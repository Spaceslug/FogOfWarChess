#pragma once
#include <condition_variable>
#include <map>
#include <chrono>
#include "../../SlugChessCore/src/slugchess.h"

enum PlayerTypes{
    White,
    Black,
    Observer
};
struct ChessClock {
    int blackSecLeft;
    int whiteSecLeft;
    int secsPerMove;
    bool is_ticking;
};
struct ChessMove {
    std::string from;
    std::string to;
};
struct Player
{
    PlayerTypes type;
    std::string usertoken;
    grpc::ServerWriter<chesscom::MoveResult>* streamPtr;
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
    Match(const std::string& token, const std::string& whitePlayerToken, const std::string& blackPlayerToken, const std::string& fenString, VisionRules& visionRules)
    {
        matchToken = token;
        whitePlayer = whitePlayerToken;
        blackPlayer = blackPlayerToken;
        players[whitePlayerToken].usertoken = whitePlayerToken;
        players[whitePlayerToken].type = PlayerTypes::White;
        players[whitePlayerToken].askingForDrawTimstamp = std::chrono::system_clock::now();
        players[blackPlayerToken].usertoken = blackPlayerToken;
        players[blackPlayerToken].type = PlayerTypes::Black; 
        players[blackPlayerToken].askingForDrawTimstamp = std::chrono::system_clock::now();
        clock = std::make_shared<ChessClock>();
        game = std::make_shared<SlugChess>(fenString, visionRules);
        std::cout  << "Creating match: " << matchToken << " white: " << whitePlayerToken << " black:" << blackPlayerToken  << std::endl << std::flush;
    }

    bool IsWhitesMove() { return moves.size()%2 == 0; }

    bool DoMove(const std::string& usertoken, std::shared_ptr<chesscom::Move> move, int secSpent) 
    {
        std::cout  << usertoken << " iswhite: " << std::to_string(usertoken == whitePlayer) << " iswhitesmove" << std::to_string(IsWhitesMove())  << std::endl << std::flush;
        if((IsWhitesMove() && whitePlayer == usertoken)  
            || ((!IsWhitesMove()) && blackPlayer == usertoken))
        {
            std::unique_lock<std::mutex> scopeLock (mutex);
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
                *secToChange -= secSpent;
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
            
            return true;
        }
        else
        {
            std::cout  <<  usertoken << " ERROR: not this players turn to move" << std::endl << std::flush;
            return false;
        }
    }

    void PlayerDisconnected(const std::string& usertoken, chesscom::MatchEvent matchEventType)
    {
        std::unique_lock<std::mutex> scopeLock (mutex);
        //TODO Use streamPtr (or something) of player to send message: {username} left the game
        matchEvents.push_back(matchEventType);
        cv.notify_all();
    }

    void PlayerAskingForDraw(const std::string& usertoken)
    {
        std::unique_lock<std::mutex> scopeLock (mutex);
        std::string playerUsertoken, opponentUsertoken;
        if(whitePlayer == usertoken){
            if(!IsWhitesMove())return;
            playerUsertoken = usertoken;
            opponentUsertoken = blackPlayer;
        }else if(blackPlayer == usertoken){
            if(IsWhitesMove())return;
            playerUsertoken = usertoken;
            opponentUsertoken = whitePlayer;
        }else{
            std::cout  <<  usertoken << " ERROR: asked for draw but is not a player" << std::endl << std::flush;
            return;
        }
        if(players.at(opponentUsertoken).SecSinceAskedForDraw() < TIMEOUT_FOR_DRAW)
        {
            //Opponent allready asked
            matchEvents.push_back(chesscom::MatchEvent::Draw);
            cv.notify_all();
            //std::cout  <<  usertoken << " op allready asking for draw. SecSinceAsked " << std::to_string(players[opponentUsertoken].SecSinceAskedForDraw()) << std::endl << std::flush;
        }
        else if(players.at(playerUsertoken).SecSinceAskedForDraw() > TIMEOUT_FOR_DRAW)
        {
            players.at(playerUsertoken).askingForDrawTimstamp = std::chrono::system_clock::now();
            matchEvents.push_back(chesscom::MatchEvent::AskingForDraw);
            cv.notify_all();
            std::cout  <<  usertoken << " asking for draw" << std::endl << std::flush;
            players.at(playerUsertoken).SecSinceAskedForDraw();
        }else{
            std::cout  <<  usertoken << " allready asked for draw" << std::endl << std::flush;

        }
    }

    void PlayerAcceptingDraw(const std::string& usertoken)
    {
        std::unique_lock<std::mutex> scopeLock (mutex);
        std::string playerUsertoken, opponentUsertoken;
        if(whitePlayer == usertoken){
            playerUsertoken = usertoken;
            opponentUsertoken = blackPlayer;
        }else if(blackPlayer == usertoken){
            playerUsertoken = usertoken;
            opponentUsertoken = whitePlayer;
        }else{
            std::cout  <<  usertoken << " ERROR: accepting draw but is not a player" << std::endl << std::flush;
            return;
        }
        if(players.at(opponentUsertoken).SecSinceAskedForDraw() < TIMEOUT_FOR_DRAW + EXTRATIME_FOR_DRAW)
        {
            matchEvents.push_back(chesscom::MatchEvent::Draw);
            cv.notify_all();
            std::cout  <<  usertoken << " accepted draw for draw" << std::endl << std::flush;
        }else{
            std::cout <<  usertoken << "acept draw failed. To many Sec since opp asked draw: " << std::to_string(players.at(opponentUsertoken).SecSinceAskedForDraw()) << std::endl << std::flush;
        }
    }
	
    std::vector<chesscom::MatchEvent> matchEvents;
    std::string whitePlayer;
    std::string blackPlayer;
    std::string matchToken;
    std::vector<std::shared_ptr<chesscom::Move>> moves;
    std::map<std::string, Player> players;
    std::string& getWhitePlayer(){return whitePlayer;}
    std::string& getBlackPlayer(){return blackPlayer;}
    std::string& getMatchToken(){return matchToken;}
    std::shared_ptr<ChessClock> clock;
    std::shared_ptr<SlugChess> game;
    std::mutex mutex;
    std::condition_variable cv;
};
