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

    bool DoMove(const std::string& usertoken, std::shared_ptr<chesscom::Move> move); 
    void PlayerDisconnected(const std::string& usertoken, chesscom::MatchEvent matchEventType);
    void PlayerListenerJoin(const std::string& usertoken, std::shared_ptr<MoveResultStream> resultStream);
    void PlayerListenerDisconnected(const std::string& usertoken);
    void PlayerAskingForDraw(const std::string& usertoken);
    void PlayerAcceptingDraw(const std::string& usertoken);
    std::string GetPgnString();

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
    void nl_MatchFinished();
    void nl_MatchEvent(chesscom::MatchEvent matchEvent, std::string& usertoken);
    void nl_MatchEventAll(chesscom::MatchEvent matchEvent);
    void nl_MatchEventAllWithMove(chesscom::MatchEvent matchEvent);
};
