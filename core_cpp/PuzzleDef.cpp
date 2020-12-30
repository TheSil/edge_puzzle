#include <fstream>
#include <set>
#include "PuzzleDef.h"

using namespace edge;

PuzzleDef::PuzzleDef(int height, int width, int edge_colors, int inner_colors)
    : height(height), width(width)
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
    std::set<int> inner_colors;
    std::set<int> edge_colors;

    while (getline(file, line)) {
        vals.clear();
        ParseNumberLine(line, vals);
        vals.resize(5, 0);

        PieceDef piece(vals[0], vals[1], vals[2], vals[3], vals[4]);
        auto zeroes = std::count(vals.begin(), vals.end(), 0);
        if (zeroes == 2) { // corner
            def.corners.push_back(piece);
            edge_colors.insert((int)piece.patterns[0]);
            edge_colors.insert((int)piece.patterns[1]);
        }
        else if (zeroes == 1) { // edge
            def.edges.push_back(piece);
            edge_colors.insert((int)piece.patterns[0]);
            inner_colors.insert((int)piece.patterns[1]);
            edge_colors.insert((int)piece.patterns[2]);
        }
        else { // inner
            def.inner.push_back(piece);
            inner_colors.insert((int)piece.patterns[0]);
            inner_colors.insert((int)piece.patterns[1]);
            inner_colors.insert((int)piece.patterns[2]);
            inner_colors.insert((int)piece.patterns[3]);

        }
        def.all[vals[0]] = piece;
    }

    def.edge_colors = edge_colors;
    def.inner_colors = inner_colors;

    if (!hints.empty()) {
        std::ifstream hints_file(hints);
        while (getline(hints_file, line)) {
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

int PuzzleDef::GetPieceCount() const
{
    return static_cast<int>(all.size());
}

const std::set<int>& PuzzleDef::GetEdgeColors() const
{
    return edge_colors;
}

const std::set<int>& PuzzleDef::GetInnerColors() const
{
    return inner_colors;
}

PieceDef PuzzleDef::GetPieceDef(int id) const
{
    return all.at(id);
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

const std::map<int, PieceDef>& PuzzleDef::GetAll() const
{
    return all;
}

const std::vector<HintDef>& PuzzleDef::GetHints() const
{
    return hints;
}