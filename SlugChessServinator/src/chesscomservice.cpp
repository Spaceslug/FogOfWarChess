#include "chesscomservice.h"

ChessComService::ChessComService() 
{
    tokenCounter = 1;
}
ChessComService::~ChessComService()
{
    
}
grpc::Status ChessComService::RegisterUser(ServerContext* context, const chesscom::RegiserUserForm* request, chesscom::RegiserUserFormResult* response) 
{
    if(request->username().find('/') != std::string::npos){
        response->set_success(false);
        response->set_fail_message("Username '" + request->username() + "' contains '/'. Choose another one.");
        return Status::OK;
    }
    if(request->username().size() > 80){
        response->set_success(false);
        response->set_fail_message("Username '" + request->username() + "' is to long. Choose another one.");
        return Status::OK;
    }
    if(request->password().size() > 80){
        response->set_success(false);
        response->set_fail_message("Password is to long. Choose another one.");
        return Status::OK;
    }
    if(Filesystem::UserFileExists(UserStore::UsernameToDataFilename(request->username()))){
        response->set_success(false);
        response->set_fail_message("Username '" + request->username() + "' already exists. Choose another one.");
        return Status::OK;
    }else{
        auto encry_key = UserStore::EncryptionKeyFromPassword(request->password());
        chesscom::UserStaticData ud;
        ud.set_username(request->username());
        ud.set_elo(1500);
         try {
            std::string encrypted = UserStore::encrypt_string(SECRET, encry_key);
            ud.set_secret(encrypted);
       
            Filesystem::WriteUserFile(ud, UserStore::UsernameToDataFilename(ud.username()));
            response->set_success(true);
            return Status::OK;
        }
        catch (std::invalid_argument& ex){
            Messenger::Log("Failed to create user:" + std::string(ex.what()));
            //TODO: delete user data incase it got half writen
            response->set_success(false);
            response->set_fail_message("Something whent wring when create user. Please try again with different username or password");
            return Status::OK;
        }
    }
    return Status::CANCELLED;

}

Status ChessComService::Login(ServerContext* context, const chesscom::LoginForm* request, chesscom::LoginResult* response) 
{
    //START BY VALIDATING USER
    const auto encryption_key = UserStore::EncryptionKeyFromPassword(request->password());
    const auto filename = UserStore::UsernameToDataFilename(request->username());
    if(!Filesystem::UserFileExists(filename)){
        response->set_successfull_login(false);
        response->set_login_message("Username or password is wrong");
        return Status::OK;
    }
    auto& userData = *response->mutable_user_data();
    try{
        auto userdata_static = Filesystem::ReadUserFile(UserStore::UsernameToDataFilename(request->username()));
        if(SECRET == UserStore::decrypt_string(userdata_static.secret(), encryption_key)){
            userData.set_username(userdata_static.username());
            userData.set_elo(userdata_static.elo());
        }else{
            response->set_successfull_login(false);
            response->set_login_message("Username or password is wrong");
            return Status::OK;
        }

    } catch(std::invalid_argument& ex){
        response->set_successfull_login(false);
        response->set_login_message("Username and password correct, but loading of userdata failed. If trying again dossend wourk you are fucked");
        return Status::OK;
    }
    
    
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
        userData.set_usertoken(userToken);
        UserManager::Get()->LogInUser(userToken, userData, encryption_key);

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
        userData.set_usertoken(userToken);
        UserManager::Get()->LogInUser(userToken, userData, encryption_key);
    }
    else
    {
        response->set_successfull_login(false);
        response->set_login_message("Login failed because unhandled version case. Server version " VERSION ", Client version " + request->major_version() + "." + request->minor_version() + "." + request->build_version());
    }
    return Status::OK;
}

grpc::Status ChessComService::GetNamedVariants(ServerContext* context, const chesscom::Void* request, chesscom::NamedVariants* response) 
{
    //response->set_variants();
    auto variants = SlugChess::GetVariants();
    *response->mutable_variants() = {variants.begin(), variants.end()};
    return Status::OK;
}
    
grpc::Status ChessComService::Logout(grpc::ServerContext* context, const chesscom::UserIdentification* request, chesscom::LogoutResult* response) 
{
    //TODO verify user identification
    response->set_successfull_logout(UserManager::Get()->Logout(request->usertoken()));
    if(!response->successfull_logout()){
        response->set_logout_message("Could not logout as user not logged in");
    }
    return Status::OK;
}

Status ChessComService::LookForMatch(ServerContext* context, const chesscom::UserIdentification* request, chesscom::LookForMatchResult* response) 
{
    //////////     TODO: depracated as fuck
    // bool loop = true;
    // std::string match_token = "";
    // std::string userToken = request->usertoken();
    // std::cout << userToken << " looking for match"<< std::endl << std::flush;
    // {
    //     std::unique_lock<std::mutex> scopeLock (lock);
    //     if(!lookingForMatchQueue.empty()){
    //         std::string opponent = lookingForMatchQueue.front();
    //         lookingForMatchQueue.pop();
    //         match_token = MatchManager::Get()->CreateMatch(userToken, opponent);
    //         foundMatchReply[opponent] = match_token;
    //         loop = false;
    //     }
    //     else
    //     {
    //         std::cout << userToken << " entering queue"<< std::endl << std::flush;
    //         lookingForMatchQueue.emplace(userToken);
    //     }
    // }
    // while(loop){
    //     std::this_thread::sleep_for(std::chrono::milliseconds(MAX_SLEEP_MS));
    //     if(context->IsCancelled()) return grpc::Status::CANCELLED;
    //     {
    //         std::unique_lock<std::mutex> scopeLock (lock);
    //         if(foundMatchReply.count(userToken) > 0){
    //             std::cout << userToken << " found match reply MT: " << foundMatchReply[userToken]<< std::endl << std::flush;
    //             match_token = foundMatchReply[userToken];
    //             foundMatchReply.erase(userToken);
    //             loop = false;
    //         }
    //     }
    // }
    // auto matPtr = MatchManager::Get()->GetMatch(match_token);
    // {
    //     std::unique_lock<std::mutex> scopeLock (lock);
    //     std::cout << userToken << " found match " <<  match_token << std::endl << std::flush;
    //     std::cout << "  checing match" << std::endl << std::flush;
    //     std::cout << "  white player " <<  matPtr->getWhitePlayer() << std::endl << std::flush;
    // }
    // response->set_succes(true);
    // response->set_match_token(match_token);
    // response->set_is_white_player(matPtr->getWhitePlayer() == userToken);
    // response->mutable_opponent_user_data()->CopyFrom(*UserManager::Get()->GetUserData(response->is_white_player()?matPtr->getBlackPlayer():matPtr->getWhitePlayer()).get());
    // response->mutable_game_rules()->set_chess_type(chesscom::ChessType::Classic);
    // response->mutable_game_rules()->set_side_type(chesscom::SideType::Random);
    
    
    // chesscom::VisionRules* vrPtr = response->mutable_game_rules()->mutable_vision_rules();
    // auto serverVisionRules = MatchManager::Get()->ServerVisionRules();
    // vrPtr->set_enabled(serverVisionRules.enabled);
    // vrPtr->set_view_move_fields(serverVisionRules.globalRules.ViewMoveFields);
    // vrPtr->set_view_range(serverVisionRules.globalRules.ViewRange);
    // vrPtr->set_view_capture_field(serverVisionRules.globalRules.ViewCaptureField);
    // google::protobuf::Map<int, chesscom::VisionRules>* override = vrPtr->mutable_pice_overwriter();
    
    // chesscom::VisionRules special;
    // special.set_enabled(true);
    // for (auto&& piceRulesPar : serverVisionRules.overWriteRules) {
    //     special.set_view_range(piceRulesPar.second.ViewRange);
    //     special.set_view_move_fields(piceRulesPar.second.ViewMoveFields);
    //     (*override)[piceRulesPar.first] = special;
    // }
    // chesscom::TimeRules* trPtr = response->mutable_game_rules()->mutable_time_rules();
    // trPtr->CopyFrom(MatchManager::Get()->ServerTimeRules());
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
    if(matchPtr.get() == nullptr){
        Messenger::Log("Got SendMove call with match_token but no match of that token was found" + request->usertoken());
        return grpc::Status::CANCELLED;
    }
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
    if(!UserManager::Get()->UsertokenLoggedIn(request->usertoken()))
    {
        Messenger::Log("'"+request->usertoken() + "' is not a logged in user. Heartbeat failed");       
        response->set_alive(false);
        response->set_usertoken(request->usertoken());
        return grpc::Status::OK;
    }
    if(UserManager::Get()->Heartbeat(request->usertoken()))
    {
        response->set_alive(true);
        response->set_usertoken(request->usertoken());
        return grpc::Status::OK;
    }
    else
    {
        Messenger::Log("'"+request->usertoken() + "' failed heartbeat test and was force logged out");       
        UserManager::Get()->Logout(request->usertoken());
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
            Messenger::Log("ChatMessage stream ended because " + request->usertoken() + " logged out");
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

grpc::Status ChessComService::ProcessReplay(grpc::ServerContext* context,  const chesscom::ReplayRequest* request, chesscom::Replay* response)
{
    // std::string test = "[Event \"Custom SlugChess game\"]\n" 
    //                     "[Site \"REDACTED, REDACTED NOR\"]\n" 
    //                     "[Date \"2020.10.8\"]\n" 
    //                     "[Round \"1\"]\n" 
    //                     "[White \"debugman\"]\n" 
    //                     "[Black \"debug_sexyman\"]\n" 
    //                     "[Result \"0-1\"]\n" 
    //                     "[Time \"19:3:23\"]\n" 
    //                     "[Mode \"ICS\"]\n" 
    //                     "[FEN \"knrqnbbr/pppppppp/8/8/8/8/PPPPPPPP/RQRNNBKB w - - 0 1\"]\n" 
    //                     "[SetUp \"1\"]\n" 
    //                     "[Variant \"SlugChess.Torch\"]\n" 
    //                     "\n" 
    //                     "1. e3 Nb8c6 2. c4 e5 3. g3 f6 4. Qb1e4 h6 5. Ne1d3 Bg8h7 6. Qe4f3 b6 7. Qf3e4 Qd8e7 8. Nd3c5 Bh7xe4 9. Bh1xe4 Qe7xc5 10. Be4xc6 d7xc6 11. e4 Qc5xf2 12. Kg1xf2 c5 13. b4 c5xb4 14. a3 b4xa3 15. Ra1xa3 c5 16. d3 Bf8d6 17. d4 e5xd4 18. Ra3d3 f5 19. e4xf5 g6 20. f5xg6 Rh8g8 21. Rd3xd4 Rg8xg6 22. Rd4xd6 Ne8xd6 23. Nd1e3 Rg6xg3 24. Kf2xg3 h5 25. Ne3f5 Nd6xf5 26. Bf1d3 Rc8f8 27. Bd3xf5 Rf8e8 28. Rc1b1 b5 29. c4xb5 a6 30. Bf5c8 Re8xc8 31. b5xa6 Rc8g8 32. Kg3h4 Rg8g4 33. Rb1b8 Rg4xh4  0-1";
    auto pgnMap = Match::ReadSlugChessPgnString(request->pgn());
    auto sanMoves = San::SanMovesFromSan(pgnMap["San"]);

    auto* visionRule = SlugChess::GetVisionRule(pgnMap["Variant"].substr(pgnMap["Variant"].find('.')+1));
    if(visionRule == nullptr){
        Messenger::Log("Invalid visionrules '"+ pgnMap["Variant"].substr(pgnMap["Variant"].find('.')+1)+ "' requested");
        Messenger::SendServerMessage(request->user_ident().usertoken(), 
            "PGN Invalid. Variant '"+pgnMap["Variant"]+"' unknown");
        response->set_valid(false);
        return grpc::Status::OK;
    }
    auto game = std::make_shared<SlugChess>(pgnMap["FEN"], *visionRule);
    auto moves = San::SanMovesFromSan(pgnMap["San"]);
    response->set_white(pgnMap["White"]);
    response->set_black(pgnMap["Black"]);
    SlugChessConverter::SetGameState(game, response->add_game_states(), PlayerTypes::Observer);
    for(auto& move : moves){
        if(!game->DoSanMove(move)){
            Messenger::Log("San move failed '"+move+"'. Replay invalid");
            response->set_valid(false);
            return grpc::Status::OK;
        }else{
            Messenger::Log("San move '"+move+"'");
        }
        SlugChessConverter::SetGameState(game, response->add_game_states(), PlayerTypes::Observer);
    }
    switch (game->Result())
    {
    case SlugChess::EndResult::StillPlaying:
            response->set_match_event(chesscom::MatchEvent::Non);
        break;
    case SlugChess::EndResult::Draw:
            response->set_match_event(chesscom::MatchEvent::Draw);
        break;
    case SlugChess::EndResult::WhiteWin:
            response->set_match_event(chesscom::MatchEvent::WhiteWin);
        break;
    case SlugChess::EndResult::BlackWin:
            response->set_match_event(chesscom::MatchEvent::BlackWin);
        break;
    default:
        throw std::invalid_argument("WTF man. What the hell have you done??");
    }
    Messenger::Log("Replay parsed successfully");
    response->set_valid(true);
    return grpc::Status::OK;
}

grpc::Status ChessComService::ServerVisionRulesets(grpc::ServerContext* context, const chesscom::Void *request, chesscom::VisionRuleset* response) 
{
    auto visionRules = SlugChess::GetVisionRules();
    for(auto& [name, scRule] : visionRules){
        response->mutable_vision_rulesets()->insert({name, MatchManager::Get()->FromSlugChessVisionRules(*scRule)}); 
    }
    return grpc::Status::OK;
}

grpc::Status ChessComService::GetPublicUserdata(grpc::ServerContext* context, const chesscom::UserDataRequest* request, chesscom::UserData* response)
{
    //TODO verify secret
    std::string usertoken = request->user_ident().usertoken();
    if(UserManager::Get()->UsertokenLoggedIn(usertoken)){
        auto userdata = UserManager::Get()->GetPublicUserDataFromUsername(request->username());
        if(userdata.username() == request->username()){
            *response = userdata;
            
            Messenger::Log("Sending userdata about: " + userdata.username());
        }
    }
    return grpc::Status::OK;
}
