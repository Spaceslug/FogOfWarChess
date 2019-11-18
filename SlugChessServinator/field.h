#pragma once
#include <vector>

enum ChessPice {
    Non = 0,
    BlackPawn = 1,
    BlackKnight = 2,
    BlackBishop = 3,
    BlackRook = 4,
    BlackQueen = 5,
    BlackKing = 6,
    WhitePawn = 7,
    WhiteKnight = 8,
    WhiteBishop = 9,
    WhiteRook = 10,
    WhiteQueen = 11,
    WhiteKing = 12
};

struct Field {
    public:
    std::string* fieldname; //pointer to string in BoardPos
    bool AnPassan_creating;
    bool AnPassan_able;
    bool Rokade_able;
    bool FirstMove;
    bool PiceCapturedLastMove;
    ChessPice Pice;

    Field(ChessPice pice, bool anPassanCreating, bool anPassanAble, bool rokadeAble, bool firstMove, bool piceCapturedLastMove)
    {
        Pice = pice;
        AnPassan_creating = anPassanCreating;
        AnPassan_able = anPassanAble;
        Rokade_able = rokadeAble;
        FirstMove = firstMove;
        PiceCapturedLastMove = piceCapturedLastMove;
    }

    Field(ChessPice pice) : Field(pice, false, false, false, false, false)
    {

    }

    bool HasBlackPice()
    {
       switch (Pice)
        {
            case BlackKing:
            case BlackQueen:
            case BlackBishop:
            case BlackKnight:
            case BlackRook:
            case BlackPawn:
                return true;
            default:
                return false;
        }
    }

    bool HasWhitePice()
    {
        switch (Pice)
        {
            case WhiteKing:
            case WhiteQueen:
            case WhiteBishop:
            case WhiteKnight:
            case WhiteRook:
            case WhitePawn:
                return true;
            default:
                return false;
        }
    }

    static bool BlackPice(ChessPice pice)
    {
        switch (pice)
        {
            case BlackKing:
            case BlackQueen:
            case BlackBishop:
            case BlackKnight:
            case BlackRook:
            case BlackPawn:
                return true;
            default:
                return false;
        }
    }

    static bool WhitePice(ChessPice pice)
    {
        switch (pice)
        {
            case WhiteKing:
            case WhiteQueen:
            case WhiteBishop:
            case WhiteKnight:
            case WhiteRook:
            case WhitePawn:
                return true;
            default:
                return false;
        }
    }
};

