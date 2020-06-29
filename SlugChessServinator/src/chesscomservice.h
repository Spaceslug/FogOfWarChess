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
#include <chrono>
#include <thread>
#include <condition_variable>
#include <google/protobuf/util/time_util.h>


#include <grpcpp/grpcpp.h>
#include "../chesscom/chesscom.grpc.pb.h"
#include "../../SlugChessCore/src/slugchess.h"
#include "version.h"
#include "match.h"
#include "messenger.h"
#include "gamebrowser.h"
#include "matchmanager.h"
#include "usermanager.h"
#include "slugchesscoverter.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;


class ChessComService final : public chesscom::ChessCom::Service {
public:
    static const int MAX_SLEEP_MS;
    static const int SHUTDOWN_WAIT_MS;
    
    //string is userToken
    
    std::mutex lock;
    std::atomic<int> tokenCounter;
    std::queue<std::string> lookingForMatchQueue;
    std::map<std::string, std::string> foundMatchReply;
    //std::map<std::string, std::shared_ptr<::Match>> matches;

    ChessComService();
    grpc::Status Login(ServerContext* context, const chesscom::LoginForm* request, chesscom::LoginResult* response) override;
    grpc::Status LookForMatch(ServerContext* context, const chesscom::UserIdentity* request, chesscom::LookForMatchResult* response) override;
    grpc::Status Match(ServerContext* context, grpc::ServerReaderWriter< chesscom::MoveResult, chesscom::MovePacket>* stream) override;
    grpc::Status MatchEventListener(grpc::ServerContext *context, const chesscom::MatchObserver* request, grpc::ServerWriter<chesscom::MoveResult> *writer) override;
    grpc::Status SendMove(grpc::ServerContext* context, const chesscom::MovePacket* request, chesscom::Void* response) override;
    grpc::Status ChatMessageStream(ServerContext* context, grpc::ServerReaderWriter< chesscom::ChatMessage, chesscom::ChatMessage>* stream) override;
    grpc::Status HostGame(grpc::ServerContext *context, const chesscom::HostedGame *request, chesscom::LookForMatchResult *response) override;
    grpc::Status AvailableGames(grpc::ServerContext *context, const chesscom::Void *request, chesscom::HostedGamesMap *response) override;
    grpc::Status JoinGame(grpc::ServerContext *context, const chesscom::JoinGameRequest *request, chesscom::LookForMatchResult *response) override;
    grpc::Status Alive(grpc::ServerContext* context, const chesscom::Heartbeat* request, chesscom::Heartbeat* response) override;
    grpc::Status ChatMessageListener(grpc::ServerContext* context, const chesscom::UserData* request, grpc::ServerWriter< ::chesscom::ChatMessage>* writer) override;
    grpc::Status SendChatMessage(grpc::ServerContext* context, const chesscom::ChatMessage* request, chesscom::Void* response) override;

    void ChatMessageStreamLoop(ServerContext* context, std::string& usertoken, grpc::ServerReaderWriter< chesscom::ChatMessage, chesscom::ChatMessage>* stream);
    void MatchReadLoop(ServerContext* context, std::shared_ptr<::Match> matchPtr, grpc::ServerReaderWriter< chesscom::MoveResult, chesscom::MovePacket>* stream);

};
