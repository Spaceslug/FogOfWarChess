#pragma once
#include <map>
#include <vector>
#include "field.h"
//#include "gamerules.h"

class SlugChess {

    static const std::map<std::string, int32_t> BoardPosToIndex;

    static const std::vector<std::string> BoardPos;

    struct Rules{
        bool viewMoveFields;
        bool viewCaptureField;
        int viewRange;
    };

    struct VisionRules{
        bool enabled;
        Rules globalRules;
        std::map<ChessPice, Rules> overWriteRules;
    };

    bool whiteTurn;
    VisionRules rules; 
    
    int32_t GetLegalMove(std::string s);

    static int32_t BPToIndx(std::string& pos)
    {
        return BoardPosToIndex.at(pos);
    }
    private:

    std::vector<Field> _board;

};