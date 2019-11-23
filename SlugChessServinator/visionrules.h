#pragma once
#include <map>
#include "field.h"

struct Rules{
    bool viewMoveFields;
    bool viewCaptureField;
    int viewRange;
};

struct VisionRules{
    bool enabled;
    Rules globalRules;
    std::map<ChessPice, Rules> overWriteRules;
};