#include "usermanager.h"

void UserManager::LogInUser(const std::string& token, const chesscom::UserData& userData)
{
    std::scoped_lock<std::mutex> lock(_mutex);
    _logedInUsers[token] = userData;
    _timepoints[token] = std::chrono::system_clock::now();
}

bool UserManager::UsertokenLoggedIn(const std::string& token)
{
    std::scoped_lock<std::mutex> lock(_mutex);
    return _logedInUsers.count(token) > 0;
}

bool UserManager::Heartbeat(const std::string& token)
{
    if(TestHeart(token)){
        _timepoints[token] = std::chrono::system_clock::now();
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
            << std::chrono::duration_cast<std::chrono::milliseconds>(now - _timepoints[token]).count()
            << std::endl << std::flush;
        if(std::chrono::duration_cast<std::chrono::milliseconds>(now - _timepoints[token]) 
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
    std::scoped_lock<std::mutex> lock(_mutex);
    _logedInUsers.erase(token);
    _timepoints.erase(token);
    return true;
}

const chesscom::UserData& UserManager::Getuser_data(const std::string& token)
{
    std::scoped_lock<std::mutex> lock(_mutex);
    return _logedInUsers.at(token);
}