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

class PieceDef
{
public:
    PieceDef(int id = -1,
        int east = 0, int south = 0, int west = 0, int north = 0);

    int GetPattern(int idx) const;

    void SetPattern(int idx, int pattern);

private:
    int id;
    uint8_t patterns[4];
};

class PieceRef
{
public:
    PieceRef(PieceDef def, int dir);

    int GetPattern(int pos) const;

    void SetPattern(int pos, int pattern);

    int GetDir() const;

    void SetDir(int dir);

private:
    PieceDef def;
    int dir;
};

void ParseNumberLine(const std::string& line, std::vector<int>& vals);

}