#include <vector>
#include <sstream>
#include "Defs.h"

using namespace edge;

PieceDef::PieceDef(int id, int east, int south, int west, int north)
    : id(id)
{
    patterns[0] = east;
    patterns[1] = south;
    patterns[2] = west;
    patterns[3] = north;
}

int PieceDef::GetPattern(int idx) const
{
    return patterns[idx];
}

void PieceDef::SetPattern(int idx, int pattern)
{
    patterns[idx] = pattern;
}

PieceRef::PieceRef(PieceDef def, int dir)
    : def(def), dir(dir)
{
}

int PieceRef::GetPattern(int pos) const
{
    return def.GetPattern((pos - dir + 4) % 4);
}

void PieceRef::SetPattern(int pos, int pattern)
{
    return def.SetPattern((pos - dir + 4) % 4, pattern);
}

int PieceRef::GetDir() const
{
    return dir;
}

void PieceRef::SetDir(int dir)
{
    this->dir = dir;
}

void edge::ParseNumberLine(const std::string& line, std::vector<int>& vals)
{
    std::string val_str;
    std::stringstream ss(line);
    while (getline(ss, val_str, ',')) {
        int val;
        if (std::stringstream(val_str) >> val) {
            vals.push_back(val);
        }
    }
}

