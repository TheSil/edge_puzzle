#pragma once

#include <unordered_map>
#include <stack>
#include "Board.h"

namespace edge {

namespace backtracker {

class Stack {
public:

    struct LevelInfo {
        Board::Loc* loc;
        std::unordered_map<Board::Loc*,
            std::set< std::shared_ptr<PieceRef> > > forbidden;
        int score;

        LevelInfo(Board::Loc* loc) : loc(loc), score(0)
        {
        }
    };

    bool IsEmpty();

    Stack();

    std::stack<LevelInfo> visited;
    int backtrack_to;
    int start_size;

private:

};

}

}