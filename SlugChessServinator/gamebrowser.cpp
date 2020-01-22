#include "gamebrowser.h" 

int GameBrowser::HostGame(const chesscom::HostedGame& hostGame, chesscom::LookForMatchResult* matchResult, std::condition_variable* hostCV, bool* finished)
{
    int id = GetNewId();
    {
        std::unique_lock<std::mutex> scopeLock (_availableGamesMutex);
        _availableGames[id] = hostGame;
        _availableGames[id].set_id(id);        
    }
    {
        std::unique_lock<std::mutex> scopeLock (_waitingHostsMutex);
        _waitingHosts[id] = std::tuple<std::condition_variable*, chesscom::LookForMatchResult*, bool*>(hostCV, matchResult, finished);
    }
    std::cout << "Hosting game id:" << std::to_string(id) << "host:" << hostGame.host().usertoken()<< std::endl << std::flush;
    return id;
}

void GameBrowser::WriteAvailableGames(chesscom::HostedGamesMap& gamesList)
{
    std::unique_lock<std::mutex> scopeLock (_availableGamesMutex);
    *gamesList.mutable_hostedgames() = {_availableGames.begin(), _availableGames.end()};
}

void GameBrowser::JoinGame(int32_t id, const chesscom::UserData& joinerData, chesscom::LookForMatchResult* joinerMatchResult)
{
    chesscom::HostedGame hostedGame;
    std::condition_variable* hostCV;
    chesscom::LookForMatchResult* hostMatchResult;
    bool* hostFinished;

    //std::tuple<std::condition_variable*, chesscom::LookForMatchResult*, bool*> hostTuple;
    bool successExtracted = false;
    {
        std::unique_lock<std::mutex> scopeLock1 (_waitingHostsMutex);
        std::unique_lock<std::mutex> scopeLock2 (_availableGamesMutex);
        if(_availableGames.count(id) > 0)
        {
            successExtracted = true;
            hostedGame = _availableGames[id];
            _availableGames.erase(id);
            std::tie(hostCV, hostMatchResult, hostFinished) = _waitingHosts[id];
            _waitingHosts.erase(id);
        }
    }
    if(successExtracted)
    {
        hostedGame.mutable_joiner()->CopyFrom(joinerData);
        joinerMatchResult->set_succes(true);
        hostMatchResult->set_succes(true);
        joinerMatchResult->mutable_gamerules()->CopyFrom(hostedGame.gamerules());
        hostMatchResult->mutable_gamerules()->CopyFrom(hostedGame.gamerules());
        joinerMatchResult->set_opponentusername(hostedGame.host().username());
        hostMatchResult->set_opponentusername(hostedGame.joiner().username());
        std::string matchId = _matchManager->CreateMatch(hostedGame);
        bool hostWhite = _matchManager->GetMatch(matchId)->getWhitePlayer() == hostedGame.host().usertoken();
        joinerMatchResult->set_matchtoken(matchId);
        hostMatchResult->set_matchtoken(matchId);
        joinerMatchResult->set_iswhiteplayer(!hostWhite);
        hostMatchResult->set_iswhiteplayer(hostWhite);
        *hostFinished = true;
        hostCV->notify_all();
    }
    else
    {
        joinerMatchResult->set_succes(false);
    }
    
    
}

void GameBrowser::CancelHostGame(int id)
{
    std::unique_lock<std::mutex> scopeLock1 (_waitingHostsMutex);
    std::unique_lock<std::mutex> scopeLock2 (_availableGamesMutex);
    if(_waitingHosts.count(id) > 0)
    {
        _availableGames.erase(id);
        auto [cvPtr, matchPtr, finishedPtr] =_waitingHosts[id];
        _waitingHosts.erase(id);
        matchPtr->set_succes(false);
        *finishedPtr = true;
        cvPtr->notify_all();        
    }
}