#pragma once
#include <vector>
#include "gamerules.h"

class Sfen{
    public:
    /**
     * True if succsessful
     * */
    static bool WriteSfenPicesToBoard(std::vector<Field>& board, const std::string& sfen){
        // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w AHah - 0 1
        // normal startingpoint shredder-fen
        int fenIndex = 0;
        int columnIndex = 0;
        int rowindex = 7;
        bool picesLoop = true;
        while (picesLoop)
        {
            char character = sfen[fenIndex];
            switch (character)
            {
            case 'p':
                //board[BIdx(columnIndex, rowindex)] = Field(ChessPice::WhitePawn, );
                break;
            
            default:
                return false;
            }
            /* code */
            fenIndex++;
        }

        return true;

    }

    private:
    static int BIdx(int columIndex, int rowIndex){
        return GameRules::IndexFromColRow(columIndex, rowIndex);
    }

};