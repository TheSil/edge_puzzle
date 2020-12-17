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

        BoardLoc() : hint(nullptr), x(0), y(0)
        {
            // need to be relinked
            neighbours[0] = 0;
            neighbours[1] = 0;
            neighbours[2] = 0;
            neighbours[3] = 0;
        }

        BoardLoc(const BoardLoc& other) : hint(other.hint), x(other.x), y(other.y)
        {
            // need to be relinked
            neighbours[0] = 0;
            neighbours[1] = 0;
            neighbours[2] = 0;
            neighbours[3] = 0;
        }
    };

    Board(const PuzzleDef* def);

    Board(const Board& other);

    void Randomize();

    void AdjustDirBorder();

    void AdjustDirInner();

    const PuzzleDef* GetPuzzleDef() const;

    int GetScore() const;

    int GetScore(BoardLoc* loc);

    std::unique_ptr<Board> Clone() const;

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
    std::vector< std::vector<
        BoardLoc > > board;

    // fast access piece indices
    std::vector< int > corners_ids;
    std::vector< int > edges_ids;
    std::vector< int > inner_ids;
    std::vector< BoardLoc* > locations;

    // fast access coordinates
    std::vector< std::pair<int, int> > corners;
    std::vector< std::pair<int, int> > edges;
    std::vector< std::pair<int, int> > inner;
    std::vector< std::pair<int, int> > top_edges, bottom_edges, left_edges, right_edges;
};

}
