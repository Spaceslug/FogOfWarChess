#include "visionrules.h"

class GameRules {
    public:
    static bool LegalPos(std::string& pos)
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
    static std::string UpOne(std::string& startPos)
    {
        return std::to_string(startPos[0]) + std::to_string(((char)startPos[1]) + 1);
    }
    static int UpOne(int startPos)
    {
        return startPos + 8;
    }
    static std::string DownOne(std::string& startPos)
    {
        return std::to_string(startPos[0]) + std::to_string(((char)startPos[1]) - 1);
    }
    int DownOne(int startPos)
    {
        return startPos - 8;
    }
    std::string LeftOne(std::string& startPos)
    {
        return std::to_string(((char)startPos[0]) - 1) + std::to_string(startPos[1]);
    }
    static int LeftOne(int startPos)
    {
        if (startPos % 8 == 0) return -1;
        return startPos - 1;
    }
    static std::string RightOne(std::string& startPos)
    {
        return std::to_string(((char)startPos[0]) + 1) + std::to_string(startPos[1]);
    }
    static int RightOne(int startPos)
    {
        if (startPos % 8 == 7) return -1;
        return startPos + 1;
    }

    static void AddPiceVision(std::vector<Field>& board, int index, VisionRules& vision, bool visionBoard[])
    {
        int column = index % 8 - vision.globalRules.viewRange;
        int row = index / 8 - vision.globalRules.viewRange;
        //int start = index - visionRules.ViewRange - visionRules.ViewRange* 8;
        
        int iterate = vision.globalRules.viewRange * 2 + 1;
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
