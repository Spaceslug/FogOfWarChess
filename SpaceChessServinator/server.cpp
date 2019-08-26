#include <string>
#include <iostream>
#include <csignal>

#include <grpcpp/grpcpp.h>
#include "chesscom.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

//using chesscom::ChessCom;
//using chesscom::MathRequest;
//using chesscom::MathReply;
//using chesscom::ChessCom;

std::sig_atomic_t signaled = 0;
std::unique_ptr<Server> server;

std::unordered_map<std::string, std::string> userTokens;

void SigintHandler (int param)
{
    signaled = 1;
    std::cout << "signal interupt. Fuck yeah";
    server->Shutdown();
}

class ChessComImplementation final : public chesscom::ChessCom::Service {
    Status sendRequest(
        ServerContext* context, 
        const chesscom::MathRequest* request, 
        chesscom::MathReply* responce
    ) override {
        int a = request->a();
        int b = request->b();
        std::cout << "Doing multi";

        responce->set_result(a * b);

        return Status::OK;
    } 

    Status Login(
        ServerContext* context,
        const chesscom::LoginForm* request,
        chesscom::LoginResult* responce
    ) override {
        return Status::OK;
    }

    Status LookForMatch(ServerContext* context, const chesscom::UserIdentity* request, chesscom::LookForMatchResult* response) override 
    {
        
        return Status::OK;
    }

    Status Match(ServerContext* context, grpc::ServerReaderWriter< chesscom::MoveResult, chesscom::MovePacket>* stream) override 
    {
        return Status::OK;
    }


};

void Run() {
    std::string address("0.0.0.0:43326");
    ChessComImplementation service;

    ServerBuilder builder;

    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    server = builder.BuildAndStart();
    std::cout << "Chess Server listening on port: " << address << std::endl;

    server->Wait();
    std::cout << "After wait happened";
    
}

int main(int argc, char** argv) {
    void (*prev_handler)(int);
    prev_handler = signal(SIGINT, SigintHandler);
    Run();

    return 0;
}
