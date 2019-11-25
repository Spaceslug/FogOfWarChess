#pragma once
#include <functional>
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
        if (startPos % 8 == 0) return -1;
        return startPos - 1;
    }
    // static const std::string RightOne(const std::string& startPos)
    // {
    //     return std::to_string(((char)startPos[0]) + 1) + std::to_string(startPos[1]);
    // }
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

    static void GetLegalMoves(std::vector<int>& legalMoves, std::vector<Field>& board, int from, bool visionBoard[]){
        
        switch (board[from].Pice)
        {
            case ChessPice::Non:
                return;
            case ChessPice::BlackPawn:
                std::function<int(int)> moveDir = board[from].HasWhitePice()?UpOne:DownOne;
                AddPwnAttack(legalMoves, board, LeftOne(DownOne(from)), visionBoard);
                AddPwnAttack(legalMoves, board, RightOne(DownOne(from)), visionBoard);
                
                string bpDown = DownOne(fromField.FieldName);
                bool bpDownLegal = LegalPos(bpDown) && BPToFi(state, bpDown).Pice == ChessPice::Non;
                if (bpDownLegal)
                {
                    legalMoves.Add((bpDown, null));
                    if (!fromField.Field.FirstMove) break;

                    string bpDown2 = DownOne(bpDown);
                    Field bpDown2Field = BPToFi(state, bpDown2);
                    bool bpDown2Legal = LegalPos(bpDown2) && bpDown2Field.Pice == ChessPice::Non;
                    if (bpDown2Legal) legalMoves.Add((bpDown2, new List<FieldState> { new FieldState(bpDown, BPToFi(state, bpDown)) }));
                }
                break;
            case ChessPice::BlackKing:
            case ChessPice::WhiteKing:
                AddMove(state, fromField, legalMoves, UpOne(fromField.FieldName));
                AddMove(state, fromField, legalMoves, DownOne(fromField.FieldName));
                AddMove(state, fromField, legalMoves, LeftOne(fromField.FieldName));
                AddMove(state, fromField, legalMoves, RightOne(fromField.FieldName));

                AddMove(state, fromField, legalMoves, UpOne(LeftOne(fromField.FieldName)));
                AddMove(state, fromField, legalMoves, LeftOne(DownOne(fromField.FieldName)));
                AddMove(state, fromField, legalMoves, DownOne(RightOne(fromField.FieldName)));
                AddMove(state, fromField, legalMoves, RightOne(UpOne(fromField.FieldName)));

                AddCastelling(state, fromField, legalMoves, RightOne);
                AddCastelling(state, fromField, legalMoves, LeftOne);
                break;
            case ChessPice::BlackQueen:
            case ChessPice::WhiteQueen:
                AddMovesTillEnd(state, fromField, legalMoves, pos => UpOne(LeftOne(pos)));
                AddMovesTillEnd(state, fromField, legalMoves, pos => UpOne(RightOne(pos)));
                AddMovesTillEnd(state, fromField, legalMoves, pos => DownOne(LeftOne(pos)));
                AddMovesTillEnd(state, fromField, legalMoves, pos => DownOne(RightOne(pos)));
                AddMovesTillEnd(state, fromField, legalMoves, UpOne);
                AddMovesTillEnd(state, fromField, legalMoves, DownOne);
                AddMovesTillEnd(state, fromField, legalMoves, LeftOne);
                AddMovesTillEnd(state, fromField, legalMoves, RightOne);
                break;
            case ChessPice::BlackBishop:
            case ChessPice::WhiteBishop:
                AddMovesTillEnd(state, fromField, legalMoves, pos => UpOne(LeftOne(pos)));
                AddMovesTillEnd(state, fromField, legalMoves, pos => UpOne(RightOne(pos)));
                AddMovesTillEnd(state, fromField, legalMoves, pos => DownOne(LeftOne(pos)));
                AddMovesTillEnd(state, fromField, legalMoves, pos => DownOne(RightOne(pos)));
                break;
            case ChessPice::BlackKnight:
            case ChessPice::WhiteKnight:
                AddMove(state, fromField, legalMoves, UpOne(UpOne(LeftOne(fromField.FieldName))));
                AddMove(state, fromField, legalMoves, UpOne(UpOne(RightOne(fromField.FieldName))));
                AddMove(state, fromField, legalMoves, LeftOne(LeftOne(UpOne(fromField.FieldName))));
                AddMove(state, fromField, legalMoves, LeftOne(LeftOne(DownOne(fromField.FieldName))));
                AddMove(state, fromField, legalMoves, RightOne(RightOne(UpOne(fromField.FieldName))));
                AddMove(state, fromField, legalMoves, RightOne(RightOne(DownOne(fromField.FieldName))));
                AddMove(state, fromField, legalMoves, DownOne(DownOne(LeftOne(fromField.FieldName))));
                AddMove(state, fromField, legalMoves, DownOne(DownOne(RightOne(fromField.FieldName))));
                break;
            case ChessPice::BlackRook:
            case ChessPice::WhiteRook:
                AddMovesTillEnd(state, fromField, legalMoves, UpOne);
                AddMovesTillEnd(state, fromField, legalMoves, DownOne);
                AddMovesTillEnd(state, fromField, legalMoves, LeftOne);
                AddMovesTillEnd(state, fromField, legalMoves, RightOne);
                break;
            case ChessPice::WhitePawn:
                AddPwnAttack(state, fromField, legalMoves, LeftOne(UpOne(fromField.FieldName)));
                AddPwnAttack(state, fromField, legalMoves, RightOne(UpOne(fromField.FieldName)));

                string wpUp = UpOne(fromField.FieldName);
                bool wpUpLegal = LegalPos(wpUp) && BPToFi(state, wpUp).Pice == ChessPice::Non;
                if (wpUpLegal)
                {
                    legalMoves.Add((wpUp, null));
                    if (!fromField.Field.FirstMove) break;

                    string wpUp2 = UpOne(wpUp);
                    Field wpUp2Field = BPToFi(state, wpUp2);
                    bool wpUp2Legal = LegalPos(wpUp2) && wpUp2Field.Pice == ChessPice::Non;
                    if (wpUp2Legal) legalMoves.Add((wpUp2, new List<FieldState> { new FieldState(wpUp, BPToFi(state, wpUp)) }));
                }
                break;
            default:
                return legalMoves;
        }
        return legalMoves;
    }

    static void AddPwnAttack(std::vector<int>& legalMoves, std::vector<Field>& board, int to, bool visionBoard[])
        {
            bool moveIsWhite = board[from].HasWhitePice();
            if (!LegalPos(endPos)) return;
            Field currentField = state.GetFieldAt(endPos);
            if ((moveIsWhite ? Field.HasBlackPice(currentField) : Field.HasWhitePice(currentField)))
            {
                moveList.Add((endPos, null));
            }
            else if (currentField.AnPassan_able)
            {
                string anPs = (moveIsWhite ? DownOne(endPos) : UpOne(endPos));
                Field anPsField = BPToFi(state, anPs);
                if ((moveIsWhite ? anPsField.HasBlackPice() : anPsField.HasWhitePice()))
                {
                    moveList.Add((endPos, new List<FieldState> { new FieldState(anPs, anPsField) }));
                }
            }
        }
};
