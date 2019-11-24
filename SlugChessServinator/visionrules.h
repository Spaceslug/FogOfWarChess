#pragma once
#include <map>
#include "field.h"

struct Rules{
    bool ViewMoveFields;
    bool ViewCaptureField;
    int ViewRange;
    Rules(bool viewMoveFields, bool viewCaptureField,int viewRange){
        ViewMoveFields = viewMoveFields;
        ViewCaptureField = viewCaptureField;
        ViewRange = viewRange;
    }

    Rules(int range) : Rules(false, false, range){
        
    }

    Rules() : Rules(false, true, 2){
        
    }
};

struct VisionRules{
    bool enabled;
    Rules globalRules;
    std::map<ChessPice, Rules> overWriteRules;
};