#pragma once
#include <mutex>
#include <string>
#include <unordered_map>
#include <condition_variable>
#include <chrono>

#include "../chesscom/chesscom.grpc.pb.h"
#include "userstore.h"
#include "filesystem.h"

/*struct MessagePack
{
    grpc::internal::WriterInterface<chesscom::ChatMessage>* writer = nullptr;
    std::condition_variable* messageListenerCV;
    std::mutex writerLock;
};*/

struct User
{
    std::shared_ptr<chesscom::UserData> data;
    std::chrono::time_point<std::chrono::system_clock> lastHeartbeat;
    grpc::internal::WriterInterface<chesscom::ChatMessage>* messageStream = nullptr;
    std::mutex messageStreamMutex;
    std::condition_variable changedCV;
    std::vector<unsigned char> encryptionKey;
    std::chrono::time_point<std::chrono::system_clock> lastDataUpdate;
};

//PLEASE NOTE: ONE CAN NOT ASSUME A USER IS LOGGED IN FROM ONE PUBLIC CALL TO ANOTHER
//   SOME REWORK OF FUNCTIONS IS NESSESARY FOR THAT
class UserManager
{
    private:
    static UserManager* _instance;
    UserManager() {}
    ~UserManager();
    std::mutex _mutex;
    std::unordered_map<std::string, User> _logedInUsers;

    public:
    static UserManager* Get()
    {
        if(!_instance) 
        _instance = new UserManager();
        return _instance;
    }
    static void DeleteInstance(){ if(_instance != nullptr) {delete _instance; _instance = nullptr;}}

    void LogInUser(const std::string& token, const chesscom::UserData& userData, const std::vector<unsigned char>& encryptionKey);
    bool UsertokenLoggedIn(const std::string& token);
    bool Heartbeat(const std::string& token);
    bool TestHeart(const std::string& token);
    bool Logout(const std::string& token);
    std::shared_ptr<chesscom::UserData> GetUserData(const std::string& token);
    chesscom::UserData GetPublicUserDataFromUsername(const std::string& username);
    User* GetUser(const std::string& token);
    std::string GetUserName(const std::string& token);
    const std::vector<unsigned char>& GetEncryptionKey(const std::string& token);
    int ActiveUsers() { return _logedInUsers.size(); }
    void CheckHeartbeat();
    void UpdateElo(const std::string& white_usertoken, const std::string& black_usertoken, chesscom::MatchEvent result);

    void AddMessageStream(const std::string& usertoken, grpc::internal::WriterInterface< chesscom::ChatMessage>* stream);
    void RemoveMessageStream(const std::string& usertoken);
    
    
};
