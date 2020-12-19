#pragma once
#include <map>
#include <memory>
#include "../chesscom/chesscom.grpc.pb.h"

class Messenger {
    public:
    static void SendServerMessage(const std::string& revicerToken, const std::string& message);
    static void SendMessage(std::string& revicerToken, std::string& senderUername, std::string& message);
    static void SendMessage(const std::string& revicerToken, const std::string& senderUername, const std::string& message);
    static void Log(const std::string& message);
    private:
    //static std::mutex _logMutex; //static requires init on messanger.cpp
    
};
