#pragma once
#include <map>
#include <vector>
#include <iostream>
#include "visionrules.h"
#include "sfen.h"

class SlugChess {
    public:


    //SlugChess(int temp);
    SlugChess(const std::string& sfenString, const VisionRules& visionRules);


    
    int32_t GetLegalMove(std::string s);

    const std::string ToFenString();
    void PrintBoard(std::stringstream& ss);
    void PrintWhiteVision(std::stringstream& ss);
    void PrintBlackVision(std::stringstream& ss);

    void PrintVisionBoard(std::stringstream& ss, bool visionBoard[]);

    void CalculateVision();
    void CalculateLegalMoves();

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

    int _halfturn;
    bool _whiteTurn;
    VisionRules _rules; 
    std::vector<Field> _board;
    bool _whiteVision [64];
    bool _blackVision [64];
    int _lastCaptureField = -1;
    std::map<int, std::vector<int>> _legalMoves;
    std::map<int, std::vector<int>> _legalShadowMoves;

};