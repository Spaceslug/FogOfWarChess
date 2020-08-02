#pragma once
#include <map>
#include <memory>
#include "../chesscom/chesscom.grpc.pb.h"
#include "usermanager.h"

class Messenger {
    public:
    static void SendServerMessage(const std::string& revicerToken, const std::string& message)
    {
        std::string sender =  "server";
        SendMessage(revicerToken, sender, message);
    }
    static void SendMessage(std::string& revicerToken, std::string& senderUername, std::string& message)
    {
        if(UserManager::Get()->UsertokenLoggedIn(revicerToken)){
            User* user = UserManager::Get()->GetUser(revicerToken);
            std::unique_lock<std::mutex> scopeLock (user->messageStreamMutex);
            chesscom::ChatMessage msg;
            msg.set_allocated_message(&message);
            msg.set_allocated_sender_usertoken(&revicerToken);
            msg.set_allocated_sender_username(&senderUername);
            user->messageStream->Write(msg);
            msg.release_message();
            msg.release_reciver_usertoken();
            msg.release_sender_username();
        }
        
    }
    static void SendMessage(const std::string& revicerToken, const std::string& senderUername, const std::string& message)
    {
        if(UserManager::Get()->UsertokenLoggedIn(revicerToken)){
            User* user = UserManager::Get()->GetUser(revicerToken);
            std::unique_lock<std::mutex> scopeLock (user->messageStreamMutex);
            chesscom::ChatMessage msg;
            msg.set_message(message);
            msg.set_sender_usertoken(revicerToken);
            msg.set_sender_username(senderUername);
            user->messageStream->Write(msg);
        }
        
    }

    static void Log(const std::string& message)
    {
        Log(std::make_shared<std::string>(message));
    }

    static void Log(std::shared_ptr<std::string> message)
    {
        //TODO
        //if(_logMutex.try_lock())
        std::cout << *message << std::endl << std::flush;
    }

    private:
    static std::mutex _logMutex;
    
};
