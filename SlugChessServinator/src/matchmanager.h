#pragma once
#include "../chesscom/chesscom.grpc.pb.h"
#include "match.h"


class MatchManager
{
    public:
    std::string CreateMatch(std::string& player1Token, std::string& player2Token);
    std::string CreateMatch(chesscom::HostedGame& hostedGame);
    std::shared_ptr<Match> GetMatch(const std::string& matchId);
    void EraseMatch(const std::string& matchId);
    VisionRules FromChesscomVisionRules(const chesscom::VisionRules& chesscomVision);

    chesscom::TimeRules ServerTimeRules()
    {
        chesscom::TimeRules tr;
        tr.mutable_playertime()->set_minutes(5);
        tr.mutable_playertime()->set_seconds(0);
        tr.set_secondspermove(6);

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

    MatchManager(){
        defaultVisionRules = ServerVisionRules();
        defaultTimeRules = ServerTimeRules();
    }
    private:
    std::mutex _matchesMutex;
    std::map<std::string, std::shared_ptr<::Match>> _matches;
    std::atomic<int> _tokenCounter;
    chesscom::TimeRules defaultTimeRules;
    VisionRules defaultVisionRules;

    std::pair<std::string,std::string> RandomSort(const std::string& first, const std::string& second);
};