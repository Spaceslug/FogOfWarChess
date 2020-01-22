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
    return id;
}

void GameBrowser::WriteAvailableGames(chesscom::HostedGamesMap& gamesList)
{
    std::unique_lock<std::mutex> scopeLock (_availableGamesMutex);
    *gamesList.mutable_hostedgames() = {_availableGames.begin(), _availableGames.end()};
}

void GameBrowser::JoinGame(int32_t id, chesscom::LookForMatchResult* joinerMatchResult)
{
    chesscom::HostedGame hostedGame;
    std::tuple<std::condition_variable*, chesscom::LookForMatchResult*, bool*> hostTuple;
    bool successExtracted = false;
    {
        std::unique_lock<std::mutex> scopeLock1 (_waitingHostsMutex);
        std::unique_lock<std::mutex> scopeLock2 (_availableGamesMutex);
        if(_availableGames.count(id) > 0)
        {
            successExtracted = true;
            hostedGame = _availableGames[id];
            _availableGames.erase(id);
            hostTuple = _waitingHosts[id];
            _waitingHosts.erase(id);
        }
    }
    if(successExtracted)
    {

    }
    else
    {
        
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