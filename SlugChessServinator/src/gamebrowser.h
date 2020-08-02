#pragma once
#include "mutex"
#include "condition_variable"
#include "tuple"

#include "../../SlugChessCore/src/slugchess.h"
#include "../chesscom/chesscom.pb.h"


class GameBrowser{
    private:
    static GameBrowser* _instance;
    GameBrowser() {}
    std::atomic<int> _idCounter = 1000;
    std::mutex _availableGamesMutex;
    std::mutex _waitingHostsMutex;  
    google::protobuf::Map<google::protobuf::int32, chesscom::HostedGame> _availableGames;
    std::map<int, std::tuple<std::condition_variable*, chesscom::LookForMatchResult*, bool*>> _waitingHosts;

    int GetNewId(){ return _idCounter++;}

    public:
    static GameBrowser* Get()
    {
        if(!_instance) _instance = new GameBrowser();
        return _instance;
    }
    int HostGame(const chesscom::HostedGame& hostGame, chesscom::LookForMatchResult* matchResult, std::condition_variable* hostCV, bool* finished);
    void WriteAvailableGames(chesscom::HostedGamesMap& gamesList);
    void JoinGame(int32_t id, const chesscom::UserData& joinerData, chesscom::LookForMatchResult* joinerMatchResult);
    void CancelHostGame(int id);
    void UserLoggedOut(const std::string& token, std::shared_ptr<chesscom::UserData> userData);



};
