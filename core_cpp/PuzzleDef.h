#pragma once

#include <vector>
#include <map>
#include <string>
#include "Defs.h"

namespace edge {

class PuzzleDef
{
public:
    PuzzleDef(int height, int width, int edge_colors, int inner_colors);

    static PuzzleDef Load(const std::string& filename);

    int GetHeight() const;

    int GetWidth() const;

    const std::vector<PieceDef>& GetCorners() const;

    const std::vector<PieceDef>& GetEdges() const;

    const std::vector<PieceDef>& GetInner() const;


private:
    int height, width, edge_colors, inner_colors;
    std::vector<PieceDef> corners, edges, inner;
    //std::vector<void> hints;
    std::map<int, PieceDef> all;
};

}
