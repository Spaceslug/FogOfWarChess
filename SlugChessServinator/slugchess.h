#pragma once
#include <map>
#include <vector>
#include <list>
#include <iostream>
#include "visionrules.h"
#include "sfen.h"

class SlugChess {
    public:


    //SlugChess(int temp);
    SlugChess(const std::string& sfenString, const VisionRules& visionRules);


    void DoMove(const std::string& from, const std::string& to);


    const std::string ToFenString();
    void PrintBoard(std::stringstream& ss, bool whitePlayer);
    void PrintDebugBoard(std::stringstream& ss);
    void PrintWhiteVision(std::stringstream& ss);
    void PrintBlackVision(std::stringstream& ss);
    void PrintVisionBoard(std::stringstream& ss, bool visionBoard[]);

    void CalculateVision();
    void CalculateLegalMoves();

    char CurrentPlayer(){ return _whiteTurn?'w':'b'; };

    void CalPossibleCastles();

    void CleanAnPassants()
    {
        //with test
        int index = 0;
        for(Field& field : _board)
        {
            if(field.AnPassan_able)
            {
                field.AnPassan_able = false;
                break;
            }
            index++;
        }
        if(index < 63 && _board[index].AnPassan_able){
            std::cout << "Clearing an passant failed" << std::flush << std::endl;
        }
    }

    static const int32_t BPToIndx(std::string& pos)
    {
        return GameRules::BoardPosToIndex(pos);
    }
    static void CalculateLegalMoves(std::vector<Field>& board, bool visionBoard[]);
    private:

    Field ExecuteMove(const std::string from, const std::string to);
    Field ExecuteMove(int from, int to);
    void PrintBoard(std::stringstream& ss, bool visionBoard[]);
    void PrintDebugBoard(std::stringstream& ss, bool visionboard[]);

    int _halfturn;
    bool _whiteTurn;
    VisionRules _rules; 
    std::vector<Field> _board;
    bool _whiteVision [64];
    bool _blackVision [64];
    int _lastCaptureField = -1;
    std::map<int, std::vector<int>> _legalMoves;
    std::map<int, std::vector<int>> _legalShadowMoves;
    // Starts with king and ends with rook
    std::list<int> _possibleCastles;
    std::list<std::tuple<ChessPice,int>> _killedPices; //chesspice and postion it died in

};