#pragma once

#include <cstdint>
#include <string>

#define LINFO(f_, ...) printf((f_), __VA_ARGS__)
//#define LDEBUG(f_, ...) printf((f_), __VA_ARGS__)
#define LDEBUG(f_, ...) 

namespace edge {

enum Dir
{
    EAST = 0,
    SOUTH = 1,
    WEST = 2,
    NORTH = 3
};

struct PieceDef
{
    PieceDef(int id = -1,
        int east = 0, int south = 0, int west = 0, int north = 0)
        : id(id)
    {
        patterns[0] = east;
        patterns[1] = south;
        patterns[2] = west;
        patterns[3] = north;
    }

    int id;
    uint8_t patterns[4];
};

struct HintDef
{
    HintDef(int x, int y, int id, int dir)
        : x(x), y(y), id(id), dir(dir)
    {
    }

    int x, y, id, dir;
};

class PieceRef
{
public:
    PieceRef(PieceDef def, int dir);

    int GetPattern(int pos) const;

    void SetPattern(int pos, int pattern);

    int GetDir() const;

    void SetDir(int dir);

    int GetId() const;

private:
    PieceDef def;
    int dir;
};

void ParseNumberLine(const std::string& line, std::vector<int>& vals);

}