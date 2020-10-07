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

bool SlugChess::visionBoardTrue[] = {
    true, true, true, true, true, true, true, true,
    true, true, true, true, true, true, true, true,
    true, true, true, true, true, true, true, true,
    true, true, true, true, true, true, true, true,
    true, true, true, true, true, true, true, true,
    true, true, true, true, true, true, true, true,
    true, true, true, true, true, true, true, true,
    true, true, true, true, true, true, true, true};

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
    _halfTurnSinceCaptureOrPawnMove = 0;
    Sfen::WriteSfenPicesToBoard(_board, sfenString);
    CalculateVision();
    CalculateLegalMoves();
    CalculateLegalShadowMoves();
    //CalPossibleCastles();
}


const std::string SlugChess::GetCurrentFenString(){
    // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w AHah - 0 1
    std::stringstream ss, resultSS;
    //Row is from bottom to top of board
    int row = 1, column = 1, emptyCount = 0;
    std::cout << "start" << std::endl;
    while (row < 9)
    {
        int i = column-1 + (row-1)*8;
        if(_board[i].Pice == ChessPice::Non){
            emptyCount += 1;
            if(column == 8){
                ss << std::to_string(emptyCount);
                emptyCount = 0;
            }
        }else{
            if(emptyCount > 0){
                ss << std::to_string(emptyCount);
                emptyCount = 0;
            }
            ss << Field::PiceChar(_board[i].Pice);
        }
        if(column == 8){
            if(row > 1) ss << '/';
            //std::cout << ss.str() << std::endl;
            ss << resultSS.str();
            resultSS = std::move(ss);
            ss.str("");
            column = 1;
            row += 1;
        }else{
            column += 1;
        }
    }
    resultSS << ' ' << (_whiteTurn?'w':'b') << ' ';
    for(int i : _possibleCastles){
        resultSS << Field::PiceChar(_board[i].Pice);
    }
    if(_possibleCastles.size() == 0) resultSS << '-';
    resultSS << ' ' << GetAnPassant() << ' ' << std::to_string(_halfTurnSinceCaptureOrPawnMove)
        << ' ' << std::to_string(_turn);
    
	return resultSS.str();
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
    PrintDebugBoard(ss, visionBoardTrue);
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
    std::string anPass = GetAnPassant();
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
            
            for(int i = 0; i < 64;i++){
                if(_board[i].HasWhitePice()){
                    visionMoves[i];
                    GameRules::GetLegalMoves(visionMoves[i], _board, i, visionBoardTrue);
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
                    GameRules::GetLegalMoves(visionMoves[i], _board, i, visionBoardTrue);
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
    _legalWhiteMoves.clear();
    _legalBlackMoves.clear();
    for(int i = 0; i < 64;i++){
        if(_board[i].HasWhitePice()){
            std::vector<int> movesI;
            GameRules::GetLegalMoves(movesI, _board, i, _whiteVision);
            if(movesI.size() > 0) _legalWhiteMoves[i] = movesI;
        }
    }
    for(int i = 0; i < 64;i++){
        if(_board[i].HasBlackPice()){
            std::vector<int> movesI;
            GameRules::GetLegalMoves(movesI, _board, i, _blackVision);
            if(movesI.size() > 0) _legalBlackMoves[i] = movesI;

        }
    }
    _legalMoves = _whiteTurn?_legalWhiteMoves:_legalBlackMoves;
}

void SlugChess::CalculateLegalShadowMoves(){
    _shadowWhiteMoves.clear();
    _shadowBlackMoves.clear();
    std::vector<Field> whiteBoard(_board);
    for(int i = 0; i < 64;i++){
        if(!_whiteVision[i]){
            whiteBoard[i] = Field();
        }
    }
    std::vector<Field> blackBoard(_board);
    for(int i = 0; i < 64;i++){
        if(!_blackVision[i]){
            blackBoard[i] = Field();
        }
    }
    for(int i = 0; i < 64;i++){
        if(whiteBoard[i].HasBlackPice()){
            _shadowBlackMoves[i];
            //std::cout << "getting leagal moves for " << Field::PiceChar( _board[i].Pice) << *_board[i].fieldname << std::endl << std::flush;
            GameRules::GetLegalMoves(_shadowBlackMoves[i], whiteBoard, i, visionBoardTrue);
            //std::cout << "found " << _legalShadowMoves[i].size() << std::endl << std::flush;
        }
    }
    for(int i = 0; i < 64;i++){
        if(blackBoard[i].HasWhitePice()){
            _shadowWhiteMoves[i];
            //std::cout << "getting leagal moves for " << Field::PiceChar( _board[i].Pice) << *_board[i].fieldname << std::endl << std::flush;
            GameRules::GetLegalMoves(_shadowWhiteMoves[i], blackBoard, i, visionBoardTrue);
            //std::cout << "found " << _legalShadowMoves[i].size() << std::endl << std::flush;
        }
    }
   
}

void SlugChess::FindChecks(){
    // _blackFieldsThatCheck.clear();
    // _whiteFieldsThatCheck.clear();
    // auto whiteMoves = _whiteTurn?_legalMovesWithFullVision:_legalShadowMovesWithFullVision;
    // auto blackMoves = _whiteTurn?_legalShadowMovesWithFullVision:_legalMovesWithFullVision;
    // int whiteKingIndex = 0;
    // for(; whiteKingIndex < 64; whiteKingIndex++){
    //     if(_board[whiteKingIndex].Pice == ChessPice::WhiteKing)break;
    // }
    // int blackKingIndex = 0;
    // for(; blackKingIndex < 64; blackKingIndex++){
    //     if(_board[blackKingIndex].Pice == ChessPice::BlackKing)break;
    // }
    // for (auto &&keyVal : whiteMoves)
    // {
    //     if(std::find(keyVal.second.begin(), keyVal.second.end(), blackKingIndex) != keyVal.second.end()){
    //         _whiteFieldsThatCheck.push_back(keyVal.first);
    //     }
    // }
    // for (auto &&keyVal : blackMoves)
    // {
    //     if(std::find(keyVal.second.begin(), keyVal.second.end(), whiteKingIndex) != keyVal.second.end()){
    //         _blackFieldsThatCheck.push_back(keyVal.first);
    //     }
    // }
    
}
std::set<int> SlugChess::Checks(Perspective perspective){
    std::set<int> set;
    if(perspective != Perspective::Both){
        bool whitePerspec = perspective == Perspective::White;
        //if(whitePerspec)std::cout << "!!!!white prespec check" << std::endl << std::flush;
        //if(!whitePerspec)std::cout << "!!!!black prespec check" << std::endl << std::flush;
        auto moves = whitePerspec?_legalWhiteMoves:_legalBlackMoves;
        auto shadowMoves = whitePerspec?_shadowBlackMoves:_shadowWhiteMoves;
        int kingPespective = GetFieldWithPice(whitePerspec?ChessPice::WhiteKing:ChessPice::BlackKing);
        int kingOther = GetFieldWithPice(whitePerspec?ChessPice::BlackKing:ChessPice::WhiteKing);
        for (auto &&keyVal : moves)
        {
            auto kingIter = std::find(keyVal.second.begin(), keyVal.second.end(), kingOther);
            if(kingIter != keyVal.second.end()){
                set.insert(keyVal.first);
                set.insert(*kingIter);
            }
        }
        for (auto &&keyVal : shadowMoves)
        {
            auto kingIter = std::find(keyVal.second.begin(), keyVal.second.end(), kingPespective);
            if(kingIter != keyVal.second.end()){
                set.insert(keyVal.first);
                set.insert(*kingIter);
            }
        }
        
    }
    return set;
}

int SlugChess::GetFieldWithPice(ChessPice pice){
    for(int i = 0; i < 64;i++){
        if(_board[i].Pice == pice){
            return i;
        }
    }
    return -1;
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

void SlugChess::CleanAnPassants()
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
    
    _moves.push_back(std::pair<int, int>(from, to));
    //Selected = null;
    return tofield;
}

bool SlugChess::LegalMove(const std::string& from, const std::string& to)
{
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
        _killedPices.push_back(std::pair<ChessPice,int>(attackedField.Pice, GameRules::BoardPosToIndex(*attackedField.fieldname)));
        _halfTurnSinceCaptureOrPawnMove = 0;
    }else{
        _lastCaptureField = -1;
        _halfTurnSinceCaptureOrPawnMove++;
    }
    if(_lastCaptured == ChessPice::WhiteKing){
        _gameEnd = EndResult::BlackWin;
    }
    if(_lastCaptured == ChessPice::BlackKing){
        _gameEnd = EndResult::WhiteWin;
    }
    if(Field::IsPawn(_board[BPToIndx(to)])) _halfTurnSinceCaptureOrPawnMove = 0;
    CalculateVision();
    CalculateLegalMoves();
    CalculateLegalShadowMoves();
    FindChecks();
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
    ss << _sanMoves.str() << " " << ResultString();
}

std::string SlugChess::From(Perspective perspective)
{
    if(_moves.empty()) return "";
    int from = _moves.back().first;
    auto vis = VisionBoardPerspective(perspective);
    return vis[from]?BP(from):"";
}
std::string SlugChess::To(Perspective perspective)
{
    if(_moves.empty()) return "";
    int to = _moves.back().second;
    auto vis = VisionBoardPerspective(perspective);
    return vis[to]?BP(to):"";
}

bool* SlugChess::VisionBoardPerspective(Perspective perspective)
{
    switch (perspective)
    {
    case Perspective::Both:
        return visionBoardTrue;
    case Perspective::White:
        return _whiteVision;
    case Perspective::Black:
        return _blackVision;

    }
    return nullptr;
}