#pragma once
#include <map>
#include <vector>
#include <set>
#include <algorithm>

#include <google/protobuf/util/time_util.h>

#include "../chesscom/chesscom.grpc.pb.h"
#include "../../SlugChessCore/src/slugchess.h"
#include "common.h"
#include "match.h" 

class SlugChessConverter
{
    public:
    static void SetGameState(std::shared_ptr<SlugChess> game, chesscom::GameState* gameState, PlayerTypes playerType){
        //std::cout  << " set game stat " << std::endl << std::flush;
        //auto vision = isWhitePlayer?game->GetWhiteVision():game->GetBlackVision(); 
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
        auto vision_state = gameState->mutable_vision_state();
        switch (playerType)
        {
        case PlayerTypes::White:
            {
                auto wv = game->GetWhiteVision();
                *vision_state->mutable_white_vision() = {wv.begin(), wv.end()};
                CopyToMap(gameState->mutable_player_moves(), game->LegalWhiteMovesRef());
                CopyToMap(gameState->mutable_shadow_moves(), game->ShadowWhiteMovesRef());
            }
            break;
        case PlayerTypes::Black:
            {
                auto bv = game->GetBlackVision();
                *vision_state->mutable_black_vision() = {bv.begin(), bv.end()};
                CopyToMap(gameState->mutable_player_moves(), game->LegalBlackMovesRef());
                CopyToMap(gameState->mutable_shadow_moves(), game->ShadowBlackMovesRef());
            }
            break;
        case PlayerTypes::Observer:
            {
                auto wv = game->GetWhiteVision();
                auto bv = game->GetBlackVision();
                *vision_state->mutable_white_vision() = {wv.begin(), wv.end()};
                *vision_state->mutable_black_vision() = {bv.begin(), bv.end()};
                CopyToMap(gameState->mutable_player_moves(), game->WhitesTurn()?game->LegalWhiteMovesRef():game->LegalBlackMovesRef());
                CopyToMap(gameState->mutable_shadow_moves(), game->WhitesTurn()?game->ShadowBlackMovesRef():game->ShadowWhiteMovesRef());
            }
            break;
        }
        //*gameState->mutable_player_vision() = {vision.begin(), vision.end()};
        *gameState->mutable_pices() = {pices.begin(), pices.end()};
        *gameState->mutable_captured_pices() = {capturedTrans.begin(), capturedTrans.end()};
        //gameState->set_captured_pice((chesscom::Pices)game->LastCaptured());
        gameState->set_from(game->From(static_cast<SlugChess::Perspective>(playerType)));
        gameState->set_to(game->To(static_cast<SlugChess::Perspective>(playerType)));
        gameState->set_current_turn_is_white(game->WhitesTurn());
        //MOves
        gameState->mutable_player_moves()->clear();
        gameState->mutable_shadow_moves()->clear();


        auto check = gameState->mutable_check();
        check->Clear();
        for (auto &&index : game->Checks(static_cast<SlugChess::Perspective>(playerType)))
        {
            std::cout  << "playerType:" << std::to_string(playerType) << " adding check  " << SlugChess::BP(index) << std::endl << std::flush;
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