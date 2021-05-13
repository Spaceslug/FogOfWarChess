#include "gamebrowser.h"
#include "matchmanager.h"

GameBrowser* GameBrowser::_instance = 0;

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
    // for (auto &&game : _availableGames)
    // {
    //     std::cout << "Hosting game " << std::to_string(game.first) << " " << game.second.host().usertoken() << std::endl << std::flush;
    // }
    *gamesList.mutable_hosted_games() = {_availableGames.begin(), _availableGames.end()};
    // for (auto &&game : gamesList.hostedgames())
    // {
    //     std::cout << "Hosting game " << std::to_string(game.first) << " " << game.second.host().usertoken() << std::endl << std::flush;
    // }
    
}

void GameBrowser::JoinGame(int32_t id, const chesscom::UserData& joinerData, chesscom::LookForMatchResult* joinerMatchResult)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
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
        joinerMatchResult->mutable_game_rules()->CopyFrom(hostedGame.game_rules());
        hostMatchResult->mutable_game_rules()->CopyFrom(hostedGame.game_rules());
        joinerMatchResult->mutable_opponent_user_data()->CopyFrom(hostedGame.host());
        hostMatchResult->mutable_opponent_user_data()->CopyFrom(hostedGame.joiner());
        std::string matchId = MatchManager::Get()->CreateMatch(hostedGame);
        bool hostWhite = MatchManager::Get()->GetMatch(matchId)->getWhitePlayer() == hostedGame.host().usertoken();
        joinerMatchResult->set_match_token(matchId);
        hostMatchResult->set_match_token(matchId);
        joinerMatchResult->set_is_white_player(!hostWhite);
        hostMatchResult->set_is_white_player(hostWhite);
        *hostFinished = true;
        hostCV->notify_all();
    }
    else
    {
        joinerMatchResult->set_succes(false);
    }
#pragma GCC diagnostic pop    
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

void GameBrowser::UserLoggedOut(const std::string& token, std::shared_ptr<chesscom::UserData> userData)
{
    int idToRemove = -1;
    {
        std::unique_lock<std::mutex> lk (_waitingHostsMutex);
        for(auto& [id, hostedGame] :  _availableGames)
        {
//            int id = keyVal.first;
//            chesscom::HostedGame& hostedGame = ;
              if(hostedGame.host().usertoken() == token)
              {
                  idToRemove = id;
                  break;
              }
        }
    }
    if(idToRemove > 0)
    {
        CancelHostGame(idToRemove);
    }
}
