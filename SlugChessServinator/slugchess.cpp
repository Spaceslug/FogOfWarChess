#include "slugchess.h"
#include <sstream>
#include <iterator>




SlugChess::SlugChess(const std::string& sfenString, const VisionRules& visionRules){
    // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w AHah - 0 1
    // normal startingpoint shredder-fen
    _board = std::vector<Field>(64);
    _rules = visionRules;
    _killedPices.clear();
    Sfen::WriteSfenPicesToBoard(_board, sfenString);
    CalculateVision();
    CalculateLegalMoves();
    //CalPossibleCastles();
}


const std::string SlugChess::ToFenString(){

}

void SlugChess::PrintBoard(std::stringstream& ss, bool whiteplayer){
    PrintBoard(ss, whiteplayer?_whiteVision:_blackVision);
}

void SlugChess::PrintBoard(std::stringstream& ss, bool visionboard[]){
    for(int row = 7; row >= 0;row--){
        for(int col = 0; col <= 7;col++){
            int index = GameRules::IndexFromColRow(col, row);
            if(!visionboard[index]) continue;
            ss << "[ " << Field::PiceChar(_board[index].Pice) << " ]";
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
    return ExecuteMove(GameRules::BoardPosToIndex(from), GameRules::BoardPosToIndex(from));
}

Field SlugChess::ExecuteMove(int from, int to){
    Field tofield = _board[to];
    int killedPos = -1;
    int modTo = to%8;
    int modFrom = from%8;
    if(_board[from].FirstMove && (_board[from].Pice == ChessPice::WhiteKing || _board[from].Pice == ChessPice::BlackKing) && (modTo == modFrom-2 || modTo == modFrom+2)){
        //test casteling
        int newto;
        int otherto;
        ChessPice rook = _board[to].HasWhitePice()?ChessPice::WhiteRook:ChessPice::BlackRook;
        if(modTo == modFrom+2){
            newto = GameRules::RightOne(to);
            while(_board[newto].Pice != rook){
                newto = GameRules::RightOne(newto);
            }
            otherto = GameRules::LeftOne(to);
        }
        else if(modTo == modFrom-2)
        {
            newto = GameRules::LeftOne(to);
            while(_board[newto].Pice != rook){
                newto = GameRules::LeftOne(newto);
            }
            otherto = GameRules::RightOne(to);
        }
        _board[otherto] = Field(_board[newto].Pice, _board[otherto].fieldname);
        _board[newto] = Field(ChessPice::Non, _board[newto].fieldname);
        _board[to] = Field(_board[from].Pice, _board[to].fieldname);
        _board[from] = Field(ChessPice::Non, _board[from].fieldname);

    }
    else if(_board[to].AnPassan_able){
        //preform an passant
        auto moveFunc = _board[from].HasWhitePice()?GameRules::UpOne:GameRules::DownOne;
        int killedTo = moveFunc(to);
        tofield = _board[killedTo];
        _board[to] = Field(_board[from].Pice, _board[to].fieldname);
        _board[from] = Field(ChessPice::Non, _board[from].fieldname);
    }
    else
    {
        _board[to] = Field(_board[from].Pice, _board[to].fieldname);
        if(_board[to].Pice == ChessPice::WhitePawn && Field::IndexRow(to) == 7)_board[to].Pice == ChessPice::WhiteQueen;
        if(_board[to].Pice == ChessPice::BlackPawn && Field::IndexRow(to) == 0)_board[to].Pice == ChessPice::BlackQueen;
        if(_board[to].Pice == ChessPice::WhitePawn && GameRules::DownOne(GameRules::DownOne(to)) == from)_board[GameRules::DownOne(to)].AnPassan_able = true;
        if(_board[to].Pice == ChessPice::BlackPawn && GameRules::UpOne(GameRules::UpOne(to)) == from)_board[GameRules::UpOne(to)].AnPassan_able = true;
        _board[from] = Field(ChessPice::Non, _board[from].fieldname);
    }
    
   
    //Selected = null;
    return tofield;
}

void SlugChess::DoMove(const std::string& from, const std::string& to){
    
    Field attackedField = ExecuteMove(from, to);
    _whiteTurn = !_whiteTurn;
    CalculateVision();
    CalculateLegalMoves();
    _lastCaptured = attackedField.Pice;
    if(attackedField.Pice != ChessPice::Non){
        _lastCaptureField = GameRules::BoardPosToIndex(*attackedField.fieldname);
        _killedPices.push_back(std::tuple<ChessPice,int>(attackedField.Pice, GameRules::BoardPosToIndex(*attackedField.fieldname)));
    }
}
