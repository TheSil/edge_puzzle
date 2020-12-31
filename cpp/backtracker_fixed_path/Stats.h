#pragma once

#include "Board.h"
#include "MpfWrapper.h"

namespace edge {

namespace backtracker {

class Stats {
public:

    void Init(Board& board);

    void Update(int stack_pos);

    MpfWrapper GetExploredAbs();

    MpfWrapper GetExploredAbsLast();

    MpfWrapper GetExploredMax();

    MpfWrapper GetExploredRatio();

    void UpdateUnplacedCorners(int amount);

    void UpdateUnplacedEdges(int amount);

    void UpdateUnplacedInner(int amount);

private:
    std::vector<mpz_ptr> factorial;
    std::vector<mpz_ptr> explored;
    mpz_t explored_max;
    mpz_t absLast;
    int unplaced_corners_ids_count;
    int unplaced_edges_ids_count;
    int unplaced_inner_ids_count;

};

}

}