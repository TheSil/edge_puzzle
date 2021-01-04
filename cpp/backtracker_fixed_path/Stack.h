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
        std::vector< std::array<bool, 4> > forbidden;

        LevelInfo(int pieces_count)
        {
            forbidden.resize(pieces_count + 1);
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