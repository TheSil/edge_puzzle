#pragma once

#include <set>
#include <vector>
#include <stack>
#include "Board.h"

namespace edge {

class Backtracker {
public:
    Backtracker(Board& board);

    bool Step();

    void CheckFeasible(bool ignore_impossible = false);

    bool CanBePlacedAt(Board::BoardLoc* loc, std::shared_ptr<PieceRef> ref);

    void Place(Board::BoardLoc* loc, std::shared_ptr<PieceRef> ref);

    void Backtrack();

    // TBD - needs more works, arbitrary precision computations...
    int Power(int base, int exponent);

private:
    enum class State {
        SEARCHING = 0,
        BACKTRACKING = 1,
        FINALIZING = 2,
        SOLVED = 3
    };

    Board& board;
    State state;

    int best_score;
    int backtrack_to;
    int counter;
    int finalizing_threshold;
    std::stack<Board::BoardLoc*> visited;
    std::set<Board::BoardLoc*> unvisited;
    std::set<int> placed_ids;
    Board::BoardLoc* backtracked_position;
    bool find_all;
    bool enable_finalizing;
    bool connecting;
    bool constraint_reducing;

    std::set < std::shared_ptr<PieceRef> > feasible_possibilities;
    std::vector<Board::BoardLoc*> best_feasible_locations;
    std::vector<std::vector<
        std::unique_ptr< std::set<
        std::shared_ptr<PieceRef> > > > > feasible_pieces;

    std::vector<int> grid_scores;

    std::set< std::shared_ptr<PieceRef> >* best_unplaced_container;
    std::set<int>* best_unplaced_container_ids;

    std::set<int> unplaced_corners_ids;
    std::set<int> unplaced_edges_ids;
    std::set<int> unplaced_inner_ids;
    std::set< std::shared_ptr<PieceRef> > unplaced_corners;
    std::set< std::shared_ptr<PieceRef> > unplaced_edges;
    std::set< std::shared_ptr<PieceRef> > unplaced_inner;

    std::map<Board::BoardLoc*,
        std::map<int, std::set< std::shared_ptr<PieceRef> > > > forbidden;

    // stats - TBD needs some more work, what data types?
    std::vector<int> explored;
    std::vector<int> fact;
    int explored_max;

};

}
