#include <string>
#include <iostream>
#include <csignal>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <mutex>
#include <random>
#include <chrono>
#include <thread>

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


struct Match{
    std::string whitePlayer;
    std::string blackPlayer;
    std::string matchToken;
    std::vector<std::shared_ptr<chesscom::Move>> moves;
    std::string& getWhitePlayer(){return whitePlayer;}
    std::string& getBlackPlayer(){return blackPlayer;}
    std::string& getMatchToken(){return matchToken;}
    
};

static const int MAX_SLEEP_MS = 1000;
static const int SHUTDOWN_WAIT_MS = MAX_SLEEP_MS*2.5;

std::atomic<int> tokenCounter(1);
std::atomic<bool> orderedShutdown(false);
std::sig_atomic_t signaled = 0;
std::mutex lock;
std::unique_ptr<Server> server;
static std::unordered_map<std::string, std::string> userTokens;
static std::ofstream logFile;
std::queue<std::string> lookingForMatchQueue;
std::map<std::string, std::string> foundMatchReply;
std::map<std::string, std::shared_ptr<Match>> matches;



void SigintHandler (int param)
{
    signaled = 1;
    std::cout << "Signal interupt. Fuck yeah\n";
    logFile << "signal interut shutdown\n";

    server->Shutdown();
}

std::string createMatch(std::string& player1Token, std::string& player2Token){
    std::string matchToken = "match"+ tokenCounter++;
    std::shared_ptr<Match> match = std::make_shared<Match>();
    match->matchToken = matchToken;
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(0, 1);
    if(dist(mt) == 1)
    {
        match->whitePlayer = player1Token;
        match->blackPlayer = player2Token;
    }
    else
    {
        match->whitePlayer = player2Token;
        match->blackPlayer = player1Token;
    }
    
    return matchToken;
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

    Status Login(ServerContext* context, const chesscom::LoginForm* request, chesscom::LoginResult* response) override 
    {
        response->set_usertoken("usertoken"+tokenCounter++);
        response->set_successfulllogin(true);

        return Status::OK;
    }

    Status LookForMatch(ServerContext* context, const chesscom::UserIdentity* request, chesscom::LookForMatchResult* response) override 
    {
        bool loop = true;
        std::string matchToken = "";
        std::string userToken = request->usertoken();
        {
            std::unique_lock<std::mutex> scopeLock (lock);
            if(!lookingForMatchQueue.empty()){
                std::string opponent = lookingForMatchQueue.front();
                lookingForMatchQueue.pop();
                matchToken = createMatch(userToken, opponent);
                foundMatchReply[opponent] = matchToken;
                loop = false;
            }
            else
            {
                lookingForMatchQueue.emplace(userToken);
            }
        }
        while(loop){
            std::this_thread::sleep_for(std::chrono::milliseconds(MAX_SLEEP_MS));
            {
                std::unique_lock<std::mutex> scopeLock (lock);
                if(foundMatchReply.count(userToken) > 0){
                    matchToken = foundMatchReply[userToken];
                    foundMatchReply.erase(userToken);
                    loop = false;
                }
            }
        }
        response->set_succes(true);
        response->set_matchtoken(matchToken);
        response->set_iswhiteplayer(matches[matchToken]->whitePlayer == userToken);
        return Status::OK;
    }

    Status Match(ServerContext* context, grpc::ServerReaderWriter< chesscom::MoveResult, chesscom::MovePacket>* stream) override 
    {
        chesscom::MovePacket movePkt;
        stream->Read(&movePkt);
        //movePkt.

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
    logFile.open ("server.log", std::ios::out | std::ios::trunc);
    logFile << "Writing this to a file.\n";
    Run();
    logFile << "Exiting\n";
    logFile.close();
    return 0;
}
