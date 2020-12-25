#pragma once

#include <set>
#include <vector>
#include <stack>
#include <gmp.h>
#include "Board.h"

namespace edge {

namespace backtracker {

class Stack {
public:

    struct LevelInfo {
        Board::Loc* loc;
        std::map<Board::Loc*,
            std::set< std::shared_ptr<PieceRef> > > forbidden;

        LevelInfo(Board::Loc* loc) : loc(loc)
        {
        }
    };

    bool IsEmpty() {
        // empty == only root
        return visited.size() == 1;
    }

    Stack() : backtrack_to(0)
    {
        visited.push(LevelInfo(nullptr)); // root
    }

    std::stack<LevelInfo> visited;
    int backtrack_to;

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
    virtual void call(Board& board) = 0;

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
        bool find_all = false);

    bool Step();

    void CheckFeasible(std::vector<std::vector<
        std::unique_ptr< std::vector<
        std::shared_ptr<PieceRef> > > > >& feasible_pieces, 
        std::vector<Board::Loc*>& best_feasible_locations, 
        std::set< std::shared_ptr<PieceRef> >*& best_unplaced_container,
        bool ignore_impossible = false);

    bool CanBePlacedAt(Board::Loc* loc, std::shared_ptr<PieceRef> ref);

    void Place(Board::Loc* loc, std::shared_ptr<PieceRef> ref);

    bool Backtrack();

    // debug
    int GetCounter();

    Stats& GetStats();

    void RegisterOnSolve(CallbackOnSolve* callback);

private:
    enum class State {
        SEARCHING = 0,
        BACKTRACKING = 1,
        FINALIZING = 2,
        FINISHED = 3
    };

    Board& board;
    State state;
    Stack stack;

    int best_score;
    int counter;
    int finalizing_threshold;

    std::set<Board::Loc*> unvisited;
    bool find_all;
    bool enable_finalizing;
    bool connecting;
    bool constraint_reducing;

    std::set< std::shared_ptr<PieceRef> > unplaced_corners;
    std::set< std::shared_ptr<PieceRef> > unplaced_edges;
    std::set< std::shared_ptr<PieceRef> > unplaced_inner;

    std::vector< CallbackOnSolve* > on_solve;

    Stats stats;
    ColorAxisCounts rotChecker;
};

}

}
