#pragma once
#include <mutex>
#include <string>
#include <unordered_map>

#include "chesscom.grpc.pb.h"

//PLEASE NOTE: ONE CAN NOT ASSUME A USER IS LOGGED IN FROM ONE PUBLIC CALL TO ANOTHER
//   SOME REWORK OF FUNCTIONS IS NESSESARY FOR THAT
class UserManager
{
    public:
    void LogInUser(const std::string& token, const chesscom::UserData& userData);
    bool UsertokenLoggedIn(const std::string& token);
    bool TestHeart(const std::string& token);
    bool Logout(const std::string& token);
    const chesscom::UserData& GetUserData(const std::string& token);
    private:
    std::mutex _mutex;
    std::unordered_map<std::string, chesscom::UserData> _logedInUsers;
    std::unordered_map<std::string, std::chrono::time_point<std::chrono::system_clock>> _timepoints;
};