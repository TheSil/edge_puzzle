#include <random>
#include "PuzzleDef.h"
#include "Board.h"
#include "Swapper.h"


int main()
{
    std::random_device rd;
    unsigned int seed = rd();
    printf("seed: %u\n", seed);
    srand(seed);
    edge::PuzzleDef def = edge::PuzzleDef::Load(
        R"(d:\Git\edge_puzzle\data\eternity2\eternity2_256.csv)",
        R"(d:\Git\edge_puzzle\data\eternity2\eternity2_256_hints.csv)");
    edge::Board board(&def);
    board.Randomize();
    board.AdjustDirBorder();
    board.AdjustDirInner();
    int score = board.GetScore();
    printf("score: %i\n", score);

    edge::Swapper swapper(board);
    while (true) {
        swapper.DoSwap();
    }

    return 0;
}