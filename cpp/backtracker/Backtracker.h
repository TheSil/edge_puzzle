#pragma once

#include <array>
#include "Board.h"
#include "MpfWrapper.h"
#include "Stack.h"
#include "Stats.h"
#include "ColorAxisCounts.h"

namespace edge {

namespace backtracker {

class CallbackOnSolve {
public:
    virtual void Call(Board& board) = 0;

};

class Backtracker {
public:
    Backtracker(Board& board,
        std::set<std::pair<int, int>>* pieces_map = nullptr,
        bool find_all = false,
        const std::string& rotations_file = "");

    bool Step();

    Stats& GetStats();

    void RegisterOnSolve(CallbackOnSolve* callback);

    void RegisterOnNewBest(CallbackOnSolve* callback);

private:
    int CheckFeasible(Board::Loc*& feasible_location,
        PieceRef*& feasible_piece);

    void Place(Board::Loc* loc, PieceRef* ref);

    bool Backtrack();

    int EncodePatterns(int east, int south, int west, int north);

private:
    enum class State {
        SEARCHING = 0,
        BACKTRACKING = 1,
        FINISHED = 2
    };

    Board& board;
    State state;
    Stack stack;

    int counter;
    int highest_score;

    std::set<Board::Loc*> unvisited;
    bool find_all;
    bool connecting;

    std::set< PieceRef* > unplaced_pieces;

    std::unordered_map<int, std::vector< PieceRef*>> neighbour_table;
    int max_color_number;

    std::vector< CallbackOnSolve* > on_solve;
    std::vector< CallbackOnSolve* > on_new_best;

    Stats stats;
    ColorAxisCounts rot_checker;

};

}

}
