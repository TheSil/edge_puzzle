#pragma once

#include <memory>
#include <vector>
#include <array>
#include "PuzzleDef.h"

namespace edge {

class Board
{
public:
    enum class LocType {
        UNKNOWN,
        CORNER,
        EDGE,
        INNER
    };

    struct Loc
    {
        PieceRef* ref;
        Loc* neighbours[4];
        const HintDef* hint;
        int x, y;
        LocType type;


        Loc();
        Loc(const Loc& other);
        Loc& operator= (const Loc& other);
    };

    struct State {
        std::vector< std::vector<
            Loc > > board;
        std::vector< Loc* > locations_per_id;
        std::vector< std::array<std::shared_ptr<PieceRef>, 4> > refs; // pool owning all pieces references
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

    void PutPiece(Board::Loc* loc, PieceRef* ref);

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

    PieceRef* GetRef(int id, int dir);

    void ChangeDir(Board::Loc* loc, int dir);

private:

    void UpdateLinks();

    void UpdateIds();

    bool AdjustDirInner(Loc* loc);

    void AdjustDirBorderSafe(Loc* loc, int dir);

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
