#pragma once
#include "visionrules.h"

class GameRules {
    private:
    static const std::map<std::string, int32_t> _boardPosToIndex;
    static const std::vector<std::string> _boardPos;
    public:
    //static const std::map<std::string, int32_t> BoardPosToIndex;
    //static const std::vector<std::string> BoardPos;


    static const int BoardPosToIndex(const std::string& pos){
        return _boardPosToIndex.at(pos);
    }
    static const std::string& BoardPos(int index){
        return _boardPos[index];
    }
    static const std::string* BoardPosRef(int index){
        return &_boardPos[index];
    }
    static bool LegalPos(const std::string& pos)
    {
        int pos1Int = (int)pos[0];
        int pos2Int = (int)pos[1];
        return pos1Int >= 97 && pos1Int <= 104 && pos2Int >= 49 && pos2Int <= 56;
    }
    static bool LegalPos(int pos)
    {
        return pos >= 0 && pos < 64;
    }
    static bool LegalColumn(int column)
    {
        return column >= 0 && column < 8;
    }
    static bool LegalRow(int pos)
    {
        return LegalColumn(pos);
    }
    static int IndexFromColRow(int column, int row)
    {
        return column + (8 * row);
    }
    static std::string UpOne(const std::string& startPos)
    {
        return std::to_string(startPos[0]) + std::to_string(((char)startPos[1]) + 1);
    }
    static int UpOne(int startPos)
    {
        return startPos + 8;
    }
    static std::string DownOne(const std::string& startPos)
    {
        return std::to_string(startPos[0]) + std::to_string(((char)startPos[1]) - 1);
    }
    static int DownOne(int startPos)
    {
        return startPos - 8;
    }
    static const std::string LeftOne(const std::string& startPos)
    {
        return std::to_string(((char)startPos[0]) - 1) + std::to_string(startPos[1]);
    }
    static int LeftOne(int startPos)
    {
        if (startPos % 8 == 0) return -1;
        return startPos - 1;
    }
    static const std::string RightOne(const std::string& startPos)
    {
        return std::to_string(((char)startPos[0]) + 1) + std::to_string(startPos[1]);
    }
    static int RightOne(int startPos)
    {
        if (startPos % 8 == 7) return -1;
        return startPos + 1;
    }

    static void AddPiceVision(std::vector<Field>& board, int index, Rules& vision, bool visionBoard[])
    {
        int column = index % 8 - vision.ViewRange;
        int row = index / 8 - vision.ViewRange;
        //int start = index - visionRules.ViewRange - visionRules.ViewRange* 8;
        
        int iterate = vision.ViewRange * 2 + 1;
        for (int i = 0; i < iterate; i++)
        {
            if (!LegalColumn(column + i)) continue;
            for (int j = 0; j < iterate; j++)
            {
                if (!LegalRow(row+j)) continue;
                visionBoard[IndexFromColRow(column + i, row + j)] = true;
            }
        }
    }
};
