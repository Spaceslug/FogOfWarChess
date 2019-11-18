namespace GameRules {

    bool PosLegalPos(std::string& pos)
    {
        int pos1Int = (int)pos[0];
        int pos2Int = (int)pos[1];
        return pos1Int >= 97 && pos1Int <= 104 && pos2Int >= 49 && pos2Int <= 56;
    }
    bool LegalPos(int pos)
    {
        return pos >= 0 && pos < 64;
    }
    bool LegalColumn(int column)
    {
        return column >= 0 && column < 8;
    }
    bool LegalRow(int pos)
    {
        return LegalColumn(pos);
    }
    int IndexFromColRow(int column, int row)
    {
        return column + (8 * row);
    }
    std::string PosUpOne(std::string& startPos)
    {
        return std::to_string(startPos[0]) + std::to_string(((char)startPos[1]) + 1);
    }
    int UpOne(int startPos)
    {
        return startPos + 8;
    }
    std::string PosDownOne(std::string& startPos)
    {
        return std::to_string(startPos[0]) + std::to_string(((char)startPos[1]) - 1);
    }
    int DownOne(int startPos)
    {
        return startPos - 8;
    }
    std::string PosLeftOne(std::string& startPos)
    {
        return std::to_string(((char)startPos[0]) - 1) + std::to_string(startPos[1]);
    }
    int LeftOne(int startPos)
    {
        if (startPos % 8 == 0) return -1;
        return startPos - 1;
    }
    std::string PosRightOne(std::string& startPos)
    {
        return std::to_string(((char)startPos[0]) + 1) + std::to_string(startPos[1]);
    }
    int RightOne(int startPos)
    {
        if (startPos % 8 == 7) return -1;
        return startPos + 1;
    }
}
