#pragma once

#include <memory>
#include <vector>
#include "PuzzleDef.h"

namespace edge {

class Board
{
public:
    struct BoardLoc
    {
        std::unique_ptr<PieceRef> ref;
        BoardLoc* neighbours[4];
        const HintDef* hint;
        int x, y;

        BoardLoc();
        BoardLoc(const BoardLoc& other);
        BoardLoc& operator= (const BoardLoc& other);
    };

    struct State {
        std::vector< std::vector<
            BoardLoc > > board;
        std::vector< BoardLoc* > locations;
    };

    Board(const PuzzleDef* def);

    State Backup();

    void Restore(State& state);

    void Randomize();

    void AdjustDirBorder();

    void AdjustDirInner();

    const PuzzleDef* GetPuzzleDef() const;

    int GetScore() const;

    int GetScore(BoardLoc* loc);

    std::vector< std::pair<int, int> >& GetCornersCoords();

    std::vector< std::pair<int, int> >& GetEdgesCoords();

    std::vector< std::pair<int, int> >& GetInnersCoords();

    std::vector< BoardLoc* >& GetLocations();

    void SwapLocations(Board::BoardLoc* loc1,
        Board::BoardLoc* loc2);

private:

    void UpdateLinks();

    void UpdateIds();

    bool AdjustDirInner(BoardLoc* loc);

private:

    const PuzzleDef* def;
    State state;

    // fast access piece indices
    std::vector< int > corners_ids;
    std::vector< int > edges_ids;
    std::vector< int > inner_ids;

    // fast access coordinates
    std::vector< std::pair<int, int> > corners;
    std::vector< std::pair<int, int> > edges;
    std::vector< std::pair<int, int> > inner;
    std::vector< std::pair<int, int> > top_edges, bottom_edges, left_edges, right_edges;
};

}
