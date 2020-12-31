#pragma once

#include <unordered_map>
#include <unordered_set>
#include <stack>
#include "Board.h"

namespace edge {

namespace backtracker {

class Stack {
public:

    struct LevelInfo {
        Board::Loc* loc;
        std::unordered_map<Board::Loc*,
            std::unordered_set< PieceRef* > > forbidden;
        int score;

        LevelInfo(Board::Loc* loc) : loc(loc), score(0)
        {
        }
    };

    bool IsEmpty();

    Stack();

    std::stack<LevelInfo> visited;
    int start_size;

private:

};

}

}