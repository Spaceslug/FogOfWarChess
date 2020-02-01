#pragma once
#include <functional>
#include <iostream>
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
    // static std::string UpOne(const std::string& startPos)
    // {
    //     return std::to_string(startPos[0]) + std::to_string(((char)startPos[1]) + 1);
    // }
    static int UpOne(int startPos)
    {
        return startPos + 8;
    }
    // static std::string DownOne(const std::string& startPos)
    // {
    //     return std::to_string(startPos[0]) + std::to_string(((char)startPos[1]) - 1);
    // }
    static int DownOne(int startPos)
    {
        return startPos - 8;
    }
    // static const std::string LeftOne(const std::string& startPos)
    // {
    //     return std::to_string(((char)startPos[0]) - 1) + std::to_string(startPos[1]);
    // }
    static int LeftOne(int startPos)
    {
        if (startPos % 8 == 0) return -99;
        return startPos - 1;
    }
    // static const std::string RightOne(const std::string& startPos)
    // {
    //     return std::to_string(((char)startPos[0]) + 1) + std::to_string(startPos[1]);
    // }
    static int RightOne(int startPos)
    {
        if (startPos % 8 == 7) return -99;
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

    static void GetLegalMoves(std::vector<int>& legalMoves, std::vector<Field>& board, int from, bool visionBoard[]){
        
        //std::cout << "Getting legal moves from " << std::to_string(from) << std::endl;
        switch (board[from].Pice)
        {
            case ChessPice::Non:
            {
                return;
            }
            case ChessPice::BlackPawn:
            {
                std::function<int(int)> moveDir = board[from].HasWhitePice()?UpOne:DownOne;
                AddPwnAttack(legalMoves, board, from, LeftOne(DownOne(from)), visionBoard);
                AddPwnAttack(legalMoves, board, from, RightOne(DownOne(from)), visionBoard);
                
                AddPawnMove(legalMoves, board, from, DownOne, visionBoard);

                break;
            }
            case ChessPice::BlackKing:
            case ChessPice::WhiteKing:
            {
                AddAttackMove(legalMoves, board, from, UpOne(from), visionBoard);
                AddAttackMove(legalMoves, board, from, DownOne(from), visionBoard);
                AddAttackMove(legalMoves, board, from, LeftOne(from), visionBoard);
                AddAttackMove(legalMoves, board, from, RightOne(from), visionBoard);

                AddAttackMove(legalMoves, board, from, UpOne(LeftOne(from)), visionBoard);
                AddAttackMove(legalMoves, board, from, LeftOne(DownOne(from)), visionBoard);
                AddAttackMove(legalMoves, board, from, DownOne(RightOne(from)), visionBoard);
                AddAttackMove(legalMoves, board, from, RightOne(UpOne(from)), visionBoard);

                if(board[from].FirstMove){
                    AddCastelling(legalMoves, board, from, RightOne, visionBoard);
                    AddCastelling(legalMoves, board, from, LeftOne, visionBoard);
                }
                
                break;
            }
            case ChessPice::BlackQueen:
            case ChessPice::WhiteQueen:
            {
                AddAttackMovesTillEnd(legalMoves, board, from, [](int pos){return UpOne(LeftOne(pos));}, visionBoard);
                AddAttackMovesTillEnd(legalMoves, board, from, [](int pos){return UpOne(RightOne(pos));}, visionBoard);
                AddAttackMovesTillEnd(legalMoves, board, from, [](int pos){return DownOne(LeftOne(pos));}, visionBoard);
                AddAttackMovesTillEnd(legalMoves, board, from, [](int pos){return DownOne(RightOne(pos));}, visionBoard);

                AddAttackMovesTillEnd(legalMoves, board, from, UpOne, visionBoard);
                AddAttackMovesTillEnd(legalMoves, board, from, DownOne, visionBoard);
                AddAttackMovesTillEnd(legalMoves, board, from, LeftOne, visionBoard);
                AddAttackMovesTillEnd(legalMoves, board, from, RightOne, visionBoard);
                break;
            }
            case ChessPice::BlackBishop:
            case ChessPice::WhiteBishop:
            {
                //std::cout << "Bsiop Left then Up -";
                AddAttackMovesTillEnd(legalMoves, board, from, [](int pos){return UpOne(LeftOne(pos));}, visionBoard);
                //std::cout << "Bsiop Right then Up -";
                AddAttackMovesTillEnd(legalMoves, board, from, [](int pos){return UpOne(RightOne(pos));}, visionBoard);
                //std::cout << "Bsiop Left then Down -";
                AddAttackMovesTillEnd(legalMoves, board, from, [](int pos){return DownOne(LeftOne(pos));}, visionBoard);
                //std::cout << "Bsiop Right then Up -" << std::endl;
                AddAttackMovesTillEnd(legalMoves, board, from, [](int pos){return DownOne(RightOne(pos));}, visionBoard);
                break;
            }
            case ChessPice::BlackKnight:
            case ChessPice::WhiteKnight:
            {
                AddAttackMove(legalMoves, board, from, UpOne(UpOne(LeftOne(from))), visionBoard);
                AddAttackMove(legalMoves, board, from, UpOne(UpOne(RightOne(from))), visionBoard);
                AddAttackMove(legalMoves, board, from, LeftOne(LeftOne(UpOne(from))), visionBoard);
                AddAttackMove(legalMoves, board, from, LeftOne(LeftOne(DownOne(from))), visionBoard);
                AddAttackMove(legalMoves, board, from, RightOne(RightOne(UpOne(from))), visionBoard);
                AddAttackMove(legalMoves, board, from, RightOne(RightOne(DownOne(from))), visionBoard);
                AddAttackMove(legalMoves, board, from, DownOne(DownOne(LeftOne(from))), visionBoard);
                AddAttackMove(legalMoves, board, from, DownOne(DownOne(RightOne(from))), visionBoard);
                break;
            }
            case ChessPice::BlackRook:
            case ChessPice::WhiteRook:
            {
                AddAttackMovesTillEnd(legalMoves, board, from, UpOne, visionBoard);
                AddAttackMovesTillEnd(legalMoves, board, from, DownOne, visionBoard);
                AddAttackMovesTillEnd(legalMoves, board, from, LeftOne, visionBoard);
                AddAttackMovesTillEnd(legalMoves, board, from, RightOne, visionBoard);
                break;
            }
            case ChessPice::WhitePawn:
            {
                AddPwnAttack(legalMoves, board, from, LeftOne(UpOne(from)), visionBoard);
                AddPwnAttack(legalMoves, board, from, RightOne(UpOne(from)), visionBoard);

                AddPawnMove(legalMoves, board, from, UpOne, visionBoard);
                break;
            }
            default:
            {
                return;
            }
        }
        return;
    }

    static void AddPwnAttack(std::vector<int>& legalMoves, std::vector<Field>& board, int from, int to, bool visionBoard[])
    {
        if (LegalPos(to) && visionBoard[to]){
            if(board[to].AnPassan_able) {
                legalMoves.push_back(to);
                return;
            }
            if(board[to].Pice == Non){
                return;
            }
            if(board[from].HasWhitePice() != board[to].HasWhitePice()){
                legalMoves.push_back(to);
            }
        }
    }

    

    static void AddPawnMove(std::vector<int>& legalMoves, std::vector<Field>& board, int from, std::function<int(int)> moveDir,  bool visionBoard[])
    {
        int to = moveDir(from);
        if(LegalPos(to) && visionBoard[to] && board[to].Pice == Non){
            legalMoves.push_back(to);
            if(board[from].FirstMove){
                int toto = moveDir(to);
                if(LegalPos(toto) && visionBoard[toto] && board[toto].Pice == Non){
                    legalMoves.push_back(toto);
                }
            }

        }
    }

    static void AddAttackMove(std::vector<int>& legalMoves, std::vector<Field>& board, int from, int to, bool visionBoard[])
    {
        if (LegalPos(to) && visionBoard[to]){
            if(board[to].Pice == Non){
                legalMoves.push_back(to);
                return;
            }
            if(board[from].HasWhitePice() != board[to].HasWhitePice()){
                legalMoves.push_back(to);
            }
        }
    }

    static void AddAttackMovesTillEnd(std::vector<int>& legalMoves, std::vector<Field>& board, int from, std::function<int(int)> moveFunc, bool visionBoard[])
    {
        bool validMove = true;
        int currentField = from;
        bool moveIsWhite = board[from].HasWhitePice();
        while (true)
        {
            currentField = moveFunc(currentField);
            if (!LegalPos(currentField)) break;
            validMove = !(moveIsWhite ? board[currentField].HasWhitePice() : board[currentField].HasBlackPice());

            if (validMove && visionBoard[currentField])
            {
                legalMoves.push_back(currentField);
                if ((moveIsWhite ? board[currentField].HasBlackPice() : board[currentField].HasWhitePice()))
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }

    static void AddCastelling(std::vector<int>& legalMoves, std::vector<Field>& board, int from, std::function<int(int)> moveFunc, bool visionBoard[])
    {
        bool validMove = true;
        int currentField = from;
        bool moveIsWhite = board[currentField].HasWhitePice();
        while (true)
        {
            currentField = moveFunc(currentField);
            if (!LegalPos(currentField)) break;
            if(!visionBoard[currentField]) break;

            validMove = !(moveIsWhite ? board[currentField].HasWhitePice() : board[currentField].HasBlackPice());
            if (validMove)
            {
                if ((moveIsWhite ? board[currentField].HasBlackPice() : board[currentField].HasWhitePice()))
                {
                    break;
                }
            }
            else if (board[currentField].Pice == (moveIsWhite ? ChessPice::WhiteRook : ChessPice::BlackRook) && board[currentField].FirstMove)
            {
                legalMoves.push_back(currentField);
                break;
            }
            else
            {
                break;
            }
        }
    }
};
