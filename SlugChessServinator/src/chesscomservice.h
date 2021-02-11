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
#include <memory>
#include <google/protobuf/util/time_util.h>


#include <grpcpp/grpcpp.h>
#include "../chesscom/chesscom.grpc.pb.h"
#include "../../SlugChessCore/src/slugchess.h"
#include "version.h"
#include "match.h"
#include "consts.h"
#include "messenger.h"
#include "gamebrowser.h"
#include "matchmanager.h"
#include "usermanager.h"
#include "userstore.h"
#include "slugchesscoverter.h"
#include "filesystem.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;


class ChessComService final : public chesscom::ChessCom::Service {
public:
    
    //string is userToken
    
    std::mutex lock;
    std::atomic<int> tokenCounter;
    std::queue<std::string> lookingForMatchQueue;
    std::map<std::string, std::string> foundMatchReply;
    //std::map<std::string, std::shared_ptr<::Match>> matches;

    ChessComService();
    ~ChessComService();
    grpc::Status RegisterUser(ServerContext* context, const chesscom::RegiserUserForm* request, chesscom::RegiserUserFormResult* response) override;
    grpc::Status Login(ServerContext* context, const chesscom::LoginForm* request, chesscom::LoginResult* response) override;
    grpc::Status GetNamedVariants(ServerContext* context, const chesscom::Void* request, chesscom::NamedVariants* response) override;
    
    grpc::Status Logout(grpc::ServerContext* context, const chesscom::UserIdentification* request, chesscom::LogoutResult* response) override;
    grpc::Status LookForMatch(ServerContext* context, const chesscom::UserIdentification* request, chesscom::LookForMatchResult* response) override;
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
    grpc::Status ProcessReplay(grpc::ServerContext* context,  const chesscom::ReplayRequest* request, chesscom::Replay* response) override;
    grpc::Status ServerVisionRulesets(grpc::ServerContext* context, const chesscom::Void *request, chesscom::VisionRuleset* response) override;
    grpc::Status GetPublicUserdata(grpc::ServerContext* context, const chesscom::UserDataRequest* request, chesscom::UserData* response) override;

    void ChatMessageStreamLoop(ServerContext* context, std::string& usertoken, grpc::ServerReaderWriter< chesscom::ChatMessage, chesscom::ChatMessage>* stream);
    void MatchReadLoop(ServerContext* context, std::shared_ptr<::Match> matchPtr, grpc::ServerReaderWriter< chesscom::MoveResult, chesscom::MovePacket>* stream);

};
