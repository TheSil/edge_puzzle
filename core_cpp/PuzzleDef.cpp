#include <fstream>
#include "PuzzleDef.h"

using namespace edge;

PuzzleDef::PuzzleDef(int height, int width, int edge_colors, int inner_colors)
    : height(height), width(width),
    edge_colors(edge_colors), inner_colors(inner_colors)
{
}

PuzzleDef PuzzleDef::Load(const std::string& filename, const std::string& hints)
{
    std::ifstream file(filename);
    std::string line;
    std::vector<int> vals;

    getline(file, line);
    ParseNumberLine(line, vals);
    PuzzleDef def(vals[0], vals[1], vals[2], vals[3]);

    while (getline(file, line)) {
        vals.clear();
        ParseNumberLine(line, vals);
        vals.resize(5, 0);

        PieceDef piece(vals[0], vals[1], vals[2], vals[3], vals[4]);
        auto zeroes = std::count(vals.begin(), vals.end(), 0);
        if (zeroes == 2) { // corner
            def.corners.push_back(piece);
        }
        else if (zeroes == 1) { // edge
            def.edges.push_back(piece);
        }
        else { // inner
            def.inner.push_back(piece);
        }
        def.all[vals[0]] = piece;
    }

    if (!hints.empty()) {
        while (getline(file, line)) {
            vals.clear();
            ParseNumberLine(line, vals);
            vals.resize(4, 0);
            def.hints.push_back(HintDef(vals[0], vals[1], vals[2], vals[3]));
        }
    }

    return def;
}

int PuzzleDef::GetHeight() const
{
    return height;
}

int PuzzleDef::GetWidth() const
{
    return width;
}

const std::vector<PieceDef>& PuzzleDef::GetCorners() const
{
    return corners;
}

const std::vector<PieceDef>& PuzzleDef::GetEdges() const
{
    return edges;
}

const std::vector<PieceDef>& PuzzleDef::GetInner() const
{
    return inner;
}

const std::vector<HintDef>& PuzzleDef::GetHints() const
{
    return hints;
}