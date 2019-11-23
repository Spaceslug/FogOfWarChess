#pragma once
#include <map>
#include <vector>
#include <iostream>
#include "visionrules.h"
#include "sfen.h"

class SlugChess {
    public:
    static const std::map<std::string, int32_t> BoardPosToIndex;

    static const std::vector<std::string> BoardPos;

    SlugChess(int temp);
    SlugChess(const std::string& sfenString);


    
    int32_t GetLegalMove(std::string s);

    static int32_t BPToIndx(std::string& pos)
    {
        return BoardPosToIndex.at(pos);
    }
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
    private:

    int halfturn;
    bool whiteTurn;
    VisionRules rules; 
    std::vector<Field> _board;
    bool _whiteVision [64];
    bool _blackVision [64];

};