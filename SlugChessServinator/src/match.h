#pragma once
#include <condition_variable>
#include <map>
#include <chrono>
#include <memory>
#include "messenger.h"
#include "common.h"
#include "slugchesscoverter.h"
#include "../../SlugChessCore/src/slugchess.h"

class Match {
public:

    Match(const std::string& token, const std::string& whitePlayerToken, const std::string& blackPlayerToken, 
            const std::string& fenString, const std::string& ruleType, VisionRules& visionRules);

    bool IsWhitesMove() { return moves.size()%2 == 0; }

    bool DoMove(const std::string& usertoken, std::shared_ptr<chesscom::Move> move); 
    void PlayerDisconnected(const std::string& usertoken, chesscom::MatchEvent matchEventType);
    void PlayerListenerJoin(const std::string& usertoken, std::shared_ptr<MoveResultStream> resultStream);
    void PlayerListenerDisconnected(const std::string& usertoken);
    void PlayerAskingForDraw(const std::string& usertoken);
    void PlayerAcceptingDraw(const std::string& usertoken);
    std::string GetPgnString(time_t& ttime);
    static std::map<std::string, std::string> ReadSlugChessPgnString(const std::string& pgn);

    void SendMessageAllPlayers(const std::string& message);
    bool Ongoing(){ return !_matchFinished; }
	
    std::vector<chesscom::MatchEvent> matchEvents;

    std::vector<std::shared_ptr<chesscom::Move>> moves;
    std::string& getWhitePlayer(){return _whitePlayer;}
    std::string& getBlackPlayer(){return _blackPlayer;}
    std::string& getMatchToken(){return _matchToken;}
    PlayerTypes getPlayerType(const std::string& usertoken){ return _players.at(usertoken).type; }
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
    std::string _pgn = "";
    std::string _ruleType;

    void nl_SendMessageAllPlayers(const std::string& message);
    void nl_MatchFinished();
    void nl_MatchEventAskingForDraw(chesscom::MatchEvent matchEvent, std::string& usertoken);
    void nl_MatchEventAll(chesscom::MatchEvent matchEvent, bool moveHappened);
};
