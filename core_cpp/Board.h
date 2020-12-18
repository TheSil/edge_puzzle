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
        std::shared_ptr<PieceRef> ref;
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
        std::vector< BoardLoc* > locations_per_id;
    };

    Board(const PuzzleDef* def);

    void Save(const std::string& filename);

    void Load(const std::string& filename);

    Board::State Backup();

    void Restore(Board::State& state);

    void Randomize();

    void AdjustDirBorder();

    void AdjustDirInner();

    void PutPiece(int id, int x, int y, int dir);

    void PutPiece(Board::BoardLoc* loc, std::shared_ptr<PieceRef> ref);

    void RemovePiece(Board::BoardLoc* loc);

    const PuzzleDef* GetPuzzleDef() const;

    int GetScore() const;

    int GetScore(BoardLoc* loc);

    std::vector< std::pair<int, int> >& GetCornersCoords();

    std::vector< std::pair<int, int> >& GetEdgesCoords();

    std::vector< std::pair<int, int> >& GetInnersCoords();

    std::vector< BoardLoc* >& GetLocations();

    void SwapLocations(Board::BoardLoc* loc1,
        Board::BoardLoc* loc2);

    BoardLoc* GetLocation(int x, int y);

    bool IsCorner(int x, int y);

    bool IsInner(int x, int y);

    bool IsEdge(int x, int y);


private:

    void UpdateLinks();

    void UpdateIds();

    bool AdjustDirInner(BoardLoc* loc);

private:

    const PuzzleDef* def;
    Board::State state;

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
