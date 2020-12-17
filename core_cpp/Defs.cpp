#include <vector>
#include <sstream>
#include "Defs.h"

using namespace edge;

PieceRef::PieceRef(PieceDef def, int dir)
    : def(def), dir(dir)
{
}

int PieceRef::GetPattern(int pos) const
{
    return def.patterns[(pos - dir + 4) % 4];
}

void PieceRef::SetPattern(int pos, int pattern)
{
    def.patterns[(pos - dir + 4) % 4] = pattern;
}

int PieceRef::GetDir() const
{
    return dir;
}

void PieceRef::SetDir(int dir)
{
    this->dir = dir;
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

