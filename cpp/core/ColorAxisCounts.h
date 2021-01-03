#pragma once

#include <vector>
#include "PuzzleDef.h"

namespace edge {

namespace backtracker {

class ColorAxisCounts
{

public:
    void Init(const PuzzleDef* def);

    void Place(int k0, int k1, int k2, int k3);

    void Unplace(int k0, int k1, int k2, int k3);

    bool CanBeFinished(int k);

    bool IsFinished();

private:

    void InitColor(int k);

    std::vector<int> colors_horizontal;
    std::vector<int> colors_vertical;
    std::vector<int> colors_available;

};

}

}