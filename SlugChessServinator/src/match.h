#pragma once
#include <condition_variable>
#include "../../SlugChessCore/src/slugchess.h"

struct ChessClock {
    int blackSecLeft;
    int whiteSecLeft;
};
struct ChessMove {
    std::string from;
    std::string to;
};
class Match {
public:

    Match(const std::string& token, const std::string& whitePlayerToken, const std::string& blackPlayerToken, const std::string& fenString, VisionRules& visionRules)
    {
        matchToken = token;
        whitePlayer = whitePlayerToken;
        blackPlayer = blackPlayerToken;
        clock = std::make_shared<ChessClock>();
        game = std::make_shared<SlugChess>("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w AHah - 0 1", visionRules);
    }
	
    //bool newUpdate = false;
    bool askingForDraw = false;
    //chesscom::MatchEvent matchEvent = chesscom::MatchEvent::Non;
    std::vector<chesscom::MatchEvent> matchEvents;
    std::string whitePlayer;
    std::string blackPlayer;
    std::string matchToken;
    std::vector<std::shared_ptr<chesscom::Move>> moves;
    std::string& getWhitePlayer(){return whitePlayer;}
    std::string& getBlackPlayer(){return blackPlayer;}
    std::string& getMatchToken(){return matchToken;}
    std::shared_ptr<ChessClock> clock;
    std::shared_ptr<SlugChess> game;
    std::mutex mutex;
    std::condition_variable cv;
};
