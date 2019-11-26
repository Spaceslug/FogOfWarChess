#include "slugchess.h"
#include <sstream>
#include <iterator>




SlugChess::SlugChess(const std::string& sfenString, const VisionRules& visionRules){
    // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w AHah - 0 1
    // normal startingpoint shredder-fen
    _board = std::vector<Field>(64);
    _rules = visionRules;
    Sfen::WriteSfenPicesToBoard(_board, sfenString);
    CalculateVision();
    CalculateLegalMoves();
}

int32_t SlugChess::GetLegalMove(std::string s){
    return stoi(s);
}

const std::string SlugChess::ToFenString(){

}

void SlugChess::PrintBoard(std::stringstream& ss){
    for(int row = 7; row >= 0;row--){
        for(int col = 0; col <= 7;col++){
            int index = GameRules::IndexFromColRow(col, row);
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

// void SlugChess::CalculateLegalMoves(std::vector<Field>& board, bool visionBoard[]){
//     for(int i = 0; i < 64;i++){
//         if(board[i].Pice != ChessPice::Non){
//             _legalMoves[i];
//             GameRules::GetLegalMoves(_legalMoves[i], _board, i, visionBoard);
//         }
//     }
// }