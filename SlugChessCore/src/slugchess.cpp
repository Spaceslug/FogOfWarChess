#include "slugchess.h"

#include <iterator>
#ifdef WIN
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#endif



SlugChess::SlugChess(const std::string& sfenString, const VisionRules& visionRules){
    // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w AHah - 0 1
    // normal startingpoint shredder-fen
    _fenString = sfenString;
    _board = std::vector<Field>(64);
    _rules = visionRules;
    _killedPices.clear();
    _whiteTurn = true;
    _gameEnd = EndResult::StillPlaying;
    _turn = 1;
    _halfTurnSinceCapture = 0;
    Sfen::WriteSfenPicesToBoard(_board, sfenString);
    CalculateVision();
    CalculateLegalMoves();
    CalculateLegalShadowMoves();
    //CalPossibleCastles();
}


const std::string SlugChess::ToFenString(){
	return "";
}

void SlugChess::PrintBoard(std::stringstream& ss, bool whiteplayer){
    PrintBoard(ss, whiteplayer?_whiteVision:_blackVision);
}

void SlugChess::PrintBoard(std::stringstream& ss, bool visionboard[]){
    for(int row = 7; row >= 0;row--){
        for(int col = 0; col <= 7;col++){
            int index = GameRules::IndexFromColRow(col, row);
            if(visionboard[index]) {
                ss << "[ " << Field::PiceChar(_board[index].Pice) << " ]";

            }else{
                ss << "[ " << '#' << " ]";
            }
        }
        ss << std::endl;
    }   
}

void SlugChess::PrintDebugBoard(std::stringstream& ss){
    bool vision [64];
    std::fill(std::begin(vision), std::end(vision), true);
    PrintDebugBoard(ss, vision);
}

void SlugChess::PrintDebugBoard(std::stringstream& ss, bool visionboard[]){
    for(int row = 7; row >= 0;row--){
        for(int col = 0; col <= 7;col++){
            int index = GameRules::IndexFromColRow(col, row);
            if(!visionboard[index]) continue;
            ss << "[ " << Field::PiceChar(_board[index].Pice) << " ]";
        }
        ss << std::endl;
    }
    ss << "Castling ";
    for (auto field : _board)
    {
        if(field.Pice == ChessPice::WhiteKing && field.FirstMove){
            ss << "White ";
            for (auto field2 : _board)
            {
                if(field2.Pice == ChessPice::WhiteRook && field.FirstMove){
                    ss << *field2.fieldname << " ";
                }
            }
        }
        if(field.Pice == ChessPice::BlackKing && field.FirstMove){
            ss << "Black ";
            for (auto field2 : _board)
            {
                if(field2.Pice == ChessPice::BlackRook && field.FirstMove){
                    ss << *(field2.fieldname) << " ";
                }
            }
        }
    }
    ss << std::endl;
    std::string anPass = "-";
    for (auto&& field : _board)
    {
        if(field.AnPassan_able){
            anPass = *field.fieldname;
            break;
        }
    }
    ss << "An Passant: " << anPass << std::endl;
}

void SlugChess::PrintWhiteVision(std::stringstream& ss){
    for(int row = 7; row >= 0;row--){   
        for(int col = 0; col <= 7;col++){
            int index = GameRules::IndexFromColRow(col, row);
            if(_whiteVision[index]){
                ss << "[ " << Field::PiceChar(_board[index].Pice) << " ]";
            }else{
                ss << "[ " << "#"<< " ]";
            }
        }
        ss << std::endl;
    }
}

void SlugChess::PrintBlackVision(std::stringstream& ss){
    PrintVisionBoard(ss, _blackVision);
}

void SlugChess::PrintVisionBoard(std::stringstream& ss, bool visionBoard[]){
    for(int row = 7; row >= 0;row--){   
        for(int col = 0; col <= 7;col++){
            int index = GameRules::IndexFromColRow(col, row);
            if(visionBoard[index]){
                ss << "[ " << Field::PiceChar(_board[index].Pice) << " ]";
            }else{
                ss << "[ " << "#"<< " ]";
            }
        }
        ss << std::endl;
    }
}

void SlugChess::CalculateVision(){
    if (!_rules.enabled)
    {
        std::fill(std::begin(_whiteVision), std::end(_whiteVision), true);
        std::fill(std::begin(_blackVision), std::end(_blackVision), true);
        return;
    }
    std::fill(std::begin(_whiteVision), std::end(_whiteVision), false);
    std::fill(std::begin(_blackVision), std::end(_blackVision), false);
    for (int i = 0; i < 64; i++)
    {
        if (_board[i].Pice == ChessPice::Non) continue;
        GameRules::AddPiceVision(_board, 
                                    i, 
                                    _rules.overWriteRules.count(_board[i].Pice)>0? _rules.overWriteRules[_board[i].Pice] : _rules.globalRules
                                    , _board[i].HasWhitePice()? _whiteVision:_blackVision);
        if (_rules.globalRules.ViewMoveFields)
        {
            //_legalMoves.clear();
            std::map<int, std::vector<int>> visionMoves;
            bool visionBoard [64];
            std::fill(std::begin(visionBoard), std::end(visionBoard), true);
            
            for(int i = 0; i < 64;i++){
                if(_board[i].HasWhitePice()){
                    visionMoves[i];
                    GameRules::GetLegalMoves(visionMoves[i], _board, i, visionBoard);
                }
            }
            for (auto &&keyVal : visionMoves)
            {
                for (auto &&to : keyVal.second){
                    _whiteVision[to] = true;
                }
            }
            visionMoves.clear();
            for(int i = 0; i < 64;i++){
                if(_board[i].HasBlackPice()){
                    visionMoves[i];
                    GameRules::GetLegalMoves(visionMoves[i], _board, i, visionBoard);
                }
            }
            for (auto &&keyVal : visionMoves)
            {
                for (auto &&to : keyVal.second){
                    _blackVision[to] = true;
                }
            }
        }
    }
    if (_rules.globalRules.ViewCaptureField && _lastCaptureField != -1)
    {
        _whiteVision[_lastCaptureField] = true;
        _blackVision[_lastCaptureField] = true;
    }
}

void SlugChess::CalculateLegalMoves(){
    _legalMoves.clear();
    bool* visionBoard  = _whiteTurn?_whiteVision:_blackVision;
    for(int i = 0; i < 64;i++){
        if(_board[i].Pice != ChessPice::Non && (_whiteTurn == _board[i].HasWhitePice())){
            _legalMoves[i];
            GameRules::GetLegalMoves(_legalMoves[i], _board, i, visionBoard);
        }
    }
}

void SlugChess::CalculateLegalShadowMoves(){
    _legalShadowMoves.clear();
    bool visionBoard [64];
    std::fill(std::begin(visionBoard), std::end(visionBoard), true);
    for(int i = 0; i < 64;i++){
        if(_board[i].Pice != ChessPice::Non && (_whiteTurn == _board[i].HasBlackPice() )){
            _legalShadowMoves[i];
            std::cout << "getting leagal moves for " << Field::PiceChar( _board[i].Pice) << *_board[i].fieldname << std::endl << std::flush;
            GameRules::GetLegalMoves(_legalShadowMoves[i], _board, i, visionBoard);
            std::cout << "found " << _legalShadowMoves[i].size() << std::endl << std::flush;
        }
    }
}

void SlugChess::CalPossibleCastles(){
    _possibleCastles.clear();
    for(int i = 0; i < 64;i++){
        if(_board[i].Pice == ChessPice::WhiteKing){
            if(_board[i].FirstMove){
                for(int j = 0; j < 64;j++){
                    if(_board[j].Pice == ChessPice::WhiteRook && _board[j].FirstMove){
                        _possibleCastles.push_back(j);
                    }
                }
            }
        }
        if(_board[i].Pice == ChessPice::BlackKing){
            if(_board[i].FirstMove){
                for(int j = 0; j < 64;j++){
                    if(_board[j].Pice == ChessPice::BlackRook && _board[j].FirstMove){
                        _possibleCastles.push_back(j);
                    }
                }
            }
        }
    }
}

Field SlugChess::ExecuteMove(const std::string from, const std::string to){
    return ExecuteMove(GameRules::BoardPosToIndex(from), GameRules::BoardPosToIndex(to));
}

Field SlugChess::ExecuteMove(int from, int to){
    Field tofield = Field(_board[to].Pice, _board[to].fieldname);
    //int killedPos = -1;
    int modTo = to%8;
    int modFrom = from%8;
    //std::cout << "!!! Board modTo" << std::to_string(modTo) << "modFrom " << std::to_string(modFrom) << std::endl;
    //std::cout << "!!! Board from first move " << std::to_string(_board[from].FirstMove) << std::endl;
    //std::cout << "!!! Board from is king " << std::to_string(_board[from].Pice == ChessPice::WhiteKing || _board[from].Pice == ChessPice::BlackKing) << std::endl;
    //std::cout << "!!! Board mod diff 2 " << std::to_string(modTo == modFrom-2 || modTo == modFrom+2) << std::endl;
    if(_board[from].FirstMove && (_board[from].Pice == ChessPice::WhiteKing || _board[from].Pice == ChessPice::BlackKing) && (modTo > modFrom+1 || modTo < modFrom-1)){
        //std::cout << "!!!Dooing Castling" << std::endl;
        int kingEnd;
        int rookEnd;
        //ChessPice rook = _board[to].HasWhitePice()?ChessPice::WhiteRook:ChessPice::BlackRook;
        if(modTo > modFrom+1){
            kingEnd = GameRules::RightOne(GameRules::RightOne(from));
            rookEnd = GameRules::RightOne(from);
        }
        else if(modTo < modFrom-1)
        {
            kingEnd = GameRules::LeftOne(GameRules::LeftOne(from));
            rookEnd = GameRules::LeftOne(from);
        }
        _board[rookEnd] = Field(_board[to].Pice, _board[rookEnd].fieldname);
        _board[kingEnd] = Field(_board[from].Pice, _board[kingEnd].fieldname);
        _board[to] = Field(ChessPice::Non, _board[to].fieldname);
        _board[from] = Field(ChessPice::Non, _board[from].fieldname);
        tofield = Field(_board[to].Pice, _board[to].fieldname);
    }
    else if((_board[from].Pice == ChessPice::WhitePawn || _board[from].Pice == ChessPice::BlackPawn) && modFrom != modTo && _board[to].Pice == ChessPice::Non){
        //preform an passant
        auto moveFunc = _board[from].HasWhitePice()?GameRules::DownOne:GameRules::UpOne;
        int killedTo = moveFunc(to);
        tofield = Field(_board[killedTo].Pice, _board[killedTo].fieldname);
        _board[to] = Field(_board[from].Pice, _board[to].fieldname);
        _board[from] = Field(ChessPice::Non, _board[from].fieldname);
        _board[killedTo] = Field(ChessPice::Non, _board[killedTo].fieldname);
    }
    else
    {
        //std::cout << *_board[from].fieldname << " is from " << *_board[to].fieldname << " is to " << std::endl; 
        _board[to] = Field(_board[from].Pice, _board[to].fieldname);
        //std::cout << *_board[to].fieldname << " should now have " << Field::PiceChar(_board[from].Pice) << std::endl; 
        if(_board[to].Pice == ChessPice::WhitePawn && Field::IndexRow(to) == 7)_board[to].Pice = ChessPice::WhiteQueen;
        if(_board[to].Pice == ChessPice::BlackPawn && Field::IndexRow(to) == 0)_board[to].Pice = ChessPice::BlackQueen;
        if(_board[to].Pice == ChessPice::WhitePawn && GameRules::DownOne(GameRules::DownOne(to)) == from)_board[GameRules::DownOne(to)].AnPassan_able = true;
        if(_board[to].Pice == ChessPice::BlackPawn && GameRules::UpOne(GameRules::UpOne(to)) == from)_board[GameRules::UpOne(to)].AnPassan_able = true;
        _board[from] = Field(ChessPice::Non, _board[from].fieldname);
        //std::cout << Field::PiceChar(_board[to].Pice) << " is on row " << std::to_string(Field::IndexRow(to)) << std::endl; 
        //std::cout << *_board[to].fieldname << " is now " << Field::PiceChar(_board[to].Pice) << std::endl; 
    }
    
    _moves.push_back(std::tuple<int, int>(from, to));
    //Selected = null;
    return tofield;
}

void SlugChess::DoMove(const std::string& from, const std::string& to){
    //WriteLan(from,to);
    CleanAnPassants();
    WriteMoveSan(from, to);
    Field attackedField = ExecuteMove(from, to);
    _whiteTurn = !_whiteTurn;
    if(_whiteTurn) _turn++;
    _lastCaptured = attackedField.Pice;
    if(attackedField.Pice != ChessPice::Non){
        //std::cout << "Killed pice " << Field::PiceChar(attackedField.Pice) << " at " << *attackedField.fieldname << std::endl;
        _lastCaptureField = GameRules::BoardPosToIndex(*attackedField.fieldname);
        _killedPices.push_back(std::tuple<ChessPice,int>(attackedField.Pice, GameRules::BoardPosToIndex(*attackedField.fieldname)));
    }else{
        _lastCaptureField = -1;
    }
    if(_lastCaptured == ChessPice::WhiteKing){
        _gameEnd = EndResult::WhiteWin;
    }
    if(_lastCaptured == ChessPice::BlackKing){
        _gameEnd = EndResult::BlackWin;
    }
    CalculateVision();
    CalculateLegalMoves();
    CalculateLegalShadowMoves();
}

void SlugChess::WriteMoveSan(const std::string& fromStr, const std::string& toStr){
    int from = GameRules::BoardPosToIndex(fromStr);
    int to = GameRules::BoardPosToIndex(toStr);
    int modTo = to%8;
    int modFrom = from%8;
    if(_whiteTurn) _sanMoves << std::to_string(_turn) << ". ";
    //_sanMoves << " ";
    if(Field::IsPawn(_board[from]))
    {
        if(Field::IndexColumn(from) == Field::IndexColumn(to))
        {
            _sanMoves << GameRules::BoardPos(to);
        }else{
            _sanMoves << GameRules::BoardPos(from) << "x" << GameRules::BoardPos(to);
        }
        if((_whiteTurn && Field::IndexRow(to) == 7) ||(!_whiteTurn && Field::IndexRow(to) == 0)){
            _sanMoves << "=Q";
        }
    }
    else if(Field::IsKing(_board[from]) && _board[from].FirstMove && (modTo > modFrom+1 || modTo < modFrom-1))
    {
        if(modTo > modFrom+1){
            _sanMoves << "O-O";
        }
        else if(modTo < modFrom-1)
        {
            _sanMoves << "O-O-O";
        }
    }
    else
    {
        _sanMoves << Field::PiceCharCapital(_board[from].Pice);
        _sanMoves << GameRules::BoardPos(from) << (_board[to].Pice != ChessPice::Non?"x":"") << GameRules::BoardPos(to);
    }
    
    _sanMoves << " ";
}

void SlugChess::PrintSanMoves(std::stringstream& ss)
{
    ss << _sanMoves.str();
}