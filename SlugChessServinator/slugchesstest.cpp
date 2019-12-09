#include <sstream>

#include "slugchess.h"

int main(int argc, char** argv) {
    void (*prev_handler)(int);

    std::cout << "Slugchess boardpos b3 " << std::to_string(GameRules::BoardPosToIndex("b3")) << "" << std::flush << std::endl;
    //std::cout << "Slugchess boardpos b3 " << std::to_string(SlugChess::BPToIndx("b3")) << "" << std::flush << std::endl;
    std::cout << "Slugchess  3 right one" << std::to_string(GameRules::RightOne(3)) << "" << std::flush << std::endl;
    std::stringstream ss;
    VisionRules rules;
    rules.globalRules = Rules();
    rules.globalRules.ViewCaptureField = true;
    rules.globalRules.ViewMoveFields = true;
    rules.globalRules.ViewRange = 2;
    rules.enabled = true;
    rules.overWriteRules[ChessPice::WhitePawn] = Rules(false,true, 1);
    rules.overWriteRules[ChessPice::BlackPawn] = Rules(false,true, 1);
    //SlugChess slugChess("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w AHah - 0 1", rules);
    SlugChess slugChess("rnbqkbnr/1ppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR w AHah - 0 1", rules);
    slugChess.PrintBoard(ss);
    slugChess.PrintWhiteVision(ss);
    ss << std::endl;
    slugChess.PrintBlackVision(ss);
    std::cout << ss.str();
    int postest = 54;
    std::cout << "Pos " << std::to_string(postest) << ": row " << std::to_string(Field::IndexRow(postest)) << " col " << std::to_string(Field::IndexColumn(postest));
    std::cout << " colrow " << std::to_string(GameRules::IndexFromColRow(Field::IndexColumn(postest), (Field::IndexRow(postest)))) << std::endl;   
    return 0;
}
