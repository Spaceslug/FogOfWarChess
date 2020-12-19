#include "messenger.h"
#include "usermanager.h"

void Messenger::SendServerMessage(const std::string& revicerToken, const std::string& message)
{
    std::string sender =  "server";
    SendMessage(revicerToken, sender, message);
}

void Messenger::SendMessage(std::string& revicerToken, std::string& senderUername, std::string& message)
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

void Messenger::SendMessage(const std::string& revicerToken, const std::string& senderUername, const std::string& message)
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

void Messenger::Log(const std::string& message)
{
    //Log(std::make_shared<std::string>(message));
    //std::lock_guard<std::mutex> lock(_logMutex);
    std::cout << message << std::endl << std::flush;
}