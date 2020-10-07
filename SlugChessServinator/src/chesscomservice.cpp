#include "chesscomservice.h"

ChessComService::ChessComService()
{
    tokenCounter = 1;
}
Status ChessComService::Login(ServerContext* context, const chesscom::LoginForm* request, chesscom::LoginResult* response) 
{
    if(request->major_version() == MAJOR_VER && request->minor_version() == MINOR_VER)
    {
        std::cout << "User " << request->major_version() << " " << MAJOR_VER << " " << request->minor_version() << " " << MINOR_VER << " " << request->build_version() << " " << BUILD_VER << std::endl << std::flush;
        if(request->build_version() >= BUILD_VER)
        {
            
        }
        else
        {
            response->set_login_message("You should upgrade to latest version. Server version " VERSION ", Client version " + request->major_version() + "." + request->minor_version() + "." + request->build_version()+". You can find the latest version at 'http://spaceslug.no/slugchess/'");
        }
        std::string userToken = request->username() + "-" + std::to_string(tokenCounter++);
        //PRETEND TO FERCH USERDATA FROM A USER DATABASE
        auto& userData = *response->mutable_user_data();
        userData.set_username(request->username());
        userData.set_usertoken(userToken);
        userData.set_elo(9999);
        UserManager::Get()->LogInUser(userToken, userData);

        std::cout << "User " << response->user_data().username() << " " << response->user_data().usertoken() << " logged in" << std::endl << std::flush;
        response->set_successfull_login(true);
    }
    else if(request->major_version() < MAJOR_VER || (request->major_version() == MAJOR_VER && request->minor_version() < MINOR_VER))
    {
        response->set_successfull_login(false);
        response->set_login_message("Login failed because version missmatch. Server version " VERSION ", Client version " + request->major_version() + "." + request->minor_version() + "." + request->build_version()+". Normaly SlugChess Client should autoupdate. You can find the latest version at 'https://spaceslug.no/slugchess/'");
    }
    else if(request->major_version() > MAJOR_VER || (request->major_version() == MAJOR_VER && request->minor_version() > MINOR_VER))
    {
        response->set_login_message("Client version higher then server. Assuming you are dev or something. Server version " VERSION ", Client version " + request->major_version() + "." + request->minor_version() + "." + request->build_version()+".");
        std::string userToken = request->username() + "-" + std::to_string(tokenCounter++);
        //PRETEND TO FERCH USERDATA FROM A USER DATABASE
        auto& userData = *response->mutable_user_data();
        userData.set_username(request->username());
        userData.set_usertoken(userToken);
        userData.set_elo(9999);
        UserManager::Get()->LogInUser(userToken, userData);
    }
    else
    {
        response->set_successfull_login(false);
        response->set_login_message("Login failed because unhandled version case. Server version " VERSION ", Client version " + request->major_version() + "." + request->minor_version() + "." + request->build_version());
    }
    return Status::OK;
}

Status ChessComService::LookForMatch(ServerContext* context, const chesscom::UserIdentity* request, chesscom::LookForMatchResult* response) 
{
    bool loop = true;
    std::string match_token = "";
    std::string userToken = request->usertoken();
    std::cout << userToken << " looking for match"<< std::endl << std::flush;
    {
        std::unique_lock<std::mutex> scopeLock (lock);
        if(!lookingForMatchQueue.empty()){
            std::string opponent = lookingForMatchQueue.front();
            lookingForMatchQueue.pop();
            match_token = MatchManager::Get()->CreateMatch(userToken, opponent);
            foundMatchReply[opponent] = match_token;
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
                match_token = foundMatchReply[userToken];
                foundMatchReply.erase(userToken);
                loop = false;
            }
        }
    }
    auto matPtr = MatchManager::Get()->GetMatch(match_token);
    {
        std::unique_lock<std::mutex> scopeLock (lock);
        std::cout << userToken << " found match " <<  match_token << std::endl << std::flush;
        std::cout << "  checing match" << std::endl << std::flush;
        std::cout << "  white player " <<  matPtr->getWhitePlayer() << std::endl << std::flush;
    }
    response->set_succes(true);
    response->set_match_token(match_token);
    response->set_is_white_player(matPtr->getWhitePlayer() == userToken);
    response->mutable_opponent_user_data()->CopyFrom(*UserManager::Get()->GetUserData(response->is_white_player()?matPtr->getBlackPlayer():matPtr->getWhitePlayer()).get());
    response->mutable_game_rules()->set_chess_type(chesscom::ChessType::Classic);
    response->mutable_game_rules()->set_side_type(chesscom::SideType::Random);
    
    
    chesscom::VisionRules* vrPtr = response->mutable_game_rules()->mutable_vision_rules();
    auto serverVisionRules = MatchManager::Get()->ServerVisionRules();
    vrPtr->set_enabled(serverVisionRules.enabled);
    vrPtr->set_view_move_fields(serverVisionRules.globalRules.ViewMoveFields);
    vrPtr->set_view_range(serverVisionRules.globalRules.ViewRange);
    vrPtr->set_view_capture_field(serverVisionRules.globalRules.ViewCaptureField);
    //std::cout << " Vision rules" << std::endl << std::flush;
    google::protobuf::Map<int, chesscom::VisionRules>* override = vrPtr->mutable_pice_overwriter();
    
    chesscom::VisionRules special;
    special.set_enabled(true);
    for (auto&& piceRulesPar : serverVisionRules.overWriteRules) {
        special.set_view_range(piceRulesPar.second.ViewRange);
        special.set_view_move_fields(piceRulesPar.second.ViewMoveFields);
        (*override)[piceRulesPar.first] = special;
    }
    chesscom::TimeRules* trPtr = response->mutable_game_rules()->mutable_time_rules();
    trPtr->CopyFrom(MatchManager::Get()->ServerTimeRules());
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
                std::cout  << movePkt.match_token() << " " <<  movePkt.usertoken()<< " MatchStreamCanceled in read thread " << std::endl << std::flush;
                break;
            }
            if(movePkt.asking_for_draw())throw "draw not implemented";
            std::shared_ptr<chesscom::Move> movePtr = std::make_shared<chesscom::Move>(movePkt.move());
            // if(movePkt.doing_move()){
            //     movePtr->set_from(movePkt.move().from());
            //     movePtr->set_to(movePkt.move().to());
            //     movePtr->mutable_timestamp()->Swap(movePkt.mutable_move()->mutable_timestamp());
            //     movePtr->set_sec_spent(movePkt.move().sec_spent());
            // }
            MatchManager::DoMoveInMatch(movePkt.cheat_matchevent(), movePkt.usertoken(), matchPtr, movePtr);
        }
        std::cout  << movePkt.match_token() << " " <<  movePkt.usertoken()<< " ReadThread End" << std::endl << std::flush;
    }
    catch(std::exception& ex)
    {
        std::cout  << movePkt.match_token() << " " <<  movePkt.usertoken()<< " Gracefully exit read thread exception: " << ex.what() << std::endl << std::flush;
    }
}

Status ChessComService::Match(ServerContext* context, grpc::ServerReaderWriter< chesscom::MoveResult, chesscom::MovePacket>* stream) 
{
    chesscom::MovePacket movePkt;
    stream->Read(&movePkt);
    if(movePkt.doing_move()){
        std::cout << "Error: doing move as first " << movePkt.usertoken() << ". Terminating!" << std::endl << std::flush;
        return Status::OK; 
    }
    std::cout << "Opening matchstream for " << movePkt.match_token() << " " <<  movePkt.usertoken() << std::endl << std::flush;
    std::string userToken = movePkt.usertoken();
    std::shared_ptr<::Match> matchPtr = MatchManager::Get()->GetMatch(movePkt.match_token());
    std::cout  << movePkt.match_token() << " " <<  movePkt.usertoken()<< " Starting read thread " << std::endl << std::flush;
    std::thread t1([this, context, matchPtr, stream](){
        this->MatchReadLoop(context, matchPtr, stream);
    });
    //Blocks until match is over
    MatchManager::MatchListenLoop(userToken, matchPtr, context, stream);
    if(context->IsCancelled()) {
        std::cout  << movePkt.match_token() << " " <<  movePkt.usertoken()<< " Matchstream cancelled " << std::endl << std::flush;
        t1.join();
        return grpc::Status::OK;
    }
    t1.join();
    MatchManager::Get()->EraseMatch(movePkt.match_token());
    std::cout  << movePkt.match_token() << " " <<  movePkt.usertoken()<< " Matchstream ended." << std::endl << std::flush;
    return Status::OK;
}

grpc::Status ChessComService::MatchEventListener(grpc::ServerContext *context, const chesscom::MatchObserver* request, grpc::ServerWriter<chesscom::MoveResult> *writer)
{
    std::cout << "Opening MatchEventListener for " << request->match_id() << " " <<  request->usertoken() << std::endl << std::flush;
    std::shared_ptr<::Match> matchPtr = MatchManager::Get()->GetMatch(request->match_id());
    //Blocks until match is over
    MatchManager::MatchListenLoop(request->usertoken(), matchPtr, context, writer);
    if(context->IsCancelled()) {
        std::cout  << request->match_id() << " " <<  request->usertoken()<< " MatchEventListener cancelled " << std::endl << std::flush;
        return grpc::Status::OK;
    }
    MatchManager::Get()->EraseMatch(request->match_id());
    std::cout  << request->match_id() << " " <<  request->usertoken()<< " MatchEventListener ended." << std::endl << std::flush;
    return Status::OK;
}

grpc::Status ChessComService::SendMove(grpc::ServerContext* context, const chesscom::MovePacket* request, chesscom::Void* response) 
{
    if(request->usertoken() == ""){
        std::cout << "Got SendMove call with invalid usertoken" << request->usertoken() << std::endl << std::flush;
        return grpc::Status::CANCELLED;
    }
    if(request->match_token() == ""){
        std::cout << "Got SendMove call with invalid match_token" << request->usertoken() << std::endl << std::flush;
        return grpc::Status::CANCELLED;
    }
    if(request->asking_for_draw())throw "draw not implemented";
    std::shared_ptr<::Match> matchPtr = MatchManager::Get()->GetMatch(request->match_token());
    std::shared_ptr<chesscom::Move> movePtr = std::make_shared<chesscom::Move>(request->move());
    MatchManager::DoMoveInMatch(request->cheat_matchevent(), request->usertoken(), matchPtr, movePtr);
    
    return grpc::Status::OK;
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
            if(chatPkt.sender_usertoken() != usertoken)
            {
                std::cout << usertoken << " Wrong usertoken in chat thread " << std::endl << std::flush;
            }
            else
            {
                std::cout << usertoken << " Prossesing massage to " << chatPkt.reciver_usertoken() << std::endl << std::flush;
                Messenger::SendMessage(*chatPkt.mutable_reciver_usertoken(), *chatPkt.mutable_sender_username(), *chatPkt.mutable_message());                
            }
            
        }
        std::cout  <<  usertoken<< " ChatMessageStream End" << std::endl << std::flush;
    }
    catch(std::exception& ex)
    {
        std::cout << usertoken << " Gracefully exit ChatMessageStream thread exception: " << ex.what() << std::endl << std::flush;
    }
}

Status ChessComService::ChatMessageStream(ServerContext* context, grpc::ServerReaderWriter< chesscom::ChatMessage, chesscom::ChatMessage>* stream) 
{
    chesscom::ChatMessage chatPkt;
    stream->Read(&chatPkt);
    std::string userToken = chatPkt.sender_usertoken();

    std::cout << "Opening ChatMessageStream for "  <<  userToken << std::endl << std::flush;
    std::thread t1([this, context, &userToken, stream](){
        this->ChatMessageStreamLoop(context, userToken, stream);
    });
    UserManager::Get()->AddMessageStream(userToken, &(*stream));
    bool loop = true;
    while (loop)
    {
        if(context->IsCancelled()) {
            std::cout <<  userToken<< " ChatMessageStream cancelled " << std::endl << std::flush;
            t1.join();
            UserManager::Get()->RemoveMessageStream(userToken);
            return grpc::Status::OK;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(MAX_SLEEP_MS));
    }
    t1.join();
    UserManager::Get()->RemoveMessageStream(userToken);
    std::cout  <<  userToken << " ChatMessageStream ended." << std::endl << std::flush;
    return Status::OK;
}

grpc::Status ChessComService::HostGame(grpc::ServerContext *context, const chesscom::HostedGame *request, chesscom::LookForMatchResult *response)
{
    if(!UserManager::Get()->UsertokenLoggedIn(request->host().usertoken())) return grpc::Status::CANCELLED;
    std::cout << " hosting game enter:"<< std::endl << std::flush;
    std::mutex mutex;
    std::condition_variable cv;
    std::unique_lock<std::mutex> lk(mutex);
    bool finished = false;
    int id = GameBrowser::Get()->HostGame(*request, response, &cv, &finished);
    while (!context->IsCancelled())
    {
        if(finished){
            return grpc::Status::OK;
        }

        cv.wait_for(lk, std::chrono::milliseconds(MAX_SLEEP_MS));
    }
    std::cout << "Stopped hosting game id:" << std::to_string(id)<< std::endl << std::flush;
    GameBrowser::Get()->CancelHostGame(id);
    return grpc::Status::OK;
}

grpc::Status ChessComService::AvailableGames(grpc::ServerContext *context, const chesscom::Void *request, chesscom::HostedGamesMap *response) 
{
    GameBrowser::Get()->WriteAvailableGames(*response);
    return grpc::Status::OK;
}

grpc::Status ChessComService::JoinGame(grpc::ServerContext *context, const chesscom::JoinGameRequest *request, chesscom::LookForMatchResult *response) 
{
    GameBrowser::Get()->JoinGame(request->id(), request->joiner(), response);
    return grpc::Status::OK;
}

grpc::Status ChessComService::Alive(grpc::ServerContext* context, const chesscom::Heartbeat* request, chesscom::Heartbeat* response)
{
    if(UserManager::Get()->Heartbeat(request->usertoken()))
    {
        response->set_alive(true);
        response->set_usertoken(request->usertoken());
        return grpc::Status::OK;
    }
    else
    {
        std::cout << request->usertoken() << " failed heartbeat test and was force logged out" <<  std::endl << std::flush;       
        UserManager::Get()->Logout(request->usertoken());
        UserManager::Get()->RemoveMessageStream(request->usertoken());
        response->set_alive(false);
        response->set_usertoken(request->usertoken());
        return grpc::Status::OK;
    }

}

grpc::Status ChessComService::ChatMessageListener(grpc::ServerContext* context, const chesscom::UserData* request, grpc::ServerWriter< ::chesscom::ChatMessage>* writer) 
{
    std::cout << "Opening ChatMessageStream for "  <<  request->usertoken() << std::endl << std::flush;
    UserManager::Get()->AddMessageStream(request->usertoken(), writer);
    std::condition_variable& changedCV =  UserManager::Get()->GetUser(request->usertoken())->changedCV;
    std::mutex mutex;
    std::unique_lock<std::mutex> lk(mutex);
    Messenger::SendMessage(request->usertoken(), "system", "Welcome " + request->username() + " to SlugChess. You are now connected to the chat system.");
    Messenger::SendMessage(request->usertoken(), "system", "Type '/help' from more info on commands");
    while (true)
    {
        if(context->IsCancelled()) {
            std::cout <<  request->usertoken() << " ChatMessageStream cancelled " << std::endl << std::flush;
            UserManager::Get()->RemoveMessageStream(request->usertoken());
            return grpc::Status::OK;
        }
        if(!UserManager::Get()->UsertokenLoggedIn(request->usertoken()))
        {
            std::cout << "ChatMessage stream ended because user logged out" << std::endl << std::flush;
            return grpc::Status::OK;
        }
        changedCV.wait_for(lk, std::chrono::milliseconds(MAX_SLEEP_MS));
//        std::this_thread::sleep_for(std::chrono::milliseconds(MAX_SLEEP_MS));
    }
    return grpc::Status::OK;
}

grpc::Status ChessComService::SendChatMessage(grpc::ServerContext* context, const chesscom::ChatMessage* request, chesscom::Void* response)
{
    //Also sending the message back to sender so it shows up in their log
    Messenger::SendMessage(request->reciver_usertoken(), request->sender_username(), request->message());
    Messenger::SendMessage(request->sender_usertoken(), request->sender_username(), request->message());
    return grpc::Status::OK;
}
