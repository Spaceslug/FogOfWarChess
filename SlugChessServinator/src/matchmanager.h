#pragma once
#include "../chesscom/chesscom.grpc.pb.h"
#include "match.h"


class MatchManager
{
    private:
    static MatchManager* _instance;
    MatchManager() 
    {
        defaultVisionRules = ServerVisionRules();
        defaultTimeRules = ServerTimeRules();
    }

    std::mutex _matchesMutex;
    std::map<std::string, std::shared_ptr<::Match>> _matches;
    std::atomic<int> _tokenCounter;
    chesscom::TimeRules defaultTimeRules;
    VisionRules defaultVisionRules;

    std::pair<std::string,std::string> RandomSort(const std::string& first, const std::string& second);

    public:
    static MatchManager* Get()
    {
        if (!_instance)
        _instance = new MatchManager();
        return _instance;
    }
    static void MatchListenLoop(std::string listenerUsertoken, 
        std::shared_ptr<Match> matchPtr,
        grpc::ServerContext* contextPtr,
        grpc::internal::WriterInterface<chesscom::MoveResult>* writerPtr);
    static void DoMoveInMatch(
        chesscom::MatchEvent event,
        std::string usertoken, 
        std::shared_ptr<Match> matchPtr, 
        std::shared_ptr<chesscom::Move> movePtr);

    std::string CreateMatch(std::string& player1Token, std::string& player2Token);
    std::string CreateMatch(chesscom::HostedGame& hostedGame);
    std::shared_ptr<Match> GetMatch(const std::string& matchId);
    void EraseMatch(const std::string& matchId);
    int OngoingMatches() { return _matches.size(); }
    VisionRules FromChesscomVisionRules(const chesscom::VisionRules& chesscomVision);
    void UserLoggedOut(const std::string& token, std::shared_ptr<chesscom::UserData> userData);

    chesscom::TimeRules ServerTimeRules()
    {
        chesscom::TimeRules tr;
        tr.mutable_player_time()->set_minutes(5);
        tr.mutable_player_time()->set_seconds(0);
        tr.set_seconds_per_move(6);

        return tr;
    }

    VisionRules ServerVisionRules() 
    {
        VisionRules rules;
        rules.globalRules = Rules();
        rules.globalRules.ViewCaptureField = true;
        rules.globalRules.ViewMoveFields = false;
        rules.globalRules.ViewRange = 2;
        rules.enabled = true;
        rules.overWriteRules[ChessPice::WhitePawn] = Rules(false, true, 1);
        rules.overWriteRules[ChessPice::BlackPawn] = Rules(false, true, 1);
	
        return rules;
    }
};
