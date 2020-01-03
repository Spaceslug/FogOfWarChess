#pragma once
#include <string>
#include <iostream>
#include <csignal>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <vector>
#include <mutex>
#include <random>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <google/protobuf/util/time_util.h>


#include <grpcpp/grpcpp.h>
#include "chesscom.grpc.pb.h"
#include "slugchess.h"
#include "version.h"
#include "match.h"	

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;


class ChessComService final : public chesscom::ChessCom::Service {
public:
    static const int MAX_SLEEP_MS;
    static const int SHUTDOWN_WAIT_MS;
    
    std::mutex _messageStreamsMutex;
    //string is userToken
    std::map<std::string, grpc::ServerReaderWriter< chesscom::ChatMessage, chesscom::ChatMessage>*> _messageStreams;
    std::mutex lock;
    std::atomic<int> tokenCounter;
    std::unordered_map<std::string, std::string> userTokens;
    std::queue<std::string> lookingForMatchQueue;
    std::map<std::string, std::string> foundMatchReply;
    std::map<std::string, std::shared_ptr<::Match>> matches;
    VisionRules serverVisionRules;
    chesscom::TimeRules serverTimeRules;

    chesscom::TimeRules ServerTimeRules()
    {
        chesscom::TimeRules tr;
        tr.mutable_playertime()->set_minutes(5);
        tr.mutable_playertime()->set_seconds(0);
        tr.set_secondspermove(6);

        return tr;
    }

    VisionRules ServerVisionRules() 
    {
        VisionRules rules;
        rules.globalRules = Rules();
        rules.globalRules.ViewCaptureField = true;
        rules.globalRules.ViewMoveFields = false;
        rules.globalRules.ViewRange = 2;
        rules.enabled = true;
        rules.overWriteRules[ChessPice::WhitePawn] = Rules(false, true, 1);
        rules.overWriteRules[ChessPice::BlackPawn] = Rules(false, true, 1);
	
        return rules;
    }
std::string createMatch(std::string& player1Token, std::string& player2Token){
    std::string matchToken = "match"+ std::to_string(tokenCounter++);
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(0, 1);
    std::string white;
    std::string black; 
    if(dist(mt) == 1)
    {
        white = player1Token;
        black = player2Token;
    }
    else
    {
        white = player2Token;
        black = player1Token;
    }
    std::shared_ptr<::Match> match = std::make_shared<::Match>(matchToken, white, black,	 
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w AHah - 0 1", serverVisionRules);

    match->clock->blackSecLeft = serverTimeRules.playertime().minutes() * 60 + serverTimeRules.playertime().seconds();
    match->clock->whiteSecLeft = match->clock->blackSecLeft;
    matches[matchToken] = match;
    std::cout << "  checing match " << " black sec left " << std::to_string(match->clock->blackSecLeft) << " white sec left " << std::to_string(match->clock->whiteSecLeft) << std::endl << std::flush;
    auto matPtr = matches[matchToken];
    std::cout << "  white player " <<  matPtr->whitePlayer << std::endl << std::flush;
    return matchToken;
}
    ChessComService();
    Status Login(ServerContext* context, const chesscom::LoginForm* request, chesscom::LoginResult* response) override;
    Status LookForMatch(ServerContext* context, const chesscom::UserIdentity* request, chesscom::LookForMatchResult* response) override;
    void MatchReadLoop(ServerContext* context, std::shared_ptr<::Match> matchPtr, grpc::ServerReaderWriter< chesscom::MoveResult, chesscom::MovePacket>* stream);
    Status Match(ServerContext* context, grpc::ServerReaderWriter< chesscom::MoveResult, chesscom::MovePacket>* stream) override;
    void SendChatMessage(std::string& senderUsername, std::string& reciverUsertoken, std::string& message);
    void ChatMessageStreamLoop(ServerContext* context, std::string& usertoken, grpc::ServerReaderWriter< chesscom::ChatMessage, chesscom::ChatMessage>* stream);
    Status ChatMessageStream(ServerContext* context, grpc::ServerReaderWriter< chesscom::ChatMessage, chesscom::ChatMessage>* stream) override;
};
