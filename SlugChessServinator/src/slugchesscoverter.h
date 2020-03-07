#pragma once
#include <map>
#include <vector>
#include <set>

#include <google/protobuf/util/time_util.h>

#include "../chesscom/chesscom.grpc.pb.h"
#include "../../SlugChessCore/src/slugchess.h"

class SlugChessConverter
{
    public:
    static void SetMove(std::shared_ptr<SlugChess> game, std::shared_ptr<chesscom::Move> move, bool isWhitePlayer){
        auto ww = game->GetWhiteVision(); 
        auto bw = game->GetBlackVision(); 
        auto pices = game->GetPices(); 
        *move->mutable_whitevision() = {ww.begin(), ww.end()};
        *move->mutable_blackvision() = {bw.begin(), bw.end()};
        *move->mutable_pices() = {pices.begin(), pices.end()};
        move->set_capturedpice((chesscom::Pices)game->LastCaptured());
        //MOves
        CopyToMap(move->mutable_availablemoves(), game->LegalMovesRef());
        //White MOves
        CopyToMap(move->mutable_whitemoves(), game->LegalWhiteMovesRef());
        //Black MOves
        CopyToMap(move->mutable_blackmoves(), game->LegalBlackMovesRef());
        //White Shadow MOves
        CopyToMap(move->mutable_whiteshadowmoves(), game->ShadowWhiteMovesRef());
        //Black Shadow MOves
        CopyToMap(move->mutable_blackshadowmoves(), game->ShadowBlackMovesRef());

        auto check = move->mutable_check();
        for (auto &&index : game->Checks(isWhitePlayer?SlugChess::Perspective::White:SlugChess::Perspective::Black))
        {
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