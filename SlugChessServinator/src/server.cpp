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
#include "../chesscom/chesscom.grpc.pb.h"
#include "../../SlugChessCore/src/slugchess.h"
#include "version.h"
#include "chesscomservice.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

std::atomic<bool> orderedShutdown(false);
std::sig_atomic_t signaled = 0;
std::unique_ptr<Server> server;
static std::ofstream logFile;
static std::ofstream cfgFile;


void SigintHandler (int param)
{
    signaled = 1;
    std::cout << "Signal interupt. Fuck yeah\n";
    logFile << "signal interut shutdown\n";
    auto deadline = std::chrono::system_clock::now() +   std::chrono::milliseconds(ChessComService::SHUTDOWN_WAIT_MS);
    server->Shutdown(deadline);
}


void Run(std::string port) {
    //Booting up singeltons
    MatchManager::Get();
    UserManager::Get();
    GameBrowser::Get();

    std::string address("0.0.0.0:" + port);
    ChessComService service;

    ServerBuilder builder;

    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::cout << "SlugChess Server version " VERSION " starting" << std::endl;
    server = builder.BuildAndStart();
    std::cout << "SlugChess Server listening on port: " << address << std::endl;

    server->Wait();
    std::cout << "After wait happened" << std::endl;
    
}

int main(int argc, char** argv) {
    //TODO: reenable signal handling
    //void (*prev_handler)(int);
    //prev_handler = signal(SIGINT, SigintHandler);
    logFile.open ("server.log", std::ios::out | std::ios::trunc);
    logFile << "Writing this to a file.\n"<< std::flush;
    
    //logFile << "---ServerRules---\n" << vrString << "\n";
    //std::cout << "---ServerRules---\n" << vrString << std::endl << std::flush;
    std::string port = "43326";
    if(argc == 2){
        port = argv[1];
        std::cout << "Custom port '" << port << "'" << std::flush << std::endl;
    }
    Run(port);
    logFile << "Exiting\n";
    logFile.close();
    return 0;
}
