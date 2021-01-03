#include <algorithm>
#include "ColorAxisCounts.h"

using namespace edge::backtracker;

#define ABS(x) ((x < 0) ? (-x) : (x))

void ColorAxisCounts::Init(const PuzzleDef* def)
{
    for (auto& piece : def->GetAll()) {
        InitColor(piece.second.patterns[0]);
        InitColor(piece.second.patterns[1]);
        InitColor(piece.second.patterns[2]);
        InitColor(piece.second.patterns[3]);
    }
}

void ColorAxisCounts::Place(int k0, int k1, int k2, int k3)
{
    colors_horizontal[k0] += 1;
    colors_horizontal[k2] -= 1;
    colors_vertical[k1] += 1;
    colors_vertical[k3] -= 1;
    colors_available[k0] -= 1;
    colors_available[k1] -= 1;
    colors_available[k2] -= 1;
    colors_available[k3] -= 1;
}

void ColorAxisCounts::Unplace(int k0, int k1, int k2, int k3)
{
    colors_available[k0] += 1;
    colors_available[k1] += 1;
    colors_available[k2] += 1;
    colors_available[k3] += 1;
    colors_horizontal[k0] -= 1;
    colors_horizontal[k2] += 1;
    colors_vertical[k1] -= 1;
    colors_vertical[k3] += 1;
}

bool ColorAxisCounts::CanBeFinished(int k)
{
    return colors_available[k] >= ABS(colors_horizontal[k]) + ABS(colors_vertical[k]);
}

bool ColorAxisCounts::IsFinished()
{
    bool finished = std::all_of(colors_horizontal.begin(), colors_horizontal.end(), [](int i) { return i == 0; });
    finished = finished && std::all_of(colors_vertical.begin(), colors_vertical.end(), [](int i) { return i == 0; });
    finished = finished && std::all_of(colors_available.begin(), colors_available.end(), [](int i) { return i == 0; });
    return finished;
}

void ColorAxisCounts::InitColor(int k)
{
    if (k >= colors_available.size()) {
        colors_available.resize(k + 1);
        colors_horizontal.resize(k + 1);
        colors_vertical.resize(k + 1);
    }
    colors_available[k] += 1;
}
