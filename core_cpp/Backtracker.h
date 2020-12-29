#pragma once

#include <set>
#include <vector>
#include <stack>
#include <unordered_map>
#include <array>
#include <gmp.h>
#include "Board.h"

namespace edge {

namespace backtracker {

class Stack {
public:

    struct LevelInfo {
        Board::Loc* loc;
        std::unordered_map<Board::Loc*,
            std::set< std::shared_ptr<PieceRef> > > forbidden;

        LevelInfo(Board::Loc* loc) : loc(loc)
        {
        }
    };

    bool IsEmpty() {
        // empty == only root, plus possible hints
        return visited.size() == start_size;
    }

    Stack() : backtrack_to(0), start_size(1)
    {
        visited.push(LevelInfo(nullptr)); // root
    }

    std::stack<LevelInfo> visited;
    int backtrack_to;
    int start_size;

private:

};

class MpfWrapper {
public:
    MpfWrapper(mpf_ptr other)
    {
        mpf_init(val);
        mpf_set(val, other);
    }

    MpfWrapper(const MpfWrapper& other)
    {
        mpf_init(val);
        mpf_set(val, other.val);
    }

    MpfWrapper& operator=(const MpfWrapper& other)
    {
        if (this != &other)
        {
            mpf_init(val);
            mpf_set(val, other.val);
        }
        return *this;
    }

    ~MpfWrapper()
    {
        mpf_clear(val);
    }

    void PrintExp(std::string& out)
    {
        char buf[128];
        gmp_sprintf(buf, "%.2FE", val);
        out = buf;
    }

private:
    mpf_t val;

};



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

class CallbackOnSolve {
public:
    virtual void Call(Board& board) = 0;

};

#define ABS(x) ((x < 0) ? (-x) : (x))

class ColorAxisCounts
{

public:
    void Init(const PuzzleDef* def);

    void Place(int k0, int k1, int k2, int k3);

    void Unplace(int k0, int k1, int k2, int k3);

    bool CanBeFinished(int k);

private:

    void InitColor(int k);

    std::vector<int> colors_horizontal;
    std::vector<int> colors_vertical;
    std::vector<int> colors_available;

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

    // debug
    int GetCounter();

private:
    int CheckFeasible(Board::Loc*& feasible_location,
        std::shared_ptr<PieceRef>& feasible_piece);

    void Place(Board::Loc* loc, std::shared_ptr<PieceRef>& ref);

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
    int finalizing_threshold;
    int highest_stack_pos;

    // cached
    int height;
    int width;

    std::set<Board::Loc*> unvisited;
    bool find_all;
    bool connecting;

    std::set< std::shared_ptr<PieceRef> > unplaced_pieces;

    std::unordered_map<int, std::vector< std::shared_ptr<PieceRef>>> neighbour_table;
    int color_count;

    std::vector< CallbackOnSolve* > on_solve;
    std::vector< CallbackOnSolve* > on_new_best;

    std::vector< std::array<std::shared_ptr<PieceRef>, 4> > refs; // pool owning all pieces references

    Stats stats;
    ColorAxisCounts rotChecker;
};

}

}
