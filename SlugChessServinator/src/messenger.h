#pragma once
#include "map"
#include "../chesscom/chesscom.grpc.pb.h"

class Messenger {
    public:
    void SendServerMessage(std::string& revicerToken, std::string& message)
    {
        std::string sender =  "server";
        SendMessage(revicerToken, sender, message);
    }
    void SendMessage(std::string& revicerToken, std::string& senderUername, std::string& message)
    {
        std::unique_lock<std::mutex> scopeLock (_messageStreamsMutex);
        if(_messageStreams.count(revicerToken) > 0)
        {
            chesscom::ChatMessage msg;
            msg.set_allocated_message(&message);
            msg.set_allocated_sender_usertoken(&revicerToken);
            msg.set_allocated_sender_username(&senderUername);
            _messageStreams[revicerToken]->Write(msg);
            msg.release_message();
            msg.release_reciver_usertoken();
            msg.release_sender_username();
        }
        
    }
    void SendMessage(const std::string& revicerToken, const std::string& senderUername, const std::string& message)
    {
        std::unique_lock<std::mutex> scopeLock (_messageStreamsMutex);
        if(_messageStreams.count(revicerToken) > 0)
        {
            chesscom::ChatMessage msg;
            msg.set_message(message);
            msg.set_sender_usertoken(revicerToken);
            msg.set_sender_username(senderUername);
            _messageStreams[revicerToken]->Write(msg);
        }
        
    }
    void AddMessageStream(const std::string& userToken, grpc::internal::WriterInterface< chesscom::ChatMessage>* stream)
    {
        std::unique_lock<std::mutex> scopeLock (_messageStreamsMutex);
        _messageStreams[userToken] = stream;
    }
    void RemoveMessageStream(const std::string& userToken)
    {
        std::unique_lock<std::mutex> scopeLock (_messageStreamsMutex);
        _messageStreams.erase(userToken);
    }
    private:
    std::mutex _messageStreamsMutex;
    std::map<std::string, grpc::internal::WriterInterface<chesscom::ChatMessage>*> _messageStreams;
};