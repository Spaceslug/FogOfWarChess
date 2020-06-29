#pragma once
#include <mutex>
#include <string>
#include <unordered_map>

#include "../chesscom/chesscom.grpc.pb.h"

struct User
{
    std::shared_ptr<chesscom::UserData> data;
    std::chrono::time_point<std::chrono::system_clock> lastHeartbeat;
    grpc::internal::WriterInterface<chesscom::ChatMessage>* messageStream = nullptr;
    std::mutex _messageStreamMutex;
};

//PLEASE NOTE: ONE CAN NOT ASSUME A USER IS LOGGED IN FROM ONE PUBLIC CALL TO ANOTHER
//   SOME REWORK OF FUNCTIONS IS NESSESARY FOR THAT
class UserManager
{
    private:
    static UserManager* _instance;
    UserManager() {}
    std::mutex _mutex;
    std::unordered_map<std::string, User> _logedInUsers;

    public:
    static UserManager* Get()
    {
        if(!_instance) 
        _instance = new UserManager();
        return _instance;
    }

    void LogInUser(const std::string& token, const chesscom::UserData& userData);
    bool UsertokenLoggedIn(const std::string& token);
    bool Heartbeat(const std::string& token);
    bool TestHeart(const std::string& token);
    bool Logout(const std::string& token);
    std::shared_ptr<chesscom::UserData> GetUserData(const std::string& token);
    User* GetUser(const std::string& token);

    void AddMessageStream(const std::string& usertoken, grpc::internal::WriterInterface< chesscom::ChatMessage>* stream);
    void RemoveMessageStream(const std::string& usertoken);
    
    
};