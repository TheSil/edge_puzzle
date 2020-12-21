#pragma once

#include <set>
#include <vector>
#include <stack>
#include "Board.h"

namespace edge {

class Backtracker {
public:
    Backtracker(Board& board, 
        std::set<std::pair<int, int>>* pieces_map = nullptr, 
        bool find_all = false);

    bool Step();

    void CheckFeasible(bool ignore_impossible = false);

    bool CanBePlacedAt(Board::BoardLoc* loc, std::shared_ptr<PieceRef> ref);

    void Place(Board::BoardLoc* loc, std::shared_ptr<PieceRef> ref);

    void Backtrack();

    // TBD - needs more works, arbitrary precision computations...
    int Power(int base, int exponent);

    // debug
    int GetCounter();

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
    Board::BoardLoc* backtracked_position;
    bool find_all;
    bool enable_finalizing;
    bool connecting;
    bool constraint_reducing;

    std::set < std::shared_ptr<PieceRef> > feasible_possibilities;
    std::vector<Board::BoardLoc*> best_feasible_locations;
    std::vector<std::vector<
        std::unique_ptr< std::vector<
        std::shared_ptr<PieceRef> > > > > feasible_pieces;

    std::set< std::shared_ptr<PieceRef> >* best_unplaced_container;

    std::set< std::shared_ptr<PieceRef> > unplaced_corners;
    std::set< std::shared_ptr<PieceRef> > unplaced_edges;
    std::set< std::shared_ptr<PieceRef> > unplaced_inner;

    std::map<Board::BoardLoc*,
        std::map<int, std::set< std::shared_ptr<PieceRef> > > > forbidden;
};

}
