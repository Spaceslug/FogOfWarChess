#pragma once
#include <vector>
#include <locale>
#include <cwctype>
#include <locale>
#include <string>
#include <list>
#include <random>
#include <chrono>
#include <algorithm>  
#include "gamerules.h"


class Sfen{
    public:
    static const std::string GenNormalChess()
    {
        return "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w AHah - 0 1";
    }

    static const std::string GenSlugRandom()
    {
        std::vector<ChessPice> bagOfBlack = {ChessPice::BlackKing, ChessPice::BlackQueen, ChessPice::BlackRook, ChessPice::BlackRook, ChessPice::BlackBishop, ChessPice::BlackBishop, ChessPice::BlackKnight, ChessPice::BlackKnight};
        std::vector<ChessPice> bagOfWhite = {ChessPice::WhiteKing, ChessPice::WhiteQueen, ChessPice::WhiteRook, ChessPice::WhiteRook, ChessPice::WhiteBishop, ChessPice::WhiteBishop, ChessPice::WhiteKnight, ChessPice::WhiteKnight};
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        auto rand = std::default_random_engine(seed);
        shuffle(bagOfBlack.begin(), bagOfBlack.end(), rand); 
        shuffle(bagOfWhite.begin(), bagOfWhite.end(), rand);
        std::stringstream ss;
         std::for_each(bagOfBlack.begin(), bagOfBlack.end(), [&ss](ChessPice& pice){
            ss << Field::PiceChar(pice);
        });
        ss << "/pppppppp/8/8/8/8/PPPPPPPP/";
        std::for_each(bagOfWhite.begin(), bagOfWhite.end(), [&ss](ChessPice& pice){
           ss << Field::PiceChar(pice);
        });
        ss << " w - - 0 1";
        return ss.str();
    }

    /**
     * True if succsessful. Only support start 0 moves done
     * */
    static int WriteSfenPicesToBoard(std::vector<Field>& board, const std::string& sfen)
    {
        // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w AHah - 0 1
        // normal startingpoint shredder-fen

        //Field(ChessPice pice, bool anPassanAble, bool firstMove, bool piceCapturedLastMove)
        int fenIndex = 0;
        int columnIndex = 0;
        int rowindex = 7;
        bool picesLoop = true;
        while (picesLoop)
        {
            char character = sfen[fenIndex];
            //std::cout << "fenchar  " << character << std::endl;
            switch (character)
            {
            case '/':
                columnIndex = 0;
                rowindex--;
                break;
            case ' ':
                picesLoop = false;
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                columnIndex+= character - '0';
                break;
            case 'p':
                if(rowindex == 6){
                    board[BIdx(columnIndex, rowindex)] = Field(ChessPice::BlackPawn, false, true, false);
                }else{
                    board[BIdx(columnIndex, rowindex)] = Field(ChessPice::BlackPawn, false, false, false);
                }
                columnIndex++;
                break;
            case 'P':
                if(rowindex == 1){
                    board[BIdx(columnIndex, rowindex)] = Field(ChessPice::WhitePawn, false, true, false);
                }else{
                    board[BIdx(columnIndex, rowindex)] = Field(ChessPice::WhitePawn, false, false, false);
                }
                columnIndex++;
                break;
            case 'n':
                board[BIdx(columnIndex, rowindex)] = Field(ChessPice::BlackKnight);
                columnIndex++;
                break;
            case 'N':
                board[BIdx(columnIndex, rowindex)] = Field(ChessPice::WhiteKnight);
                columnIndex++;
                break;
                case 'b':
                board[BIdx(columnIndex, rowindex)] = Field(ChessPice::BlackBishop);
                columnIndex++;
                break;
            case 'B':
                board[BIdx(columnIndex, rowindex)] = Field(ChessPice::WhiteBishop);
                columnIndex++;
                break;
            case 'q':
                board[BIdx(columnIndex, rowindex)] = Field(ChessPice::BlackQueen);
                columnIndex++;
                break;
            case 'Q':
                board[BIdx(columnIndex, rowindex)] = Field(ChessPice::WhiteQueen);
                columnIndex++;
                break;
            case 'k': 
                board[BIdx(columnIndex, rowindex)] = Field(ChessPice::BlackKing);
                columnIndex++;
                break;
            case 'K': 
                board[BIdx(columnIndex, rowindex)] = Field(ChessPice::WhiteKing);
                columnIndex++;
                break;
            case 'r': 
                board[BIdx(columnIndex, rowindex)] = Field(ChessPice::BlackRook);
                columnIndex++;
                break;
            case 'R': 
                board[BIdx(columnIndex, rowindex)] = Field(ChessPice::WhiteRook);
                columnIndex++;
                break;
            default:
                return false;
            }
            fenIndex++;
        }
        // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w AHah - 0 1
        fenIndex += 2; //ignore whose move it is
        bool castleLoop = true;
        while (castleLoop)
        {
            char character = sfen[fenIndex];
            //std::cout << "fenchar cast pos " << std::string(1,std::tolower(character))+'1' << std::endl;
            if(character == '-'){
                fenIndex++;
                castleLoop = false;
            }else if( character == ' '){
                castleLoop = false;
            }else if(character >= 'A' && character <= 'H'){
                int posIndex = GameRules::BoardPosToIndex(std::string(1,std::tolower(character, std::locale()))+'1');
                //std::cout << "fenchar cast posIndex  " << std::to_string(posIndex) << std::endl;
                if(board[posIndex].Pice == ChessPice::WhiteRook){
                    for (auto&& field : board)
                    {
                        if(field.Pice == ChessPice::WhiteKing){
                            field.FirstMove = true;
                        }
                    }
                    board[posIndex].FirstMove = true;
                }
            }else if(character >= 'a' && character <= 'h'){
                int posIndex = GameRules::BoardPosToIndex(std::string(1,std::tolower(character, std::locale()))+'8');
                if(board[posIndex].Pice == ChessPice::BlackRook){
                    for (auto&& field : board)
                    {
                        if(field.Pice == BlackKing){
                            field.FirstMove = true;
                        }
                    }
                    board[posIndex].FirstMove = true;
                }
            }
            fenIndex++;
        }

        if(sfen[fenIndex] != '-'){
            //std::cout << "fenchar cast anpass " << std::string(1,sfen[fenIndex]) + std::string(1,sfen[fenIndex+1]) << std::endl;
            board[GameRules::BoardPosToIndex(sfen.substr(fenIndex, 2))].AnPassan_able = true;
            fenIndex+= 2;
        }else{
            fenIndex++;
        }

        //Set fieldName
        for(int i = 0; i < 64; i++)
        {
            board[i].fieldname = GameRules::BoardPosRef(i);
        }
        
        std::cout << "Sfen board builder finished" << std::endl;
        
        return true;

    }

    private:
    static int BIdx(int columIndex, int rowIndex){
        return GameRules::IndexFromColRow(columIndex, rowIndex);
    }

};