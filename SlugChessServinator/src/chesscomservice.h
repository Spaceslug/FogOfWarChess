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
    Messenger messenger;
    GameBrowser gameBrowser;
    MatchManager matchManager;
    UserManager userManager;

    ChessComService();
    Status Login(ServerContext* context, const chesscom::LoginForm* request, chesscom::LoginResult* response) override;
    Status LookForMatch(ServerContext* context, const chesscom::UserIdentity* request, chesscom::LookForMatchResult* response) override;
    void MatchReadLoop(ServerContext* context, std::shared_ptr<::Match> matchPtr, grpc::ServerReaderWriter< chesscom::MoveResult, chesscom::MovePacket>* stream);
    Status Match(ServerContext* context, grpc::ServerReaderWriter< chesscom::MoveResult, chesscom::MovePacket>* stream) override;
    void ChatMessageStreamLoop(ServerContext* context, std::string& usertoken, grpc::ServerReaderWriter< chesscom::ChatMessage, chesscom::ChatMessage>* stream);
    Status ChatMessageStream(ServerContext* context, grpc::ServerReaderWriter< chesscom::ChatMessage, chesscom::ChatMessage>* stream) override;
    grpc::Status HostGame(grpc::ServerContext *context, const chesscom::HostedGame *request, chesscom::LookForMatchResult *response) override;
    grpc::Status AvailableGames(grpc::ServerContext *context, const chesscom::Void *request, chesscom::HostedGamesMap *response) override;
    grpc::Status JoinGame(grpc::ServerContext *context, const chesscom::JoinGameRequest *request, chesscom::LookForMatchResult *response) override;
    grpc::Status Alive(grpc::ServerContext* context, const chesscom::Heartbeat* request, chesscom::Heartbeat* response) override;
};
