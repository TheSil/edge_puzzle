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
        int x, y;
    };

    Board(const PuzzleDef* def);

    void Randomize();

    void AdjustDirBorder();

    void AdjustDirInner();

    int GetScore() const;

    int GetScore(BoardLoc* loc);

    std::unique_ptr<Board> Clone() const;

    std::vector< BoardLoc* >& GetCorners();

    std::vector< BoardLoc* >& GetEdges();

    std::vector< BoardLoc* >& GetInners();

private:

    void UpdateLinks();

    bool AdjustDirInner(BoardLoc* loc);

private:

    const PuzzleDef* def;
    std::vector< std::vector<
        BoardLoc > > board;

    // fast access pointers
    std::vector< BoardLoc* > corners;
    std::vector< BoardLoc* > top_edges, bottom_edges, left_edges, right_edges;
    std::vector< BoardLoc* > edges;
    std::vector< BoardLoc* > inner;
};

}
