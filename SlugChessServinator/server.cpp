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
#include <google/protobuf/util/time_util.h>


#include <grpcpp/grpcpp.h>
#include "chesscom.grpc.pb.h"
#include "slugchess.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

//using chesscom::ChessCom;
//using chesscom::MathRequest;
//using chesscom::MathReply;
//using chesscom::ChessCom;

#define MAJOR_VER "0"
#define MINOR_VER "7"
#define BUILD_VER "0"
#define VERSION MAJOR_VER "." MINOR_VER "." BUILD_VER

struct ChessClock {
    int blackSecLeft;
    int whiteSecLeft;
};

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
    std::shared_ptr<ChessClock> clock;
    std::shared_ptr<SlugChess> game;
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
static std::ofstream cfgFile;
std::queue<std::string> lookingForMatchQueue;
std::map<std::string, std::string> foundMatchReply;
std::map<std::string, std::shared_ptr<MatchStruct>> matches;
VisionRules serverVisionRules;
chesscom::TimeRules serverTimeRules;

std::mutex _messageStreamsMutex;
//string is userToken
std::map<std::string, grpc::ServerReaderWriter< chesscom::ChatMessage, chesscom::ChatMessage>*> _messageStreams;


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
    match->clock = std::make_shared<ChessClock>();
    match->game = std::make_shared<SlugChess>("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w AHah - 0 1", serverVisionRules);
    match->clock->blackSecLeft = serverTimeRules.playertime().minutes() * 60 + serverTimeRules.playertime().seconds();
    match->clock->whiteSecLeft = match->clock->blackSecLeft;
    matches[matchToken] = match;
    std::cout << "  checing match " << " black sec left " << std::to_string(match->clock->blackSecLeft) << " white sec left " << std::to_string(match->clock->whiteSecLeft) << std::endl << std::flush;
    auto matPtr = matches[matchToken];
    std::cout << "  white player " <<  matPtr->whitePlayer << std::endl << std::flush;
    return matchToken;
}

class ChessComImplementation final : public chesscom::ChessCom::Service {
    Status Login(ServerContext* context, const chesscom::LoginForm* request, chesscom::LoginResult* response) override 
    {
        if(request->majorversion() == MAJOR_VER && request->minorversion() == MINOR_VER)
        {
            std::cout << "User " << request->majorversion() << " " << MAJOR_VER << " " << request->minorversion() << " " << MINOR_VER << " " << request->buildversion() << " " << BUILD_VER << std::endl << std::flush;
            if(request->buildversion() >= BUILD_VER)
            {
                
            }
            else
            {
                response->set_loginmessage("You should upgrade to latest version. Server version " VERSION ", Client version " + request->majorversion() + "." + request->minorversion() + "." + request->buildversion()+". You can find the latest version at 'http://spaceslug.no/slugchess/list.html'");
            }
            auto it = std::find_if(userTokens.begin(), userTokens.end(),
                    [request](const std::pair<std::string, std::string> &p) {
                        return p.second == request->username();
                    });

            if (it != userTokens.end()) {
                userTokens.erase(it);
            }
            std::string userToken = request->username() + "-" + std::to_string(tokenCounter++);
            userTokens[userToken] = request->username();

            std::cout << "User " << request->username() << " " << response->usertoken() << " logged in" << std::endl << std::flush;
            response->set_usertoken(userToken);
            response->set_successfulllogin(true);
        }
        else
        {
            response->set_successfulllogin(false);
            response->set_loginmessage("Login failed because version missmatch. Server version " VERSION ", Client version " + request->majorversion() + "." + request->minorversion() + "." + request->buildversion()+". You can find the latest version at 'http://spaceslug.no/slugchess/list.html'");
            
        }
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
                //std::cout << userToken << " checking foundMatchReply " << std::flush << std::to_string(foundMatchReply.count(userToken)) << std::endl << std::flush;
                if(foundMatchReply.count(userToken) > 0){
                    std::cout << userToken << " found match reply MT: " << foundMatchReply[userToken]<< std::endl << std::flush;
                    matchToken = foundMatchReply[userToken];
                    foundMatchReply.erase(userToken);
                    loop = false;
                }
            }
        }
        auto matPtr = matches[matchToken];
        {
            std::unique_lock<std::mutex> scopeLock (lock);
            std::cout << userToken << " found match " <<  matchToken << std::endl << std::flush;
            std::cout << "  checing match" << std::endl << std::flush;
            std::cout << "  white player " <<  matPtr->whitePlayer << std::endl << std::flush;
        }
        response->set_succes(true);
        response->set_matchtoken(matchToken);
        response->set_iswhiteplayer(matPtr->whitePlayer == userToken);
        response->set_opponentusername(userTokens[response->iswhiteplayer()?matPtr->blackPlayer:matPtr->whitePlayer]);
        
        chesscom::VisionRules* vrPtr = response->mutable_rules();
        vrPtr->set_enabled(serverVisionRules.enabled);
        vrPtr->set_viewmovefields(serverVisionRules.globalRules.ViewMoveFields);
        vrPtr->set_viewrange(serverVisionRules.globalRules.ViewRange);
        vrPtr->set_viewcapturefield(serverVisionRules.globalRules.ViewCaptureField);
        //std::cout << " Vision rules" << std::endl << std::flush;
        google::protobuf::Map<int, chesscom::VisionRules>* override = vrPtr->mutable_piceoverwriter();
        
        chesscom::VisionRules special;
        special.set_enabled(true);
        for (auto&& piceRulesPar : serverVisionRules.overWriteRules) {
            special.set_viewrange(piceRulesPar.second.ViewRange);
            special.set_viewmovefields(piceRulesPar.second.ViewMoveFields);
            (*override)[piceRulesPar.first] = special;
        }
        chesscom::TimeRules* trPtr = response->mutable_timerules();
        trPtr->CopyFrom(serverTimeRules);
        //dsa = serverVisionRules;
        return Status::OK;
    }

    void MatchReadLoop(ServerContext* context, std::shared_ptr<MatchStruct> matchPtr, grpc::ServerReaderWriter< chesscom::MoveResult, chesscom::MovePacket>* stream){
        chesscom::MovePacket movePkt;
        bool keepRunning = true;
        try{
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
                    movePtr->mutable_timestamp()->Swap(movePkt.mutable_move()->mutable_timestamp());
                    movePtr->set_secspent(movePkt.move().secspent());
                }
                
                switch (movePkt.cheatmatchevent())
                {
                case chesscom::MatchEvent::Non:
                    std::cout  << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " Got move " << movePkt.move().from() << " " << movePkt.move().to() << " secspent " << std::to_string(movePtr->secspent()) << std::endl << std::flush;
                    {
                        std::unique_lock<std::mutex> scopeLock (lock);
                        bool isWhitePlayer = matchPtr->whitePlayer == movePkt.usertoken();
                        bool isPlayersCurrentTurn = matchPtr->moves.size()%2 == (isWhitePlayer?0:1);
                        if(isPlayersCurrentTurn){
                            matchPtr->game->DoMove(movePtr->from(), movePtr->to());
                            auto ww = matchPtr->game->GetWhiteVision(); 
                            auto bw = matchPtr->game->GetBlackVision(); 
                            auto pices = matchPtr->game->GetPices(); 
                            *movePtr->mutable_whitevision() = {ww.begin(), ww.end()};
                            *movePtr->mutable_blackvision() = {bw.begin(), bw.end()};
                            *movePtr->mutable_pices() = {pices.begin(), pices.end()};
                            //MOves
                            auto avMoves = movePtr->mutable_availablemoves();
                            chesscom::FieldMoves fm;
                            auto fmRF = fm.mutable_list();
                            for (auto &&keyVal : *matchPtr->game->LegalMovesRef())
                            {
                                for (auto &&pos : keyVal.second)
                                {
                                    fmRF->Add(SlugChess::BP(keyVal.first));
                                }
                                (*avMoves)[SlugChess::BP(keyVal.first)].CopyFrom(fm);
                                fmRF->Clear();
                            }
                            
                            matchPtr->moves.push_back(movePtr);
                            if(isWhitePlayer){
                                matchPtr->clock->whiteSecLeft -= movePtr->secspent();
                                if(matchPtr->clock->whiteSecLeft <= 0) {
                                    matchPtr->matchEvents.push_back(chesscom::MatchEvent::BlackWin);
                                }else{
                                    matchPtr->matchEvents.push_back(chesscom::MatchEvent::Non); 
                                    matchPtr->clock->whiteSecLeft += (matchPtr->moves.size()==1?0:serverTimeRules.secondspermove());
                                }
                            }
                            else
                            {
                                matchPtr->clock->blackSecLeft -= movePtr->secspent();
                                if(matchPtr->clock->blackSecLeft <= 0) {
                                    matchPtr->matchEvents.push_back(chesscom::MatchEvent::WhiteWin);        
                                }else{
                                    matchPtr->matchEvents.push_back(chesscom::MatchEvent::Non); 
                                    matchPtr->clock->blackSecLeft += serverTimeRules.secondspermove();
                                }
                            }
                            
                            //std::cout  << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " whitesecs left " << std::to_string(matchPtr->clock->whiteSecLeft) << " blacksecs left " << std::to_string(matchPtr->clock->blackSecLeft) << std::endl << std::flush;
                            //std::string output;google::protobuf::util::MessageToJsonString(*movePtr, &output);
                            //matchPtr->
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
                    case chesscom::MatchEvent::ExpectedClosing:
                    std::cout << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " Got UnexpectedClosing" << std::endl << std::flush;
                    keepRunning = false;
                    break;
                case chesscom::MatchEvent::WhiteWin:
                case chesscom::MatchEvent::BlackWin:
                    {
                        std::unique_lock<std::mutex> scopeLock (lock);
                        std::cout << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " Got move " << movePkt.move().from() << " " << movePkt.move().to() << std::endl << std::flush;
                        matchPtr->game->DoMove(movePtr->from(), movePtr->to());
                        auto ww = matchPtr->game->GetWhiteVision(); 
                        auto bw = matchPtr->game->GetBlackVision(); 
                        auto pices = matchPtr->game->GetPices(); 
                        *movePtr->mutable_whitevision() = {ww.begin(), ww.end()};
                        *movePtr->mutable_blackvision() = {bw.begin(), bw.end()};
                        *movePtr->mutable_pices() = {pices.begin(), pices.end()};
                        //MOves
                        auto avMoves = movePtr->mutable_availablemoves();
                        chesscom::FieldMoves fm;
                        auto fmRF = fm.mutable_list();
                        for (auto &&keyVal : *matchPtr->game->LegalMovesRef())
                        {
                            for (auto &&pos : keyVal.second)
                            {
                                fmRF->Add(SlugChess::BP(keyVal.first));
                            }
                            (*avMoves)[SlugChess::BP(keyVal.first)].CopyFrom(fm);
                            fmRF->Clear();
                        }
                        matchPtr->moves.push_back(movePtr);
                        std::cout << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " Got Win" << std::endl << std::flush;
                        matchPtr->matchEvents.push_back(movePkt.cheatmatchevent());
                        //keepRunning = false;
                        break;
                    }
                    break;
                default:
                    throw "fuck fwafdw";
                    break;
                }
            }
            std::cout  << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " ReadThread End" << std::endl << std::flush;
        }
        catch(std::exception ex)
        {
            std::cout  << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " Gracefully exit read thread exception: " << ex.what() << std::endl << std::flush;
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
                        //continue;
                    }
                    else if(matchPtr->matchEvents[lastEventNum] == chesscom::MatchEvent::WhiteWin || matchPtr->matchEvents[lastEventNum] == chesscom::MatchEvent::BlackWin)
                    {
                        std::cout  << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " Someone won!! " << matchPtr->matchEvents[lastEventNum] << std::endl << std::flush;
                        moveResultPkt.set_movehappned(true);
                        moveResultPkt.set_opponentaskingfordraw(false);
                        moveResultPkt.set_allocated_move(matchPtr->moves.back().get());
                        moveResultPkt.set_matchevent(matchPtr->matchEvents[lastEventNum]);
                        bool whiteCurretMove = matchPtr->moves.size()%2 == 0;
                        //true means last move was from black
                        moveResultPkt.set_secsleft(whiteCurretMove?matchPtr->clock->blackSecLeft:matchPtr->clock->whiteSecLeft);
                        //grpc::WriteOptions options;
                        //std::cout  << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " IsBufferedHint " << options.get_buffer_hint() << std::endl << std::flush;
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
                        bool whiteCurretMove = matchPtr->moves.size()%2 == 0;
                        //true means last move was from black
                        moveResultPkt.set_secsleft(whiteCurretMove?matchPtr->clock->blackSecLeft:matchPtr->clock->whiteSecLeft);
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
        {
            std::unique_lock<std::mutex> scopeLock (lock);
            matches.erase(movePkt.matchtoken());
        }
        std::cout  << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " Matchstream ended." << std::endl << std::flush;
        return Status::OK;
    }

    void SendChatMessage(std::string& senderUsername, std::string& reciverUsertoken, std::string& message){
        std::unique_lock<std::mutex> scopeLock (_messageStreamsMutex);
        chesscom::ChatMessage msg;
        msg.set_allocated_message(&message);
        msg.set_allocated_reciver(&reciverUsertoken);
        msg.set_allocated_sender(&senderUsername);
        _messageStreams[reciverUsertoken]->Write(msg);
        msg.release_message();
        msg.release_reciver();
        msg.release_sender();
    }

    void ChatMessageStreamLoop(ServerContext* context, std::string& usertoken, grpc::ServerReaderWriter< chesscom::ChatMessage, chesscom::ChatMessage>* stream){
        chesscom::ChatMessage chatPkt;
        bool keepRunning = true;
        try{
            //std::cout <<  usertoken << " Starting chat read thread " << std::endl << std::flush;
            while (!context->IsCancelled() && keepRunning)
            {
                if(!stream->Read(&chatPkt)){
                    //throw "premeture end of steam"

                    std::cout << usertoken << " ChatStreamCanceled in read thread " << std::endl << std::flush;
                    break;
                }
                if(chatPkt.usertoken() != usertoken)
                {
                    std::cout << usertoken << " Wrong usertoken in chat thread " << std::endl << std::flush;
                }
                else
                {
                    std::cout << usertoken << " Prossesing massage to " << chatPkt.reciver() << std::endl << std::flush;
                    std::unique_lock<std::mutex> scopeLock (lock);
                    for(auto matchKeyVal : matches){
                        if(matchKeyVal.second->whitePlayer == usertoken && userTokens[matchKeyVal.second->blackPlayer] == chatPkt.reciver())
                        {
                            std::cout << usertoken << " Sending message to  " << matchKeyVal.second->blackPlayer << std::endl << std::flush;
                            SendChatMessage(*chatPkt.mutable_sender(), matchKeyVal.second->blackPlayer, *chatPkt.mutable_message());
                        }
                        else if(matchKeyVal.second->blackPlayer == usertoken&& userTokens[matchKeyVal.second->whitePlayer] == chatPkt.reciver())
                        {
                            std::cout << usertoken << " Sending message to  " << matchKeyVal.second->whitePlayer << std::endl << std::flush;
                            SendChatMessage(*chatPkt.mutable_sender(), matchKeyVal.second->whitePlayer, *chatPkt.mutable_message());
                        }
                    }   
                }
                
            }
            std::cout  <<  usertoken<< " ChatMessageStream End" << std::endl << std::flush;
        }
        catch(std::exception ex)
        {
            std::cout << usertoken << " Gracefully exit ChatMessageStream thread exception: " << ex.what() << std::endl << std::flush;
        }
    }

    Status ChatMessageStream(ServerContext* context, grpc::ServerReaderWriter< chesscom::ChatMessage, chesscom::ChatMessage>* stream) override 
    {
        chesscom::ChatMessage chatPkt;
        stream->Read(&chatPkt);
        std::string userToken = chatPkt.usertoken();

        std::cout << "Opening ChatMessageStream for "  <<  userToken << std::endl << std::flush;
        std::thread t1([this, context, &userToken, stream](){
            this->ChatMessageStreamLoop(context, userToken, stream);
        });
        bool loop = true;
        while (loop)
        {
            if(context->IsCancelled()) {
                std::cout <<  userToken<< " ChatMessageStream cancelled " << std::endl << std::flush;
                t1.join();
                return grpc::Status::OK;
            }

            {
                std::unique_lock<std::mutex> scopeLock (_messageStreamsMutex);
                _messageStreams[userToken] = stream;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(MAX_SLEEP_MS));
        }
        t1.join();
        {
            std::unique_lock<std::mutex> scopeLock (_messageStreamsMutex);
            _messageStreams.erase(userToken);
        }
        std::cout  <<  userToken << " ChatMessageStream ended." << std::endl << std::flush;
        return Status::OK;
    }
};



VisionRules ServerVisionRules(){
     VisionRules rules;
     rules.globalRules = Rules();
     rules.globalRules.ViewCaptureField = true;
     rules.globalRules.ViewMoveFields = false;
     rules.globalRules.ViewRange = 2;
     rules.enabled = true;
     rules.overWriteRules[ChessPice::WhitePawn] = Rules(false, true, 1);
     rules.overWriteRules[ChessPice::BlackPawn] = Rules(false, true, 1);

    //chesscom::VisionRules vr;
    //vr.set_enabled(true);
    //vr.set_viewmovefields(false);
    //vr.set_viewrange(2);
    //vr.set_viewcapturefield(true);
    //std::cout << " Vision rules" << std::endl << std::flush;
    //google::protobuf::Map<int, chesscom::VisionRules>* override = vr.mutable_piceoverwriter();
    //chesscom::VisionRules special;
    //special.set_enabled(true);
    //special.set_viewmovefields(false);
    //special.set_viewrange(1);
    //std::cout << " redy to mute" << std::endl << std::flush;
    //(*override)[chesscom::Pices::BlackPawn] = special;
    //(*override)[chesscom::Pices::WhitePawn] = special;
    //(*override) [chesscom::Pices::WhiteKnight] = special;
    //(*override) [chesscom::Pices::BlackKnight] = special;
    return rules;
}

chesscom::TimeRules ServerTimeRules(){
    chesscom::TimeRules tr;
    tr.mutable_playertime()->set_minutes(5);
    tr.mutable_playertime()->set_seconds(0);
    tr.set_secondspermove(6);

    return tr;
}

void Run(std::string port) {
    std::string address("0.0.0.0:" + port);
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
    //std::string vrString = serverVisionRules.SerializeAsString();
    serverTimeRules = ServerTimeRules();
    
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
