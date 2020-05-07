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
    if(request->major_version() == MAJOR_VER && request->minor_version() == MINOR_VER)
    {
        std::cout << "User " << request->major_version() << " " << MAJOR_VER << " " << request->minor_version() << " " << MINOR_VER << " " << request->build_version() << " " << BUILD_VER << std::endl << std::flush;
        if(request->build_version() >= BUILD_VER)
        {
            
        }
        else
        {
            response->set_login_message("You should upgrade to latest version. Server version " VERSION ", Client version " + request->major_version() + "." + request->minor_version() + "." + request->build_version()+". You can find the latest version at 'http://spaceslug.no/slugchess/list.html'");
        }
        std::string userToken = request->username() + "-" + std::to_string(tokenCounter++);
        //PRETEND TO FERCH USERDATA FROM A USER DATABASE
        auto& userData = *response->mutable_user_data();
        userData.set_username(request->username());
        userData.set_usertoken(userToken);
        userData.set_elo(9999);
        userManager.LogInUser(userToken, userData);

        std::cout << "User " << response->user_data().username() << " " << response->user_data().usertoken() << " logged in" << std::endl << std::flush;
        response->set_successfull_login(true);
    }
    else
    {
        response->set_successfull_login(false);
        response->set_login_message("Login failed because version missmatch. Server version " VERSION ", Client version " + request->major_version() + "." + request->minor_version() + "." + request->build_version()+". You can find the latest version at 'http://spaceslug.no/slugchess/list.html'");
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
            match_token = matchManager.CreateMatch(userToken, opponent);
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
    auto matPtr = matchManager.GetMatch(match_token);
    {
        std::unique_lock<std::mutex> scopeLock (lock);
        std::cout << userToken << " found match " <<  match_token << std::endl << std::flush;
        std::cout << "  checing match" << std::endl << std::flush;
        std::cout << "  white player " <<  matPtr->whitePlayer << std::endl << std::flush;
    }
    response->set_succes(true);
    response->set_match_token(match_token);
    response->set_is_white_player(matPtr->whitePlayer == userToken);
    response->mutable_opponent_user_data()->CopyFrom(userManager.Getuser_data(response->is_white_player()?matPtr->blackPlayer:matPtr->whitePlayer));
    response->mutable_game_rules()->set_chess_type(chesscom::ChessType::Classic);
    response->mutable_game_rules()->set_side_type(chesscom::SideType::Random);
    
    
    chesscom::VisionRules* vrPtr = response->mutable_game_rules()->mutable_vision_rules();
    auto serverVisionRules = matchManager.ServerVisionRules();
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
    trPtr->CopyFrom(matchManager.ServerTimeRules());
    //dsa = serverVisionRules;
    return Status::OK;
}

void ChessComService::MatchReadLoop(ServerContext* context, std::shared_ptr<::Match> matchPtr, grpc::ServerReaderWriter< chesscom::MoveResult, chesscom::MovePacket>* stream){
    chesscom::MovePacket movePkt;
    bool keepRunning = true;
    bool isWhitePlayer = matchPtr->whitePlayer == movePkt.usertoken();
    try{
        while (!context->IsCancelled() && keepRunning)
        {
            if(!stream->Read(&movePkt)){
                //throw "premeture end of steam"
                std::cout  << movePkt.match_token() << " " <<  movePkt.usertoken()<< " MatchStreamCanceled in read thread " << std::endl << std::flush;
                break;
            }
            if(movePkt.asking_for_draw())throw "draw not implemented";
            std::shared_ptr<chesscom::Move> movePtr = std::make_shared<chesscom::Move>();
            if(movePkt.doing_move()){
                movePtr->set_from(movePkt.move().from());
                movePtr->set_to(movePkt.move().to());
                movePtr->mutable_timestamp()->Swap(movePkt.mutable_move()->mutable_timestamp());
                movePtr->set_sec_spent(movePkt.move().sec_spent());
            }
            
            switch (movePkt.cheat_matchevent())
            {
            case chesscom::MatchEvent::Non:
                std::cout  << movePkt.match_token() << " " <<  movePkt.usertoken()<< " Got move " << movePkt.move().from() << " " << movePkt.move().to() << " secspent " << std::to_string(movePtr->sec_spent()) << std::endl << std::flush;
                {
                    std::unique_lock<std::mutex> scopeLock (lock);
                    bool isWhitePlayer = matchPtr->whitePlayer == movePkt.usertoken();
                    bool isPlayersCurrentTurn = matchPtr->moves.size()%2 == (isWhitePlayer?0:1);
                    if(isPlayersCurrentTurn){
                        matchPtr->game->DoMove(movePtr->from(), movePtr->to());
                        
                        //SlugChessConverter::SetMove(matchPtr->game, movePtr, isWhitePlayer);

                        chesscom::MatchEvent expectedmatch_event = chesscom::MatchEvent::Non;
                        if(matchPtr->game->Result() != SlugChess::EndResult::StillPlaying){
                            switch (matchPtr->game->Result())
                            {
                            case SlugChess::EndResult::Draw:
                                expectedmatch_event = chesscom::MatchEvent::Draw;
                                break;
                            case SlugChess::EndResult::WhiteWin:
                                expectedmatch_event = chesscom::MatchEvent::WhiteWin;
                                break;
                            case SlugChess::EndResult::BlackWin:
                                expectedmatch_event = chesscom::MatchEvent::BlackWin;
                                break;
                            default:
                                break;
                            }
                        }
                        if(isWhitePlayer){
                            matchPtr->clock->whiteSecLeft -= movePtr->sec_spent();
                            if(matchPtr->clock->whiteSecLeft <= 0) {
                                expectedmatch_event = chesscom::MatchEvent::BlackWin;
                            }else{
                                //matchPtr->match_events.push_back(chesscom::MatchEvent::Non); 
                                matchPtr->clock->whiteSecLeft += (matchPtr->moves.size()==0?0:matchManager.ServerTimeRules().seconds_per_move());
                            }
                        }
                        else
                        {
                            matchPtr->clock->blackSecLeft -= movePtr->sec_spent();
                            if(matchPtr->clock->blackSecLeft <= 0) {
                                expectedmatch_event = chesscom::MatchEvent::WhiteWin;
                            }else{
                                //matchPtr->match_events.push_back(chesscom::MatchEvent::Non); 
                                matchPtr->clock->blackSecLeft += matchManager.ServerTimeRules().seconds_per_move();
                            }
                        }
                        matchPtr->matchEvents.push_back(expectedmatch_event);
                        matchPtr->moves.push_back(movePtr);
                        std::cout << "adding ChessMove2" << std::endl << std::flush;
                        matchPtr->cv.notify_all();
                        //std::cout  << movePkt.match_token() << " " <<  movePkt.usertoken()<< " whitesecs left " << std::to_string(matchPtr->clock->whiteSecLeft) << " blacksecs left " << std::to_string(matchPtr->clock->blackSecLeft) << std::endl << std::flush;
                        //std::string output;google::protobuf::util::MessageToJsonString(*movePtr, &output);
                        //matchPtr->
                    }else{
                        std::cout  << movePkt.match_token() << " " <<  movePkt.usertoken()<< " ERROR: not this players turn to move" << std::endl << std::flush;
                    }
                
                }
                break;
            case chesscom::MatchEvent::UnexpectedClosing:
                {
                    std::unique_lock<std::mutex> scopeLock (lock);
                    //if(movePkt.doingmove()){
                    //    std::cout  << movePkt.match_token() << " " <<  movePkt.usertoken()<< " Got move " << movePkt.move().from() << " " << movePkt.move().to() << std::endl << std::flush;
                    //    matchPtr->moves.push_back(movePtr);
                    //}else{
                        //matchPtr->moves.push_back(std::make_shared<chesscom::Move>());
                    //}
                    std::cout << movePkt.match_token() << " " <<  movePkt.usertoken()<< " Got UnexpectedClosing" << std::endl << std::flush;
                    matchPtr->matchEvents.push_back(chesscom::MatchEvent::UnexpectedClosing);
                    //matchPtr->newUpdate = true;
                    keepRunning = false;
                    break;
                }
                break;
                case chesscom::MatchEvent::ExpectedClosing:
                std::cout << movePkt.match_token() << " " <<  movePkt.usertoken()<< " Got UnexpectedClosing" << std::endl << std::flush;
                keepRunning = false;
                break;
            case chesscom::MatchEvent::WhiteWin:
            case chesscom::MatchEvent::BlackWin:
                {
                    std::unique_lock<std::mutex> scopeLock (lock);
                    std::cout << movePkt.match_token() << " " <<  movePkt.usertoken()<< " Got move " << movePkt.move().from() << " " << movePkt.move().to() << std::endl << std::flush;
                    matchPtr->game->DoMove(movePtr->from(), movePtr->to());
                    
                    //SlugChessConverter::SetMove(matchPtr->game, movePtr, isWhitePlayer);
                    matchPtr->moves.push_back(movePtr);
                    std::cout << "adding ChessMove" << std::endl << std::flush;
                   
                    std::cout << movePkt.match_token() << " " <<  movePkt.usertoken()<< " Got Win" << std::endl << std::flush;
                    matchPtr->matchEvents.push_back(movePkt.cheat_matchevent());
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
        std::cout  << movePkt.match_token() << " " <<  movePkt.usertoken()<< " ReadThread End" << std::endl << std::flush;
    }
    catch(std::exception ex)
    {
        std::cout  << movePkt.match_token() << " " <<  movePkt.usertoken()<< " Gracefully exit read thread exception: " << ex.what() << std::endl << std::flush;
    }
}

Status ChessComService::Match(ServerContext* context, grpc::ServerReaderWriter< chesscom::MoveResult, chesscom::MovePacket>* stream) 
{
    chesscom::MovePacket movePkt;
    chesscom::MoveResult moveResultPkt;
    chesscom::GameState state;
    stream->Read(&movePkt);
    if(movePkt.doing_move()){
        std::cout << "Error: doing move as first " << movePkt.usertoken() << ". Terminating!" << std::endl << std::flush;
        return Status::OK; 
    }
    std::cout << "Opening matchstream for " << movePkt.match_token() << " " <<  movePkt.usertoken() << std::endl << std::flush;
    std::string userToken = movePkt.usertoken();
    std::shared_ptr<::Match> matchPtr = matchManager.GetMatch(movePkt.match_token());
    std::cout  << movePkt.match_token() << " " <<  movePkt.usertoken()<< " Starting read thread " << std::endl << std::flush;
    std::thread t1([this, context, matchPtr, stream](){
        this->MatchReadLoop(context, matchPtr, stream);
    });
    bool playerIsWhite = matchPtr->whitePlayer == userToken;
    bool loop = true;
    uint lastEventNum = 0;
    std::unique_lock<std::mutex> lk(matchPtr->mutex);
    //Sending init
    std::cout  << " Sending init gamestate to  " << userToken << std::endl << std::flush;
    moveResultPkt.set_move_happned(false);
    moveResultPkt.set_opponent_asking_for_draw(false);
    SlugChessConverter::SetGameState(matchPtr->game, &state, playerIsWhite);
    moveResultPkt.set_allocated_game_state(&state);
    moveResultPkt.set_match_event(chesscom::MatchEvent::Non);
    moveResultPkt.set_secs_left(matchPtr->clock->whiteSecLeft);
    stream->Write(moveResultPkt);
    //moveResultPkt.release_move();
    moveResultPkt.release_game_state();
    while (loop)
    {
        if(context->IsCancelled()) {
            std::cout  << movePkt.match_token() << " " <<  movePkt.usertoken()<< " Matchstream cancelled " << std::endl << std::flush;
            t1.join();
            return grpc::Status::OK;
        }
        //bool isPlayersCurrentTurn;
        //{
        //    std::unique_lock<std::mutex> scopeLock (lock);
        //    isPlayersCurrentTurn = matchPtr->moves.size()%2 == (playerWhite?0:1); 
        //}
        //std::cout  << movePkt.match_token() << " " <<  movePkt.usertoken()<< " IsCurrentTurn "<< std::to_string(isPlayersCurrentTurn) << std::endl << std::flush;
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
                    std::cout  << movePkt.match_token() << " " <<  movePkt.usertoken()<< " Opponent UnexpectedClosing" << std::endl << std::flush;
                    moveResultPkt.set_move_happned(false);
                    moveResultPkt.set_opponent_asking_for_draw(false);
                    //moveResultPkt.set_allocated_move(matchPtr->moves.back().get());
                    moveResultPkt.set_match_event(chesscom::MatchEvent::UnexpectedClosing);
                    stream->Write(moveResultPkt);
                    loop = false;
                    //continue;
                }
                else if(matchPtr->matchEvents[lastEventNum] == chesscom::MatchEvent::WhiteWin || matchPtr->matchEvents[lastEventNum] == chesscom::MatchEvent::BlackWin)
                {
                    std::cout  << " Going to send other move " << std::endl << std::flush;
                    std::cout  << movePkt.match_token() << " " <<  movePkt.usertoken()<< " Someone won!! " << matchPtr->matchEvents[lastEventNum] << std::endl << std::flush;
                    moveResultPkt.set_move_happned(true);
                    moveResultPkt.set_opponent_asking_for_draw(false);
                    SlugChessConverter::SetGameState(matchPtr->game, &state, playerIsWhite);
                    moveResultPkt.set_allocated_game_state(&state);
                    //moveResultPkt.set_allocated_move(matchPtr->moves.back().get());
                    moveResultPkt.set_match_event(matchPtr->matchEvents[lastEventNum]);
                    bool whiteCurretMove = matchPtr->moves.size()%2 == 0;
                    //true means last move was from black
                    moveResultPkt.set_secs_left(whiteCurretMove?matchPtr->clock->blackSecLeft:matchPtr->clock->whiteSecLeft);
                    //grpc::WriteOptions options;
                    //std::cout  << movePkt.match_token() << " " <<  movePkt.usertoken()<< " IsBufferedHint " << options.get_buffer_hint() << std::endl << std::flush;
                    std::stringstream ss;
                    ss << "Moves of the game in SAN:" << std::endl;
                    matchPtr->game->PrintSanMoves(ss);
                    std::string message = ss.str();
                    messenger.SendServerMessage(userToken, message);
                    stream->Write(moveResultPkt);
                    moveResultPkt.release_game_state();
                    loop = false;
                    continue;
                }
                else
                {
                    std::cout  << " Going to send move " << std::endl << std::flush;
                    moveResultPkt.set_move_happned(true);
                    moveResultPkt.set_opponent_asking_for_draw(false);
                    //moveResultPkt.set_allocated_move(matchPtr->moves.back().get());
                    SlugChessConverter::SetGameState(matchPtr->game, &state, playerIsWhite);
                    moveResultPkt.set_allocated_game_state(&state);
                    moveResultPkt.set_match_event(chesscom::MatchEvent::Non);
                    bool whiteCurretMove = matchPtr->moves.size()%2 == 0;
                    //true means last move was from black
                    moveResultPkt.set_secs_left(whiteCurretMove?matchPtr->clock->blackSecLeft:matchPtr->clock->whiteSecLeft);
                    stream->Write(moveResultPkt);
                    //moveResultPkt.release_move();
                    moveResultPkt.release_game_state();
                    std::cout  << movePkt.match_token() << " " <<  movePkt.usertoken()<< " SentMoveResult " << std::endl << std::flush;
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
        matchManager.EraseMatch(movePkt.match_token());
    }
    std::cout  << movePkt.match_token() << " " <<  movePkt.usertoken()<< " Matchstream ended." << std::endl << std::flush;
    return Status::OK;
}

grpc::Status ChessComService::MatchEventListener(grpc::ServerContext *context, const chesscom::MatchObserver* request, grpc::ServerWriter<chesscom::MoveResult> *writer)
{
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
            if(chatPkt.sender_usertoken() != usertoken)
            {
                std::cout << usertoken << " Wrong usertoken in chat thread " << std::endl << std::flush;
            }
            else
            {
                std::cout << usertoken << " Prossesing massage to " << chatPkt.reciver_usertoken() << std::endl << std::flush;
                messenger.SendMessage(*chatPkt.mutable_reciver_usertoken(), *chatPkt.mutable_sender_username(), *chatPkt.mutable_message());                
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
    std::string userToken = chatPkt.sender_usertoken();

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
        messenger.RemoveMessageStream(request->usertoken());
        response->set_alive(false);
        response->set_usertoken(request->usertoken());
        return grpc::Status::OK;
    }

}