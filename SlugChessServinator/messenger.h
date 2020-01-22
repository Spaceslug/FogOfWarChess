#pragma once
#include "map"
#include "chesscom.grpc.pb.h"

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
        chesscom::ChatMessage msg;
        msg.set_allocated_message(&message);
        msg.set_allocated_reciver(&revicerToken);
        msg.set_allocated_sender(&senderUername);
        _messageStreams[revicerToken]->Write(msg);
        msg.release_message();
        msg.release_reciver();
        msg.release_sender();
    }
    void AddMessageStream(std::string& userToken, grpc::ServerReaderWriter< chesscom::ChatMessage, chesscom::ChatMessage>* stream)
    {
        std::unique_lock<std::mutex> scopeLock (_messageStreamsMutex);
        _messageStreams[userToken] = stream;
    }
    void RemoveMessageStream(std::string& userToken)
    {
        std::unique_lock<std::mutex> scopeLock (_messageStreamsMutex);
        _messageStreams.erase(userToken);
    }
    private:
    std::mutex _messageStreamsMutex;
    std::map<std::string, grpc::ServerReaderWriter< chesscom::ChatMessage, chesscom::ChatMessage>*> _messageStreams;
};