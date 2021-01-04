#include "Stack.h"

using namespace edge::backtracker;

bool Stack::IsEmpty() {
    // empty == only root, plus possible hints
    return visited.size() == start_size;
}

Stack::Stack() : start_size(1)
{
    visited.push(LevelInfo(0)); // root
}
