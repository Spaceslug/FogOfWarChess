#pragma once
#include "mutex"
#include "condition_variable"
#include "tuple"

#include "../../SlugChessCore/src/slugchess.h"
#include "matchmanager.h"
#include "../chesscom/chesscom.pb.h"


class GameBrowser{
    public:
    MatchManager* _matchManager;
    int HostGame(const chesscom::HostedGame& hostGame, chesscom::LookForMatchResult* matchResult, std::condition_variable* hostCV, bool* finished);
    void WriteAvailableGames(chesscom::HostedGamesMap& gamesList);
    void JoinGame(int32_t id, const chesscom::UserData& joinerData, chesscom::LookForMatchResult* joinerMatchResult);
    void CancelHostGame(int id);
    private:
    std::atomic<int> _idCounter = 1000;
    std::mutex _availableGamesMutex;
    std::mutex _waitingHostsMutex;
    
    //std::set<chesscom::HostedGame, decltype(&hosted_games_compare)> _availableGames = std::set<chesscom::HostedGame, decltype(&hosted_games_compare)>(&hosted_games_compare);
    google::protobuf::Map<google::protobuf::int32, chesscom::HostedGame> _availableGames;
    std::map<int, std::tuple<std::condition_variable*, chesscom::LookForMatchResult*, bool*>> _waitingHosts;

    int GetNewId(){ return _idCounter++;}



};