#pragma once

#include <memory>
#include <vector>
#include "PuzzleDef.h"

namespace edge {

class Board
{
public:
    struct Loc
    {
        std::shared_ptr<PieceRef> ref;
        Loc* neighbours[4];
        const HintDef* hint;
        int x, y;

        Loc();
        Loc(const Loc& other);
        Loc& operator= (const Loc& other);
    };

    struct State {
        std::vector< std::vector<
            Loc > > board;
        std::vector< Loc* > locations_per_id;
    };

    Board(const PuzzleDef* def);

    void Save(const std::string& filename);

    void Load(const std::string& filename);

    Board::State Backup();

    void Restore(Board::State& state);

    void Randomize();

    void AdjustDirBorder();

    void AdjustDirBorderSingle(Board::Loc* loc);

    void AdjustDirInner();

    void PutPiece(int id, int x, int y, int dir);

    void PutPiece(Board::Loc* loc, std::shared_ptr<PieceRef> ref);

    void RemovePiece(Board::Loc* loc);

    const PuzzleDef* GetPuzzleDef() const;

    int GetScore() const;

    int GetScore(Loc* loc);

    std::vector< std::pair<int, int> >& GetCornersCoords();

    std::vector< std::pair<int, int> >& GetEdgesCoords();

    std::vector< std::pair<int, int> >& GetInnersCoords();

    std::vector< Loc* >& GetLocations();

    void SwapLocations(Board::Loc* loc1,
        Board::Loc* loc2);

    Loc* GetLocation(int x, int y);

    bool IsCorner(int x, int y);

    bool IsInner(int x, int y);

    bool IsEdge(int x, int y);


private:

    void UpdateLinks();

    void UpdateIds();

    bool AdjustDirInner(Loc* loc);

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
