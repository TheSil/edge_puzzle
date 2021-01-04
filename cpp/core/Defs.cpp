#include <vector>
#include <sstream>
#include "Defs.h"

using namespace edge;

PieceRef::PieceRef(PieceDef def, int dir)
    : def(def), dir(dir)
{
    for (int i = 0; i < 4; ++i) {
        oriented_patterns[i] = def.patterns[(i - dir + 4) % 4];
    }
}

int PieceRef::GetPattern(int pos) const
{
    return oriented_patterns[pos];
}

int PieceRef::GetDir() const
{
    return dir;
}

int PieceRef::GetId() const
{
    return def.id;
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

PieceDef::PieceDef(int id, int east, int south, int west, int north)
    : id(id)
{
    patterns[0] = east;
    patterns[1] = south;
    patterns[2] = west;
    patterns[3] = north;
}

HintDef::HintDef(int x, int y, int id, int dir)
    : x(x), y(y), id(id), dir(dir)
{
}
