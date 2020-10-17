#include <sstream>

#include "slugchess.h"

#ifdef WIN
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#endif

SlugChess* game;
std::vector<std::string> sanMoves;
int sanTicker = 0;

void GameLoop(){
    std::string s;
    std::cout << "Enter commands to play" << std::endl;
    while(s != "exit"){
        std::cin >> s;
        std::cout << "-" << s << std::endl;
        if(s == "exit"){
            continue;
        }else if(s == "play"){
            if(game != nullptr) delete game;   
            VisionRules rules;
            rules.globalRules = Rules();
            rules.globalRules.ViewCaptureField = true;
            rules.globalRules.ViewMoveFields = false;
            rules.globalRules.ViewRange = 2;
            rules.enabled = true;
            rules.overWriteRules[ChessPice::WhitePawn] = Rules(false,true, 1);
            rules.overWriteRules[ChessPice::BlackPawn] = Rules(false,true, 1);     
            //game = new SlugChess("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w AHah - 0 1", rules);
            game = new SlugChess("rnbqkbnr/pppp1ppp/4R3/8/4p3/8/PPPPPPPP/1NBQKBNR w AHah - 0 1", rules);
            std::stringstream ss;
            game->PrintBoard(ss, game->WhitesTurn());        
            std::cout << ss.str() << std::endl;
        }else if(s == "playr"){
            if(game != nullptr) delete game;   
            VisionRules rules;
            rules.globalRules = Rules();
            rules.globalRules.ViewCaptureField = true;
            rules.globalRules.ViewMoveFields = false;
            rules.globalRules.ViewRange = 2;
            rules.enabled = true;
            rules.overWriteRules[ChessPice::WhitePawn] = Rules(false,true, 1);
            rules.overWriteRules[ChessPice::BlackPawn] = Rules(false,true, 1);     
            game = new SlugChess(Sfen::GenSlugRandom(), rules);
            std::stringstream ss;
            game->PrintBoard(ss, game->WhitesTurn());        
            std::cout << ss.str() << std::endl;
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
        }else if(s == "san"){
            std::stringstream ss;
            std::cout << "Printing moves" << std::endl;

            game->PrintSanMoves(ss);        
            std::cout << ss.str() << std::endl;
        }else if(s == "fen"){
            std::stringstream ss;
            std::cout << "Printing fen" << std::endl;

            game->PrintSanMoves(ss);        
            std::cout << game->GetCurrentFenString() << std::endl;
        }else if(s == "testsan"){
            std::cout << "Testing SAN" << std::endl;
            if(game != nullptr) delete game;   
            VisionRules rules;
            rules.globalRules = Rules(); //Torch
            rules.globalRules.ViewCaptureField = true;
            rules.globalRules.ViewMoveFields = true;
            rules.globalRules.ViewRange = 0;
            rules.enabled = true;
            rules.overWriteRules[ChessPice::WhitePawn] = Rules(true,true, 1);
            rules.overWriteRules[ChessPice::BlackPawn] = Rules(true,true, 1);   
            rules.overWriteRules[ChessPice::WhiteKnight] = Rules(true,true, 1);
            rules.overWriteRules[ChessPice::BlackKnight] = Rules(true,true, 1);  
            game = new SlugChess("knrqnbbr/pppppppp/8/8/8/8/PPPPPPPP/RQRNNBKB w - - 0 1", rules);
            std::stringstream ss;
            game->PrintBoard(ss, game->WhitesTurn());        
            sanMoves = San::SanMovesFromSan("1. e3 Nb8c6 2. c4 e5 3. g3 f6 4. Qb1e4 h6 5. Ne1d3 Bg8h7 6. Qe4f3 b6 7. Qf3e4 Qd8e7 8. Nd3c5 Bh7xe4 9. Bh1xe4 Qe7xc5 10. Be4xc6 d7xc6 11. e4 Qc5xf2 12. Kg1xf2 c5 13. b4 c5xb4 14. a3 b4xa3 15. Ra1xa3 c5 16. d3 Bf8d6 17. d4 e5xd4 18. Ra3d3 f5 19. e4xf5 g6 20. f5xg6 Rh8g8 21. Rd3xd4 Rg8xg6 22. Rd4xd6 Ne8xd6 23. Nd1e3 Rg6xg3 24. Kf2xg3 h5 25. Ne3f5 Nd6xf5 26. Bf1d3 Rc8f8 27. Bd3xf5 Rf8e8 28. Rc1b1 b5 29. c4xb5 a6 30. Bf5c8 Re8xc8 31. b5xa6 Rc8g8 32. Kg3h4 Rg8g4 33. Rb1b8 Rg4xh4  0-1");
            for( auto&& move : sanMoves){ //apparently the order is flipped?
                std::cout << "Move: " << move << std::endl;
            }
            std::cout << ss.str() << std::endl;
        }else if(s == "movsan"){
            for(int i = 0; i < sanMoves.size(); i++)
            if(game->DoSanMove(sanMoves[i])){
            //    if(game->DoSanMove("sanMoves[sanTicker++]")){
                std::stringstream ss;
                std::cout << "Printing board" << std::endl;
                bool whiteplayer = game->WhitesTurn();
                std::cout << "white player " << std::to_string(whiteplayer) << std::endl;
                game->PrintBoard(ss, whiteplayer);        
                //std::cout << ss.str() << std::endl;
            }
            else{
                std::cout << "ERROR: failed dooing san move " << sanMoves[i] << std::endl;
            }
        }else if(s == "check"){ 
            std::stringstream ss;
            //std::cout << "Printing moves" << std::endl;
            ss << "White checks: ";
            // for (auto &&fieldIndex : *game->WhiteChecksRef())
            // {
            //     ss << game->BP(fieldIndex) << " ";
            // }
            // ss << std::endl;

            // ss << "Black checks: ";
            // for (auto &&fieldIndex : *game->BlackChecksRef())
            // {
            //     ss << game->BP(fieldIndex) << " ";
            // }
            std::cout << ss.str() << std::endl; 
        }else if( s.size() == 4){
            
            std::string from = s.substr(0, 2);
            std::string to = s.substr(2, 2);
            std::cout << "Move " << from << "-" << to << std::endl;
            if(game->LegalMove(from, to)){
                game->DoMove(from, to);

                std::stringstream ss;
                switch (game->Result())
                {
                case SlugChess::EndResult::Draw:
                    ss << "Game ended: Draw " << std::endl;
                    game->PrintDebugBoard(ss);
                    ss << "Game ended: Draw " << std::endl;
                    delete game;
                    break;
                case SlugChess::EndResult::WhiteWin:
                    ss << "Game ended: White won!" << std::endl;
                    game->PrintDebugBoard(ss);
                    ss << "Game ended: White won!" << std::endl;
                    delete game;
                    break;
                case SlugChess::EndResult::BlackWin:
                    ss << "Game ended: Black won!" << std::endl;
                    game->PrintDebugBoard(ss);
                    ss << "Game ended: Black won!" << std::endl;
                    delete game;
                    break;
                case SlugChess::EndResult::StillPlaying:
                    game->PrintBoard(ss, !game->WhitesTurn());
                    ss << std::endl;
                    game->PrintBoard(ss, game->WhitesTurn());
                    break;
                default:
                    break;
                }    
                std::cout << ss.str() << std::endl;
            }else{
                std::cout << "Not a legal move!  " << from << "-" << to << std::endl;
            }
            

        }
    }
    if(game != nullptr) delete game;

}

int main(int argc, char** argv) {

    //std::cout << "Slugchess boardpos b3 " << std::to_string(GameRules::BoardPosToIndex("b3")) << "" << std::flush << std::endl;
    //std::cout << "Slugchess boardpos b3 " << std::to_string(SlugChess::BPToIndx("b3")) << "" << std::flush << std::endl;
    // std::cout << "Slugchess  3 right one" << std::to_string(GameRules::RightOne(3)) << "" << std::flush << std::endl;
    // std::stringstream ss;
    // VisionRules rules;
    // rules.globalRules = Rules();
    // rules.globalRules.ViewCaptureField = true;
    // rules.globalRules.ViewMoveFields = true;
    // rules.globalRules.ViewRange = 2;
    // rules.enabled = true;
    // rules.overWriteRules[ChessPice::WhitePawn] = Rules(false,true, 1);
    // rules.overWriteRules[ChessPice::BlackPawn] = Rules(false,true, 1);
    //SlugChess slugChess("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w AHah - 0 1", rules);
    // SlugChess slugChess("rnbqkbnr/1ppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR w AHah - 0 1", rules);
    // slugChess.PrintDebugBoard(ss);
    // slugChess.PrintWhiteVision(ss);
    // ss << std::endl;
    // slugChess.PrintBlackVision(ss);
    // std::cout << ss.str();
    //int postest = 54;
    //std::cout << "Pos " << std::to_string(postest) << ": row " << std::to_string(Field::IndexRow(postest)) << " col " << std::to_string(Field::IndexColumn(postest));
    //std::cout << " colrow " << std::to_string(GameRules::IndexFromColRow(Field::IndexColumn(postest), (Field::IndexRow(postest)))) << std::endl;   
    for(int i = 0; i < 5; i++)
    std::cout << "sFen Double Fuisher Random: " << Sfen::GenSlugRandom() << std::endl;
    GameLoop();
    return 0;
}
