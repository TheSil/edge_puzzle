#include <random>
#include <sstream>
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
    int max_score = score;
    printf("score: %i\n", score);

    edge::Swapper swapper(board);
    while (true) {
        swapper.DoSwap();
        score = board.GetScore();
        if (board.GetScore() > max_score) {
            max_score = score;
            if (score > 400) {
                std::stringstream ss;
                ss << "save_" << score << ".csv";
                board.Save(ss.str());
            }
        }
    }

    return 0;
}