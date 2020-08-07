#include "match.h"
#include "messenger.h"
#include "usermanager.h"

void Match::PlayerDisconnected(const std::string& usertoken, chesscom::MatchEvent matchEventType)
{
    SendMessageAllPlayers(UserManager::Get()->GetUserName(usertoken) + " left the game");

    std::unique_lock<std::mutex> scopeLock (_mutex);
    matchEvents.push_back(matchEventType);
    cv.notify_all();
    //TORM newSystem
    if(matchEventType == chesscom::MatchEvent::WhiteWin || matchEventType == chesscom::MatchEvent::BlackWin ||
     matchEventType == chesscom::MatchEvent::Draw )
     {
         nl_MatchEventAll(matchEventType);
         nl_MatchFinished();
     }
}

void Match::PlayerListenerJoin(const std::string& usertoken, std::shared_ptr<MoveResultStream> resultStream )
{
    if(usertoken == _whitePlayer || usertoken == _blackPlayer)
    {
        //Allready have token and type
        std::unique_lock<std::mutex> scopeLock (_mutex);
        _players.at(usertoken).resultStream = resultStream;
    }
    else
    {
        //Is observer
        std::unique_lock<std::mutex> scopeLock (_mutex);
        _players[usertoken].usertoken = usertoken;
        _players[usertoken].type = PlayerTypes::Observer;
        _players[usertoken].resultStream = resultStream;
    }
}

void Match::PlayerListenerDisconnected(const std::string& usertoken)
{
    std::unique_lock<std::mutex> scopeLock (_mutex);
    if(_players.count(usertoken) > 0)
    {
        _players.at(usertoken).resultStream = nullptr;
    }
}

void Match::SendMessageAllPlayers(const std::string& message)
{
    std::unique_lock<std::mutex> scopeLock (_mutex);
    nl_SendMessageAllPlayers(message);
}
void Match::nl_SendMessageAllPlayers(const std::string& message)
{
    for(auto& [usertoken, player]  : _players)
    {
        if(player.resultStream != nullptr) //Indicates player is connected to match and wants to listen
        {
            Messenger::SendServerMessage(usertoken, message);
        }
    }
}