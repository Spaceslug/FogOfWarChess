#pragma once
#include <map>
#include "field.h"

struct SHARED_EXPORT Rules{
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

    Rules() : Rules(false, false, 0){
        
    }
};

struct SHARED_EXPORT VisionRules{
    bool enabled;
    Rules globalRules;
    std::map<ChessPice, Rules> overWriteRules;
};
