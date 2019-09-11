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

#define MAJOR_VER "0"
#define MINOR_VER "2"
#define BUILD_VER "2"
#define VERSION MAJOR_VER "." MINOR_VER "." BUILD_VER

struct MatchStruct{
    //bool newUpdate = false;
    bool askingForDraw = false;
    //chesscom::MatchEvent matchEvent = chesscom::MatchEvent::Non;
    std::vector<chesscom::MatchEvent> matchEvents;
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
std::map<std::string, std::shared_ptr<MatchStruct>> matches;
chesscom::VisionRules serverVisionRules;


void SigintHandler (int param)
{
    signaled = 1;
    std::cout << "Signal interupt. Fuck yeah\n";
    logFile << "signal interut shutdown\n";
    auto deadline = std::chrono::system_clock::now() +   std::chrono::milliseconds(SHUTDOWN_WAIT_MS);
    server->Shutdown(deadline);
}

std::string createMatch(std::string& player1Token, std::string& player2Token){
    std::string matchToken = "match"+ std::to_string(tokenCounter++);
    std::shared_ptr<MatchStruct> match = std::make_shared<MatchStruct>();
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
    matches[matchToken] = match;
    std::cout << "  checing match" << std::endl << std::flush;
    auto matPtr = matches[matchToken];
    std::cout << "  white player " <<  matPtr->whitePlayer << std::endl << std::flush;
    return matchToken;
}

class ChessComImplementation final : public chesscom::ChessCom::Service {
    Status Login(ServerContext* context, const chesscom::LoginForm* request, chesscom::LoginResult* response) override 
    {
        response->set_usertoken("usertoken"+std::to_string(tokenCounter++));
        response->set_successfulllogin(true);
        std::cout << "User " << request->username() << " " << response->usertoken() << " logged in" << std::endl << std::flush;
        return Status::OK;
    }

    Status LookForMatch(ServerContext* context, const chesscom::UserIdentity* request, chesscom::LookForMatchResult* response) override 
    {
        bool loop = true;
        std::string matchToken = "";
        std::string userToken = request->usertoken();
        std::cout << userToken << " looking for match"<< std::endl << std::flush;
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
                std::cout << userToken << " entering queue"<< std::endl << std::flush;
                lookingForMatchQueue.emplace(userToken);
            }
        }
        while(loop){
            std::this_thread::sleep_for(std::chrono::milliseconds(MAX_SLEEP_MS));
            if(context->IsCancelled()) return grpc::Status::CANCELLED;
            {
                std::unique_lock<std::mutex> scopeLock (lock);
                std::cout << userToken << " checking foundMatchReply " << std::flush << std::to_string(foundMatchReply.count(userToken)) << std::endl << std::flush;
                if(foundMatchReply.count(userToken) > 0){
                    std::cout << userToken << " found match reply MT: " << foundMatchReply[userToken]<< std::endl << std::flush;
                    matchToken = foundMatchReply[userToken];
                    foundMatchReply.erase(userToken);
                    loop = false;
                }
            }
        }
        {
            std::unique_lock<std::mutex> scopeLock (lock);
            std::cout << userToken << " found match " <<  matchToken << std::endl << std::flush;
            std::cout << "  checing match" << std::endl << std::flush;
            auto matPtr = matches[matchToken];
            std::cout << "  white player " <<  matPtr->whitePlayer << std::endl << std::flush;
        }
        response->set_succes(true);
        response->set_matchtoken(matchToken);
        response->set_iswhiteplayer(matches[matchToken]->whitePlayer == userToken);
        chesscom::VisionRules* vrPtr = response->mutable_rules();
        vrPtr->CopyFrom(serverVisionRules);
        //dsa = serverVisionRules;
        return Status::OK;
    }

    void MatchReadLoop(ServerContext* context, std::shared_ptr<MatchStruct> matchPtr, grpc::ServerReaderWriter< chesscom::MoveResult, chesscom::MovePacket>* stream){
        chesscom::MovePacket movePkt;
        bool keepRunning = true;
        while (!context->IsCancelled() && keepRunning)
        {
            if(!stream->Read(&movePkt)){
                //throw "premeture end of steam"
                std::cout  << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " MatchStreamCanceled in read thread " << std::endl << std::flush;
                break;
            }
            if(movePkt.askingfordraw())throw "draw not implemented";
            std::shared_ptr<chesscom::Move> movePtr = std::make_shared<chesscom::Move>();
            if(movePkt.doingmove()){
                movePtr->set_from(movePkt.move().from());
                movePtr->set_to(movePkt.move().to());
            }
            
            switch (movePkt.cheatmatchevent())
            {
            case chesscom::MatchEvent::Non:
                std::cout  << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " Got move " << movePkt.move().from() << " " << movePkt.move().to() << std::endl << std::flush;
                {
                    std::unique_lock<std::mutex> scopeLock (lock);
                    bool isPlayersCurrentTurn = matchPtr->moves.size()%2 == (matchPtr->whitePlayer == movePkt.usertoken()?0:1);
                    if(isPlayersCurrentTurn){
                        matchPtr->moves.push_back(movePtr);
                        matchPtr->matchEvents.push_back(chesscom::MatchEvent::Non);
                    }else{
                        std::cout  << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " ERROR: not this players turn to move" << std::endl << std::flush;
                    }
                   
                }
                break;
            case chesscom::MatchEvent::UnexpectedClosing:
                {
                    std::unique_lock<std::mutex> scopeLock (lock);
                    //if(movePkt.doingmove()){
                    //    std::cout  << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " Got move " << movePkt.move().from() << " " << movePkt.move().to() << std::endl << std::flush;
                    //    matchPtr->moves.push_back(movePtr);
                    //}else{
                        //matchPtr->moves.push_back(std::make_shared<chesscom::Move>());
                    //}
                    std::cout << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " Got UnexpectedClosing" << std::endl << std::flush;
                    matchPtr->matchEvents.push_back(chesscom::MatchEvent::UnexpectedClosing);
                    //matchPtr->newUpdate = true;
                    keepRunning = false;
                    break;
                }
                break;
            case chesscom::MatchEvent::WhiteWin:
            case chesscom::MatchEvent::BlackWin:
                {
                    std::unique_lock<std::mutex> scopeLock (lock);
                    std::cout  << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " Got move " << movePkt.move().from() << " " << movePkt.move().to() << std::endl << std::flush;
                    matchPtr->moves.push_back(movePtr);
                    std::cout  << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " Got Win" << std::endl << std::flush;
                    matchPtr->matchEvents.push_back(movePkt.cheatmatchevent());
                    keepRunning = false;
                    break;
                }
                break;
            default:
                throw "fuck fwafdw";
                break;
            }
            std::cout  << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " ReadThread End" << std::endl << std::flush;
        }
        
    }

    Status Match(ServerContext* context, grpc::ServerReaderWriter< chesscom::MoveResult, chesscom::MovePacket>* stream) override 
    {
        chesscom::MovePacket movePkt;
        chesscom::MoveResult moveResultPkt;
        stream->Read(&movePkt);
        if(movePkt.doingmove()){
            std::cout << "Error: doing move as first " << movePkt.usertoken() << ". Terminating!" << std::endl << std::flush;
            return Status::OK; 
        }
        std::cout << "Opening matchstream for " << movePkt.matchtoken() << " " <<  movePkt.usertoken() << std::endl << std::flush;
        std::string userToken = movePkt.usertoken();
        std::shared_ptr<MatchStruct> matchPtr = matches[movePkt.matchtoken()];
        std::cout  << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " Starting read thread " << std::endl << std::flush;
        std::thread t1([this, context, matchPtr, stream](){
            this->MatchReadLoop(context, matchPtr, stream);
        });
        bool playerWhite = matchPtr->whitePlayer == userToken;
        bool loop = true;
        int lastEventNum = 0;
        while (loop)
        {
            if(context->IsCancelled()) {
                std::cout  << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " Matchstream cancelled " << std::endl << std::flush;
                t1.join();
                return grpc::Status::OK;
            }
            //bool isPlayersCurrentTurn;
            //{
            //    std::unique_lock<std::mutex> scopeLock (lock);
            //    isPlayersCurrentTurn = matchPtr->moves.size()%2 == (playerWhite?0:1); 
            //}
            //std::cout  << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " IsCurrentTurn "<< std::to_string(isPlayersCurrentTurn) << std::endl << std::flush;
            //if(isPlayersCurrentTurn)
            {

            }
            bool isUpdate;
            {
                std::unique_lock<std::mutex> scopeLock (lock);
                isUpdate = matchPtr->matchEvents.size() > lastEventNum;
                if(isUpdate){
                    if(matchPtr->matchEvents[lastEventNum] == chesscom::MatchEvent::UnexpectedClosing)
                    {
                        std::cout  << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " Opponent UnexpectedClosing" << std::endl << std::flush;
                        moveResultPkt.set_movehappned(false);
                        moveResultPkt.set_opponentaskingfordraw(false);
                        //moveResultPkt.set_allocated_move(matchPtr->moves.back().get());
                        moveResultPkt.set_matchevent(chesscom::MatchEvent::UnexpectedClosing);
                        stream->Write(moveResultPkt);
                        loop = false;
                        continue;
                    }
                    else if(matchPtr->matchEvents[lastEventNum] == chesscom::MatchEvent::WhiteWin || matchPtr->matchEvents[lastEventNum] == chesscom::MatchEvent::BlackWin)
                    {
                        std::cout  << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " Someone won!! " << matchPtr->matchEvents[lastEventNum] << std::endl << std::flush;
                        moveResultPkt.set_movehappned(true);
                        moveResultPkt.set_opponentaskingfordraw(false);
                        moveResultPkt.set_allocated_move(matchPtr->moves.back().get());
                        moveResultPkt.set_matchevent(matchPtr->matchEvents[lastEventNum]);
                        stream->Write(moveResultPkt);
                        moveResultPkt.release_move();
                        loop = false;
                        continue;
                    }
                    else
                    {
                        moveResultPkt.set_movehappned(true);
                        moveResultPkt.set_opponentaskingfordraw(false);
                        moveResultPkt.set_allocated_move(matchPtr->moves.back().get());
                        moveResultPkt.set_matchevent(chesscom::MatchEvent::Non);
                        stream->Write(moveResultPkt);
                        moveResultPkt.release_move();
                        std::cout  << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " SentMoveResult " << std::endl << std::flush;
                    }
                    lastEventNum++;
                }
            }
             std::this_thread::sleep_for(std::chrono::milliseconds(MAX_SLEEP_MS));

        }
        t1.join();
        std::cout  << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " Matchstream ended." << std::endl << std::flush;
        return Status::OK;
    }
};

chesscom::VisionRules ServerVisionRules(){
    chesscom::VisionRules vr;
    vr.set_enabled(true);
    vr.set_viewmovefields(false);
    vr.set_viewrange(2);
    //std::cout << " Vision rules" << std::endl << std::flush;
    google::protobuf::Map<int, chesscom::VisionRules>* override = vr.mutable_piceoverwriter();
    chesscom::VisionRules special;
    special.set_enabled(true);
    special.set_viewmovefields(false);
    special.set_viewrange(1);
    //std::cout << " redy to mute" << std::endl << std::flush;
    (*override)[chesscom::Pices::BlackPawn] = special;
    (*override)[chesscom::Pices::WhitePawn] = special;
    return vr;
}

void Run() {
    std::string address("0.0.0.0:43326");
    ChessComImplementation service;

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
    void (*prev_handler)(int);
    prev_handler = signal(SIGINT, SigintHandler);
    logFile.open ("server.log", std::ios::out | std::ios::trunc);
    logFile << "Writing this to a file.\n"<< std::flush;
    serverVisionRules = ServerVisionRules();
    std::string vrString = serverVisionRules.SerializeAsString();
    //logFile << "---ServerRules---\n" << vrString << "\n";
    //std::cout << "---ServerRules---\n" << vrString << std::endl << std::flush;
    Run();
    logFile << "Exiting\n";
    logFile.close();
    return 0;
}
