#pragma once
#include <string>
#include <chrono>
#include <memory>
#include "../chesscom/chesscom.grpc.pb.h"

#define TIMEOUT_FOR_DRAW 5
#define EXTRATIME_FOR_DRAW 1
#define MOVE_TIME_DELAY_ALLOWED_MILLISEC 1000

enum PlayerTypes{
    White = 0,
    Black = 1,
    Observer = 2
};
struct MoveResultStream {
    bool alive;
    grpc::internal::WriterInterface<chesscom::MoveResult>* streamPtr;
};
struct ChessClock {
    int blackSecLeft;
    int whiteSecLeft;
    int secsPerMove;
    bool is_ticking;
    std::chrono::time_point<std::chrono::system_clock> lastMove; //TODO start using 
//    std::chrono::time_point<std::chrono::system_clock> lastBlackMove;
};
struct ChessMove {
    std::string from;
    std::string to;
};
struct Player
{
    PlayerTypes type;
    std::string usertoken;
    std::shared_ptr<MoveResultStream> resultStream;
    bool askingForDraw = false;
    std::chrono::time_point<std::chrono::system_clock> askingForDrawTimstamp;
    int64_t SecSinceAskedForDraw() { 
        //std::cout << usertoken << "Now: " << std::to_string(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()) << "Last: " << std::to_string(std::chrono::duration_cast<std::chrono::seconds>(askingForDrawTimstamp.time_since_epoch()).count()) << std::endl << std::flush;
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - askingForDrawTimstamp).count(); }
};