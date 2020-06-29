#pragma once
#include "map"
#include "../chesscom/chesscom.grpc.pb.h"
#include "usermanager.h"

class Messenger {
    public:
    static void SendServerMessage(std::string& revicerToken, std::string& message)
    {
        std::string sender =  "server";
        SendMessage(revicerToken, sender, message);
    }
    static void SendMessage(std::string& revicerToken, std::string& senderUername, std::string& message)
    {
        if(UserManager::Get()->UsertokenLoggedIn(revicerToken)){
            User* user = UserManager::Get()->GetUser(revicerToken);
            std::unique_lock<std::mutex> scopeLock (user->_messageStreamMutex);
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
            std::unique_lock<std::mutex> scopeLock (user->_messageStreamMutex);
            chesscom::ChatMessage msg;
            msg.set_message(message);
            msg.set_sender_usertoken(revicerToken);
            msg.set_sender_username(senderUername);
            user->messageStream->Write(msg);
        }
        
    }
    
};