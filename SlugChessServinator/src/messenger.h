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
        chesscom::ChatMessage msg;
        msg.set_allocated_message(&message);
        msg.set_allocated_reciverusertoken(&revicerToken);
        msg.set_allocated_senderusername(&senderUername);
        _messageStreams[revicerToken]->Write(msg);
        msg.release_message();
        msg.release_reciverusertoken();
        msg.release_senderusername();
    }
    void AddMessageStream(const std::string& userToken, grpc::ServerReaderWriter< chesscom::ChatMessage, chesscom::ChatMessage>* stream)
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
    std::map<std::string, grpc::ServerReaderWriter< chesscom::ChatMessage, chesscom::ChatMessage>*> _messageStreams;
};