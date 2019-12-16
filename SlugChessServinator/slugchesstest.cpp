#include <sstream>

#include "slugchess.h"

SlugChess* game;

void GameLoop(){
    std::string s;
    std::cout << "Enter commands to play" << std::endl;
    while(s != "exit"){
        std::cin >> s;
        std::cout << "-" << s << std::endl;
        if(s == "play"){
            if(game != nullptr) delete game;   
            VisionRules rules;
            rules.globalRules = Rules();
            rules.globalRules.ViewCaptureField = true;
            rules.globalRules.ViewMoveFields = false;
            rules.globalRules.ViewRange = 2;
            rules.enabled = true;
            rules.overWriteRules[ChessPice::WhitePawn] = Rules(false,true, 1);
            rules.overWriteRules[ChessPice::BlackPawn] = Rules(false,true, 1);     
            game = new SlugChess("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w AHah - 0 1", rules);
            std::stringstream ss;
            game->PrintBoard(ss, game->WhitesTurn());        
            std::cout << ss.str() << std::endl;
        }else if(s[0] == '-' && s.size() == 5){
            
            std::string from = s.substr(1, 2);
            std::string to = s.substr(3, 2);
            std::cout << "Move " << from << "-" << to << std::endl;
            if(game->LegalMove(from, to)){
                game->DoMove(from, to);    
                std::stringstream ss;
                game->PrintBoard(ss, game->WhitesTurn());        
                std::cout << ss.str() << std::endl;
            }else{
                std::cout << "Not a legal move!  " << from << "-" << to << std::endl;
            }
            

        }else if(s == "moves"){
            auto moves = *game->LegalMovesRef();
            for (auto &&keyVal : moves)
            {
                std::string from = game->BP(keyVal.first);
                std::cout << "(" << from << "):" << std::flush;
                for (auto &&toPos : keyVal.second)
                {
                    std::cout << "(" << from << "-" << game->BP(toPos) << ")";
                }
                
            }
            std::cout << std::endl;
            
        }else if(s == "board"){
            std::stringstream ss;
            std::cout << "Printing board" << std::endl;
            bool whiteplayer = game->WhitesTurn();
            std::cout << "white player " << std::to_string(whiteplayer) << std::endl;
            game->PrintBoard(ss, whiteplayer);        
            std::cout << ss.str() << std::endl;
        }
    }
    if(game != nullptr) delete game;

}

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
    slugChess.PrintDebugBoard(ss);
    slugChess.PrintWhiteVision(ss);
    ss << std::endl;
    slugChess.PrintBlackVision(ss);
    std::cout << ss.str();
    int postest = 54;
    std::cout << "Pos " << std::to_string(postest) << ": row " << std::to_string(Field::IndexRow(postest)) << " col " << std::to_string(Field::IndexColumn(postest));
    std::cout << " colrow " << std::to_string(GameRules::IndexFromColRow(Field::IndexColumn(postest), (Field::IndexRow(postest)))) << std::endl;   
    GameLoop();
    return 0;
}
