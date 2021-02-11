#include "usermanager.h"
#include "matchmanager.h"
#include "gamebrowser.h"
UserManager* UserManager::_instance = nullptr;

UserManager::~UserManager(){
    for (auto& [usertoken, user] : _logedInUsers)
    {
        UserStore::WriteUserEloToFile(user.data, user.encryptionKey);
        Messenger::Log("Wrote user ELO on quit: " + user.data->username());
    }
    
}
void UserManager::LogInUser(const std::string& token, const chesscom::UserData& userData, const std::vector<unsigned char>& encryptionKey)
{
    std::scoped_lock<std::mutex> lock(_mutex);
    _logedInUsers[token].data = std::make_shared<chesscom::UserData>();
    _logedInUsers[token].data.get()->set_username(userData.username());
    _logedInUsers[token].data.get()->set_usertoken(userData.usertoken());
    _logedInUsers[token].data.get()->set_elo(userData.elo());
    _logedInUsers[token].lastHeartbeat = std::chrono::system_clock::now();
    _logedInUsers[token].encryptionKey = encryptionKey;
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
    std::vector<uint8_t> encryptionKey;
    {
        std::scoped_lock<std::mutex> lock(_mutex);
        if(_logedInUsers.count(token) > 0){
            userdata = _logedInUsers.at(token).data;
            encryptionKey = _logedInUsers.at(token).encryptionKey;
            _logedInUsers.at(token).changedCV.notify_all();
            _logedInUsers.erase(token);
        }else{
            return false;
        }
        
    }
    UserStore::Get()->WriteUserEloToFile(userdata, encryptionKey);
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

chesscom::UserData UserManager::GetPublicUserDataFromUsername(const std::string& username)
{
    {
        std::scoped_lock<std::mutex> lock(_mutex);
        for (auto& [usertoken, user] : _logedInUsers)
        {
            if(user.data->username() == username){
                chesscom::UserData userdata;
                userdata.set_elo(user.data->elo());
                userdata.set_username(user.data->username());
                return userdata;
            }   
        }
    }
    chesscom::UserData userData;\
    const auto filename = UserStore::UsernameToDataFilename(username);
    if(Filesystem::UserFileExists(filename)){
        try{
            auto userdata_static = Filesystem::ReadUserFile(UserStore::UsernameToDataFilename(username));
            userData.set_username(userdata_static.username());
            userData.set_elo(userdata_static.elo());
            return userData;

        } catch(std::invalid_argument& ex){

        }
    }
    Messenger::Log("Failed to find userdata for '"+username+"'");
    return userData;
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
const std::vector<unsigned char>& UserManager::GetEncryptionKey(const std::string& token)
{
    std::scoped_lock<std::mutex> lock(_mutex);
    if(_logedInUsers.count(token) > 0){
        return _logedInUsers.at(token).encryptionKey;
    } 
    else {
        throw std::invalid_argument("token is not a loggen in user -> "+ token);
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
void UserManager::UpdateElo(const std::string& white_usertoken, const std::string& black_usertoken, chesscom::MatchEvent result)
    {
        if(result != chesscom::MatchEvent::WhiteWin &&
            result != chesscom::MatchEvent::BlackWin &&
            result != chesscom::MatchEvent::Draw)
        {
            throw std::invalid_argument("MatchEvent must be WhiteWin BlackWin og Draw");
        }
        auto white = GetUser(white_usertoken);
        auto black = GetUser(black_usertoken);
        auto result_float = result==chesscom::MatchEvent::BlackWin?1.0f:result==chesscom::MatchEvent::Draw?0.5f:0.0f;
        if(white != nullptr && black != nullptr){
            std::unique_lock<std::mutex> scopeLock (_mutex);
            auto [white_elo, black_elo] = UserStore::Get()->CalcNewElo(white->data->elo(), black->data->elo(), result_float);
            white->data->set_elo(white_elo);
            black->data->set_elo(black_elo);
            auto now = std::chrono::system_clock::now();
            white->lastDataUpdate = now;
            black->lastDataUpdate = now;
        }else{
            Messenger::Log("ERROR: failed to update elo as user no longer logged in");
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
