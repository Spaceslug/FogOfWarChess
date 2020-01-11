#pragma once
#include "mutex"
#include "condition_variable"
#include "tuple"

#include "slugchess.h"
#include "chesscom.pb.h"


class GameBrowser{
    public:
    int HostGame(const chesscom::HostedGame& hostGame, chesscom::LookForMatchResult* matchResult, std::condition_variable* hostCV, bool* finished);
    void WriteAvailableGames(chesscom::HostedGamesMap& gamesList);
    void JoinGame(int32_t id, chesscom::LookForMatchResult* joinerMatchResult);
    void CancelHostGame(int id);
    private:
    std::atomic<int> _idCounter;
    std::mutex _availableGamesMutex;
    std::mutex _waitingHostsMutex;
    
    //std::set<chesscom::HostedGame, decltype(&hosted_games_compare)> _availableGames = std::set<chesscom::HostedGame, decltype(&hosted_games_compare)>(&hosted_games_compare);
    google::protobuf::Map<google::protobuf::int32, chesscom::HostedGame> _availableGames;
    std::map<int, std::tuple<std::condition_variable*, chesscom::LookForMatchResult*, bool*>> _waitingHosts;

    int GetNewId(){ return _idCounter++;}



};