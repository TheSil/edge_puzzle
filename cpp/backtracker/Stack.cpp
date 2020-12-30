#include "Stack.h"

using namespace edge::backtracker;

bool Stack::IsEmpty() {
    // empty == only root, plus possible hints
    return visited.size() == start_size;
}

Stack::Stack() : backtrack_to(0), start_size(1)
{
    visited.push(LevelInfo(nullptr)); // root
}
