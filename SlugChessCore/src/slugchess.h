#pragma once
#include <map>
#include <vector>
#include <list>
#include <set>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "visionrules.h"
#include "sfen.h"

#include "shared.h"

class SlugChess {
    public:
    enum EndResult{
        StillPlaying = 0,
        Draw = 1,
        WhiteWin = 2,
        BlackWin = 3,
    };
    enum Perspective{
        White = 0,
        Black = 1,
        Both = 2,
    };
    SlugChess(const std::string& sfenString, const VisionRules& visionRules);

    void DoMove(const std::string& from, const std::string& to);
    const std::string ToFenString();

    void PrintBoard(std::stringstream& ss, bool whitePlayer);
    void PrintDebugBoard(std::stringstream& ss);
    void PrintWhiteVision(std::stringstream& ss);
    void PrintBlackVision(std::stringstream& ss);
    void PrintVisionBoard(std::stringstream& ss, bool visionBoard[]);
    void PrintSanMoves(std::stringstream& ss);

    std::string From(Perspective perspective);
    std::string To(Perspective perspective);

    std::vector<bool> GetWhiteVision(){ return std::vector<bool>(std::begin(_whiteVision), std::end(_whiteVision)); }
    std::vector<bool> GetBlackVision(){ return std::vector<bool>(std::begin(_blackVision), std::end(_blackVision)); }
    std::vector<ChessPice> GetPices(){ 
        std::vector<ChessPice> pices(64);
        for (int i = 0; i < 64; i++)
        {
            pices[i] = _board[i].Pice;
        }
        
        return pices;
    }

    void SetEnd(EndResult endResult){ _gameEnd = endResult; }
    EndResult Result(){ return _gameEnd; }

    bool LegalMove(std::string& from, std::string& to){ 
        if(from[0] < 'a' || from[0] > 'h' 
            ||from[1] < '1' || from[1] > '8'
            ||to[0] < 'a' || to[0] > 'h'
            ||to[1] < '1' || to[1] > '8' ) return false;
        if(_legalMoves.count(BPToIndx(from)) > 0){
            std::vector<int> vector = _legalMoves[BPToIndx(from)];
            return std::find(vector.begin(), vector.end(), BPToIndx(to)) != vector.end();
        }
        return false; 
    }

    std::map<int, std::vector<int>>* LegalMovesRef(){ return &_legalMoves; }
    std::map<int, std::vector<int>>* LegalWhiteMovesRef(){ return &_legalWhiteMoves; }
    std::map<int, std::vector<int>>* LegalBlackMovesRef(){ return &_legalBlackMoves; }
    std::map<int, std::vector<int>>* ShadowWhiteMovesRef(){ return &_shadowWhiteMoves; }
    std::map<int, std::vector<int>>* ShadowBlackMovesRef(){ return &_shadowBlackMoves; }

    std::set<int> Checks(Perspective perspective);
    //std::list<int>* BlackChecksRef(){ return &_blackFieldsThatCheck; }
    //std::list<int>* WhiteChecksRef(){ return &_whiteFieldsThatCheck; }

    bool WhitesTurn(){return _whiteTurn; }
    ChessPice LastCaptured(){ return _lastCaptured; }
    void CalPossibleCastles();
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

    static bool visionBoardTrue [64];


    static const int32_t BPToIndx(std::string& pos)
    {
        return GameRules::BoardPosToIndex(pos);
    }
    static std::string BP(int index)
    {
        return GameRules::BoardPos(index);
    }
    private:
    Field ExecuteMove(const std::string from, const std::string to);
    Field ExecuteMove(int from, int to);
    //void WriteLan(const std::string& from, const std::string& to);
    void PrintBoard(std::stringstream& ss, bool visionBoard[]);
    void PrintDebugBoard(std::stringstream& ss, bool visionboard[]);
    void WriteMoveSan(const std::string& from, const std::string& to);
    void CalculateVision();
    void CalculateLegalMoves();
    void CalculateLegalShadowMoves();
    void FindChecks();
    int GetFieldWithPice(ChessPice pice);
    bool* VisionBoardPerspective(Perspective perspective);
 
    static void CalculateLegalMoves(std::vector<Field>& board, bool visionBoard[]);


    std::string _fenString;
    std::list<std::pair<int, int>> _moves;
    std::stringstream _sanMoves;
    EndResult _gameEnd;
    int _halfTurnSinceCapture = 0;
    int _turn = 1;
    bool _whiteTurn;
    VisionRules _rules; 
    std::vector<Field> _board;
    bool _whiteVision [64];
    bool _blackVision [64];
    int _lastCaptureField = -1;
    std::map<int, std::vector<int>> _legalMoves;
    std::map<int, std::vector<int>> _legalWhiteMoves;
    std::map<int, std::vector<int>> _legalBlackMoves;
    std::map<int, std::vector<int>> _shadowWhiteMoves;
    std::map<int, std::vector<int>> _shadowBlackMoves;
    // Starts with king and ends with rook
    std::list<int> _possibleCastles;
    std::list<std::tuple<ChessPice,int>> _killedPices; //chesspice and postion it died in
    ChessPice _lastCaptured = ChessPice::Non;
    std::list<int> _blackFieldsThatCheck;
    std::list<int> _whiteFieldsThatCheck;

};
