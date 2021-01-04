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
        int east = 0, int south = 0, int west = 0, int north = 0);

    int id;
    uint8_t patterns[4];
};

struct HintDef
{
    HintDef(int x, int y, int id, int dir);

    int x, y, id, dir;
};

class PieceRef
{
public:
    PieceRef(PieceDef def, int dir);

    int GetPattern(int pos) const;

    int GetDir() const;

    int GetId() const;

private:
    PieceDef def;
    int dir;
    uint8_t oriented_patterns[4];
};

void ParseNumberLine(const std::string& line, std::vector<int>& vals);

}