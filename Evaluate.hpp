#pragma once

#include "Defs.h"
#include "Board.hpp"

// Value constants
#define SCORE_MATE 30000
#define SCORE_INFINITY 32000

class Evaluation {
public:
    static int Evaluate(const Board* pos);
};
