#include "chesscomservice.h"
const int ChessComService::MAX_SLEEP_MS = 1000;
const int ChessComService::SHUTDOWN_WAIT_MS = ChessComService::MAX_SLEEP_MS*2.5;

ChessComService::ChessComService()
{
    gameBrowser._matchManager = &matchManager;
    tokenCounter = 1;
}
Status ChessComService::Login(ServerContext* context, const chesscom::LoginForm* request, chesscom::LoginResult* response) 
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


        std::string userToken = request->username() + "-" + std::to_string(tokenCounter++);
        //PRETEND TO FERCH USERDATA FROM A USER DATABASE
        auto userData = *response->mutable_userdata();
        userData.set_username(request->username());
        userData.set_usertoken(userToken);
        userData.set_elo(9999);
        userManager.LogInUser(userToken, userData);

        std::cout << "User " << request->username() << " " << userToken << " logged in" << std::endl << std::flush;
        response->set_successfulllogin(true);
    }
    else
    {
        response->set_successfulllogin(false);
        response->set_loginmessage("Login failed because version missmatch. Server version " VERSION ", Client version " + request->majorversion() + "." + request->minorversion() + "." + request->buildversion()+". You can find the latest version at 'http://spaceslug.no/slugchess/list.html'");
        
    }
    return Status::OK;
}

Status ChessComService::LookForMatch(ServerContext* context, const chesscom::UserIdentity* request, chesscom::LookForMatchResult* response) 
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
            matchToken = matchManager.CreateMatch(userToken, opponent);
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
    auto matPtr = matchManager.GetMatch(matchToken);
    {
        std::unique_lock<std::mutex> scopeLock (lock);
        std::cout << userToken << " found match " <<  matchToken << std::endl << std::flush;
        std::cout << "  checing match" << std::endl << std::flush;
        std::cout << "  white player " <<  matPtr->whitePlayer << std::endl << std::flush;
    }
    response->set_succes(true);
    response->set_matchtoken(matchToken);
    response->set_iswhiteplayer(matPtr->whitePlayer == userToken);
    response->mutable_opponentuserdata()->CopyFrom(userManager.GetUserData(response->iswhiteplayer()?matPtr->blackPlayer:matPtr->whitePlayer));
    response->mutable_gamerules()->set_chesstype(chesscom::ChessType::Classic);
    response->mutable_gamerules()->set_sidetype(chesscom::SideType::Random);
    
    
    chesscom::VisionRules* vrPtr = response->mutable_gamerules()->mutable_visionrules();
    auto serverVisionRules = matchManager.ServerVisionRules();
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
    chesscom::TimeRules* trPtr = response->mutable_gamerules()->mutable_timerules();
    trPtr->CopyFrom(matchManager.ServerTimeRules());
    //dsa = serverVisionRules;
    return Status::OK;
}

void ChessComService::MatchReadLoop(ServerContext* context, std::shared_ptr<::Match> matchPtr, grpc::ServerReaderWriter< chesscom::MoveResult, chesscom::MovePacket>* stream){
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
                        movePtr->set_capturedpice((chesscom::Pices)matchPtr->game->LastCaptured());
                        //std::stringstream ss;
                        //matchPtr->game->PrintBlackVision(ss);
                        //std::cout << ss.str();
                        //MOves
                        auto avMoves = movePtr->mutable_availablemoves();
                        chesscom::FieldMoves fm;
                        auto fmRF = fm.mutable_list();
                        for (auto &&keyVal : *matchPtr->game->LegalMovesRef())
                        {
                            //std::cout << keyVal.first << " < ";
                            for (auto &&pos : keyVal.second)
                            {
                                //std::cout << SlugChess::BP(pos) << " - ";
                                fmRF->Add(SlugChess::BP(pos));
                            }
                            //std::cout << std::endl;
                            (*avMoves)[SlugChess::BP(keyVal.first)].CopyFrom(fm);
                            fmRF->Clear();
                        }
                        //Shadow MOves
                        auto avShadowMoves = movePtr->mutable_availableshadowmoves();
                        chesscom::FieldMoves fmShadow;
                        auto fmShadowRF = fmShadow.mutable_list();
                        for (auto &&keyVal : *matchPtr->game->LegalShadowMovesRef())
                        {
                            //std::cout << keyVal.first << " < ";
                            for (auto &&pos : keyVal.second)
                            {
                                //std::cout << SlugChess::BP(pos) << " - ";
                                fmShadowRF->Add(SlugChess::BP(pos));
                            }
                            std::cout << std::endl;
                            (*avShadowMoves)[SlugChess::BP(keyVal.first)].CopyFrom(fmShadow);
                            fmShadowRF->Clear();
                        }
                        chesscom::MatchEvent expectedMatchEvent = chesscom::MatchEvent::Non;
                        if(matchPtr->game->Result() != SlugChess::EndResult::StillPlaying){
                            switch (matchPtr->game->Result())
                            {
                            case SlugChess::EndResult::Draw:
                                expectedMatchEvent = chesscom::MatchEvent::Draw;
                                break;
                            case SlugChess::EndResult::WhiteWin:
                                expectedMatchEvent = chesscom::MatchEvent::WhiteWin;
                                break;
                            case SlugChess::EndResult::BlackWin:
                                expectedMatchEvent = chesscom::MatchEvent::BlackWin;
                                break;
                            default:
                                break;
                            }
                        }
                        if(isWhitePlayer){
                            matchPtr->clock->whiteSecLeft -= movePtr->secspent();
                            if(matchPtr->clock->whiteSecLeft <= 0) {
                                expectedMatchEvent = chesscom::MatchEvent::BlackWin;
                            }else{
                                //matchPtr->matchEvents.push_back(chesscom::MatchEvent::Non); 
                                matchPtr->clock->whiteSecLeft += (matchPtr->moves.size()==0?0:matchManager.ServerTimeRules().secondspermove());
                            }
                        }
                        else
                        {
                            matchPtr->clock->blackSecLeft -= movePtr->secspent();
                            if(matchPtr->clock->blackSecLeft <= 0) {
                                expectedMatchEvent = chesscom::MatchEvent::WhiteWin;
                            }else{
                                //matchPtr->matchEvents.push_back(chesscom::MatchEvent::Non); 
                                matchPtr->clock->blackSecLeft += matchManager.ServerTimeRules().secondspermove();
                            }
                        }
                        matchPtr->matchEvents.push_back(expectedMatchEvent);
                        matchPtr->moves.push_back(movePtr);
                        matchPtr->cv.notify_all();
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
                    movePtr->set_capturedpice((chesscom::Pices)matchPtr->game->LastCaptured());
                    //MOves
                    auto avMoves = movePtr->mutable_availablemoves();
                    chesscom::FieldMoves fm;
                    auto fmRF = fm.mutable_list();
                    for (auto &&keyVal : *matchPtr->game->LegalMovesRef())
                    {
                        //std::cout << keyVal.first << " < ";
                        for (auto &&pos : keyVal.second)
                        {
                            //std::cout << SlugChess::BP(pos) << " - ";
                            fmRF->Add(SlugChess::BP(pos));
                        }
                        std::cout << std::endl;
                        (*avMoves)[SlugChess::BP(keyVal.first)].CopyFrom(fm);
                        fmRF->Clear();
                    }
                    //Shadow MOves
                    auto avShadowMoves = movePtr->mutable_availableshadowmoves();
                    chesscom::FieldMoves fmShadow;
                    auto fmShadowRF = fmShadow.mutable_list();
                    for (auto &&keyVal : *matchPtr->game->LegalShadowMovesRef())
                    {
                        //std::cout << keyVal.first << " < ";
                        for (auto &&pos : keyVal.second)
                        {
                            //std::cout << SlugChess::BP(pos) << " - ";
                            fmShadowRF->Add(SlugChess::BP(pos));
                        }
                        std::cout << std::endl;
                        (*avShadowMoves)[SlugChess::BP(keyVal.first)].CopyFrom(fmShadow);
                        fmShadowRF->Clear();
                    }
                    matchPtr->moves.push_back(movePtr);
                    std::cout << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " Got Win" << std::endl << std::flush;
                    matchPtr->matchEvents.push_back(movePkt.cheatmatchevent());
                    matchPtr->cv.notify_all();
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

Status ChessComService::Match(ServerContext* context, grpc::ServerReaderWriter< chesscom::MoveResult, chesscom::MovePacket>* stream) 
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
    std::shared_ptr<::Match> matchPtr = matchManager.GetMatch(movePkt.matchtoken());
    std::cout  << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " Starting read thread " << std::endl << std::flush;
    std::thread t1([this, context, matchPtr, stream](){
        this->MatchReadLoop(context, matchPtr, stream);
    });
    bool playerWhite = matchPtr->whitePlayer == userToken;
    bool loop = true;
    int lastEventNum = 0;
    std::unique_lock<std::mutex> lk(matchPtr->mutex);
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
                    std::stringstream ss;
                    ss << "Moves of the game in SAN:" << std::endl;
                    matchPtr->game->PrintSanMoves(ss);
                    std::string message = ss.str();
                    messenger.SendServerMessage(userToken, message);
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
        //std::this_thread::sleep_for(std::chrono::milliseconds(MAX_SLEEP_MS));
        matchPtr->cv.wait(lk);

    }
    t1.join();
    {
        std::unique_lock<std::mutex> scopeLock (lock);
        matchManager.EraseMatch(movePkt.matchtoken());
    }
    std::cout  << movePkt.matchtoken() << " " <<  movePkt.usertoken()<< " Matchstream ended." << std::endl << std::flush;
    return Status::OK;
}

void ChessComService::ChatMessageStreamLoop(ServerContext* context, std::string& usertoken, grpc::ServerReaderWriter< chesscom::ChatMessage, chesscom::ChatMessage>* stream){
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
                // for(auto matchKeyVal : matches){ TODO temperaraly disabled
                //     if(matchKeyVal.second->whitePlayer == usertoken && userTokens[matchKeyVal.second->blackPlayer] == chatPkt.reciver())
                //     {
                //         std::cout << usertoken << " Sending message to  " << matchKeyVal.second->blackPlayer << std::endl << std::flush;
                //         messenger.SendMessage(*chatPkt.mutable_sender(), matchKeyVal.second->blackPlayer, *chatPkt.mutable_message());
                //     }
                //     else if(matchKeyVal.second->blackPlayer == usertoken&& userTokens[matchKeyVal.second->whitePlayer] == chatPkt.reciver())
                //     {
                //         std::cout << usertoken << " Sending message to  " << matchKeyVal.second->whitePlayer << std::endl << std::flush;
                //         messenger.SendMessage(*chatPkt.mutable_sender(), matchKeyVal.second->whitePlayer, *chatPkt.mutable_message());
                //     }
                // }   
            }
            
        }
        std::cout  <<  usertoken<< " ChatMessageStream End" << std::endl << std::flush;
    }
    catch(std::exception ex)
    {
        std::cout << usertoken << " Gracefully exit ChatMessageStream thread exception: " << ex.what() << std::endl << std::flush;
    }
}

Status ChessComService::ChatMessageStream(ServerContext* context, grpc::ServerReaderWriter< chesscom::ChatMessage, chesscom::ChatMessage>* stream) 
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
        messenger.AddMessageStream(userToken, stream);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(MAX_SLEEP_MS));
    }
    t1.join();
    messenger.RemoveMessageStream(userToken);
    std::cout  <<  userToken << " ChatMessageStream ended." << std::endl << std::flush;
    return Status::OK;
}

grpc::Status ChessComService::HostGame(grpc::ServerContext *context, const chesscom::HostedGame *request, chesscom::LookForMatchResult *response)
{
    //TODO: check if the useroken set by host acctually exitst. Make user manager
    if(!userManager.UsertokenLoggedIn(request->host().usertoken())) return grpc::Status::CANCELLED;
    std::cout << " hosting game enter:"<< std::endl << std::flush;
    std::mutex mutex;
    std::condition_variable cv;
    std::unique_lock<std::mutex> lk(mutex);
    bool finished = false;
    int id = gameBrowser.HostGame(*request, response, &cv, &finished);
    while (!context->IsCancelled())
    {
        if(finished){
            return grpc::Status::OK;
        }

        cv.wait_for(lk, std::chrono::milliseconds(MAX_SLEEP_MS));
    }
    std::cout << "Stopped hosting game id:" << std::to_string(id)<< std::endl << std::flush;
    gameBrowser.CancelHostGame(id);
    return grpc::Status::OK;
}

grpc::Status ChessComService::AvailableGames(grpc::ServerContext *context, const chesscom::Void *request, chesscom::HostedGamesMap *response) 
{
    gameBrowser.WriteAvailableGames(*response);
    return grpc::Status::OK;
}

grpc::Status ChessComService::JoinGame(grpc::ServerContext *context, const chesscom::JoinGameRequest *request, chesscom::LookForMatchResult *response) 
{
    gameBrowser.JoinGame(request->id(), request->joiner(), response);
    return grpc::Status::OK;
}

grpc::Status ChessComService::Alive(grpc::ServerContext* context, const chesscom::Heartbeat* request, chesscom::Heartbeat* response)
{
    if(userManager.TestHeart(request->usertoken()))
    {
        response->set_alive(true);
        response->set_usertoken(request->usertoken());
        return grpc::Status::OK;
    }
    else
    {
        userManager.Logout(request->usertoken());
        //TODO MUST DO NESSESARY ACTIONS IE STOP MATCES THAT INCLUDES THIS USER
        response->set_alive(false);
        response->set_usertoken(request->usertoken());
        return grpc::Status::OK;
    }

}