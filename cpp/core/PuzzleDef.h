#pragma once

#include <vector>
#include <map>
#include <set>
#include <string>
#include "Defs.h"

namespace edge {

class PuzzleDef
{
public:
    PuzzleDef(int height, int width, int edge_colors, int inner_colors);

    static PuzzleDef Load(const std::string& filename, const std::string& hints = "");

    int GetHeight() const;

    int GetWidth() const;

    int GetPieceCount() const;

    const std::set<int>& GetEdgeColors() const;

    const std::set<int>& GetInnerColors() const;

    PieceDef GetPieceDef(int id) const;

    const std::vector<PieceDef>& GetCorners() const;

    const std::vector<PieceDef>& GetEdges() const;

    const std::vector<PieceDef>& GetInner() const;

    const std::map<int, PieceDef>& GetAll() const;

    const std::vector<HintDef>& GetHints() const;

private:
    int height, width;
    std::vector<PieceDef> corners, edges, inner;
    std::vector<HintDef> hints;
    std::map<int, PieceDef> all;
    std::set<int> edge_colors, inner_colors;

};

}
