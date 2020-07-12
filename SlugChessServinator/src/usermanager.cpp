#include "usermanager.h"
#include "matchmanager.h"
#include "gamebrowser.h"
UserManager* UserManager::_instance = 0;

void UserManager::LogInUser(const std::string& token, const chesscom::UserData& userData)
{
    std::scoped_lock<std::mutex> lock(_mutex);
    _logedInUsers[token].data = std::make_shared<chesscom::UserData>();
    _logedInUsers[token].data.get()->set_username(userData.username());
    _logedInUsers[token].data.get()->set_usertoken(userData.usertoken());
    _logedInUsers[token].data.get()->set_elo(userData.elo());
    _logedInUsers[token].lastHeartbeat = std::chrono::system_clock::now();
}

bool UserManager::UsertokenLoggedIn(const std::string& token)
{
    std::scoped_lock<std::mutex> lock(_mutex);
    return _logedInUsers.count(token) > 0;
}

bool UserManager::Heartbeat(const std::string& token)
{
    if(TestHeart(token)){
        _logedInUsers.at(token).lastHeartbeat = std::chrono::system_clock::now();
        return true;
    }
    else
    {
        return false;
    }
}

bool UserManager::TestHeart(const std::string& token)
{
    if(UsertokenLoggedIn(token)){
        std::scoped_lock<std::mutex> lock(_mutex);
        auto now = std::chrono::system_clock::now();
        std::cout << token << " duration since last heartbeat " 
            << std::chrono::duration_cast<std::chrono::milliseconds>(now - _logedInUsers.at(token).lastHeartbeat).count()
            << std::endl << std::flush;
        if(std::chrono::duration_cast<std::chrono::milliseconds>(now - _logedInUsers.at(token).lastHeartbeat) 
            > std::chrono::milliseconds(1000*60*2))
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return false;
    }
    
    
}

bool UserManager::Logout(const std::string& token)
{
    std::shared_ptr<chesscom::UserData> userdata;
    {
    std::scoped_lock<std::mutex> lock(_mutex);
    userdata = _logedInUsers.at(token).data;
    _logedInUsers.at(token).changedCV.notify_all();
    _logedInUsers.erase(token);
    }
    //Add calls to managers that want to be notified of this
    GameBrowser::Get()->UserLoggedOut(token, userdata);
    MatchManager::Get()->UserLoggedOut(token, userdata);
    return true;
}

std::shared_ptr<chesscom::UserData> UserManager::GetUserData(const std::string& token)
{
    std::scoped_lock<std::mutex> lock(_mutex);
    if(_logedInUsers.count(token) > 0){
        return _logedInUsers.at(token).data;
    } 
    else {
        return std::shared_ptr<chesscom::UserData>();
    }
}

User* UserManager::GetUser(const std::string& token)
{
    std::scoped_lock<std::mutex> lock(_mutex);
    if(_logedInUsers.count(token) > 0){
        return &_logedInUsers.at(token);
    } 
    else {
        return nullptr;
    }
}
std::string UserManager::GetUserName(const std::string& token)
{
    std::scoped_lock<std::mutex> lock(_mutex);
    if(_logedInUsers.count(token) > 0){
        return _logedInUsers.at(token).data->username();
    } 
    else {
        return "{unknown username}";
    }
}

void UserManager::CheckHeartbeat()
{
    std::vector<std::string> usersToLogOut;
    {
    std::unique_lock<std::mutex> scopeLock (_mutex);
    for(auto&& keyVal : _logedInUsers)
    {
        User& user = keyVal.second;
        if(std::chrono::system_clock::now() > user.lastHeartbeat + std::chrono::minutes(2))
        {
            usersToLogOut.push_back(keyVal.first);
        }
    }
    }
    for(auto&& usertoken : usersToLogOut)
    {
        Logout(usertoken);
    }
}
void UserManager::AddMessageStream(const std::string& usertoken, grpc::internal::WriterInterface< chesscom::ChatMessage>* stream) {
    std::unique_lock<std::mutex> scopeLock (_mutex);
    if(_logedInUsers.count(usertoken) > 0){
        std::unique_lock<std::mutex> scopeLock (_logedInUsers.at(usertoken).messageStreamMutex);
        _logedInUsers.at(usertoken).messageStream = stream;
    } 
}
void UserManager::RemoveMessageStream(const std::string& usertoken)
{
    std::unique_lock<std::mutex> scopeLock (_mutex);
    if(_logedInUsers.count(usertoken) > 0){
        std::unique_lock<std::mutex> scopeLock (_logedInUsers.at(usertoken).messageStreamMutex);
        _logedInUsers.at(usertoken).messageStream = nullptr;
    } 
}
