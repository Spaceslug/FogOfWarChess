
#include <string>
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
#include <filesystem>
#include <thread>
#include <condition_variable>
#include <google/protobuf/util/time_util.h>

#include <grpcpp/grpcpp.h>
#include "../chesscom/chesscom.grpc.pb.h"
#include "../../SlugChessCore/src/slugchess.h"

#include "version.h"
#include "consts.h"
#include "chesscomservice.h"
#include "worker.h"
#include "filesystem.h"

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
    Worker::Stop();
    auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(SHUTDOWN_WAIT_MS);
    server->Shutdown(deadline);
    //std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    Worker::Join();
}


void Run(std::string port) {
    //Booting up singeltons
    Worker::Start(); //Kick of work thread.
    MatchManager::Get();
    UserManager::Get();
    GameBrowser::Get();
    //Add summart every 10 minutes
    Worker::AddWork(std::make_shared<Work>("summary", std::chrono::minutes(1), []()
    {
        time_t curettTime = time(0);
        std::cout << "Summary - " << ctime(&curettTime)
            << "\tCurrent users: " << std::to_string(UserManager::Get()->ActiveUsers()) << std::endl
            << "\tCurrent matches: " << std::to_string(MatchManager::Get()->OngoingMatches()) << std::endl
            << std::flush;   
    }));
    //Check heartbeat every 2 minutes
    Worker::AddWork(std::make_shared<Work>("heartbeat_check", std::chrono::minutes(2), []()
    {
        std::cout << "Checking Heartbeats" << std::endl << std::flush;
        UserManager::Get()->CheckHeartbeat();
    }));

    std::string address("0.0.0.0:" + port);
    ChessComService service;

    ServerBuilder builder;

    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::cout << "SlugChess Server version " VERSION " starting" << std::endl;
    server = builder.BuildAndStart();
    std::cout << "SlugChess Server listening on port: " << address << std::endl;

    server->Wait();
    std::cout << "After wait happened. Do cleanup" << std::endl;
    UserManager::DeleteInstance();
    
}

int main(int argc, char** argv) {
    //void (*prev_handler)(int);
    //prev_handler = 
    signal(SIGINT, SigintHandler);
    logFile.open ("server.log", std::ios::out | std::ios::trunc);
    logFile << "Writing this to a file.\n"<< std::flush;
    
    //logFile << "---ServerRules---\n" << vrString << "\n";
    std::cout << "SlugChess Server starting" << std::endl 
        << "Version " << VERSION << std::endl << std::flush;
    std::string port = "43326";
    if(argc == 2){
        port = argv[1];
        std::cout << "Custom port '" << port << "'" << std::flush << std::endl;
    }

    std::cout << "Root dir path  '" << Filesystem::RootDir() << "' " << std::flush << std::endl;
    Run(port);
    logFile << "Exiting\n";
    logFile.close();
    return 0;
}
