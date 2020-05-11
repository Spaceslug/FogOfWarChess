#pragma once
#include <condition_variable>
#include <map>
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
};
class Match {
public:

    Match(const std::string& token, const std::string& whitePlayerToken, const std::string& blackPlayerToken, const std::string& fenString, VisionRules& visionRules)
    {
        matchToken = token;
        whitePlayer = whitePlayerToken;
        blackPlayer = blackPlayerToken;
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

    void PlayerDisconnected(const std::string& usertoken)
    {
        std::unique_lock<std::mutex> scopeLock (mutex);
        matchEvents.push_back(chesscom::MatchEvent::UnexpectedClosing);
    }
	
    bool askingForDraw = false;
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
