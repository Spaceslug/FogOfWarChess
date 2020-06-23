#pragma once
#include <map>
#include <vector>
#include <set>
#include <algorithm>

#include <google/protobuf/util/time_util.h>

#include "../chesscom/chesscom.grpc.pb.h"
#include "../../SlugChessCore/src/slugchess.h"

class SlugChessConverter
{
    public:
    static void SetGameState(std::shared_ptr<SlugChess> game, chesscom::GameState* gameState, bool isWhitePlayer){
        //std::cout  << " set game stat " << std::endl << std::flush;
        auto vision = isWhitePlayer?game->GetWhiteVision():game->GetBlackVision(); 
        auto pices = game->GetPices(); 
        auto captured = game->KilledPices();
        std::vector<chesscom::PiceCapture> capturedTrans{captured->size()}; 
        std::transform (captured->begin(), captured->end(), capturedTrans.begin(), [](std::pair<ChessPice,int> cp)
        {
            chesscom::PiceCapture newCP;
            newCP.set_pice((chesscom::Pices)cp.first);
            newCP.set_location(SlugChess::BP(cp.second));
            return newCP;
        });

        *gameState->mutable_player_vision() = {vision.begin(), vision.end()};
        *gameState->mutable_pices() = {pices.begin(), pices.end()};
        *gameState->mutable_captured_pices() = {capturedTrans.begin(), capturedTrans.end()};
        //gameState->set_captured_pice((chesscom::Pices)game->LastCaptured());
        gameState->set_from(game->From(isWhitePlayer?SlugChess::Perspective::White:SlugChess::Perspective::Black));
        gameState->set_to(game->To(isWhitePlayer?SlugChess::Perspective::White:SlugChess::Perspective::Black));
        gameState->set_current_turn_is_white(game->WhitesTurn());
        //MOves
        gameState->mutable_player_moves()->clear();
        gameState->mutable_shadow_moves()->clear();
        CopyToMap(gameState->mutable_player_moves(), isWhitePlayer?game->LegalWhiteMovesRef():game->LegalBlackMovesRef());
        CopyToMap(gameState->mutable_shadow_moves(), isWhitePlayer?game->ShadowWhiteMovesRef():game->ShadowBlackMovesRef());

        auto check = gameState->mutable_check();
        check->Clear();
        for (auto &&index : game->Checks(isWhitePlayer?SlugChess::Perspective::White:SlugChess::Perspective::Black))
        {
            std::cout  << std::to_string(isWhitePlayer) << " adding check  " << SlugChess::BP(index) << std::endl << std::flush;
            check->Add(SlugChess::BP(index)); 
        }
    }
private:
    static void CopyToMap(google::protobuf::Map<std::string,chesscom::FieldMoves> *to, 
                            std::map<int, std::vector<int>> *from)
    {
        chesscom::FieldMoves fm;
        auto fmRF = fm.mutable_list();
        for (auto &&keyVal : *from)
        {
            //std::cout << keyVal.first << " < ";
            for (auto &&pos : keyVal.second)
            {
                //std::cout << SlugChess::BP(pos) << " - ";
                fmRF->Add(SlugChess::BP(pos));
            }
            //std::cout << std::endl;
            (*to)[SlugChess::BP(keyVal.first)].CopyFrom(fm);    
            fmRF->Clear();
        }
    }
};