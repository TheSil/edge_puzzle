#include <random>
#include <sstream>
#include "PuzzleDef.h"
#include "Board.h"
#include "Swapper.h"
#include "Backtracker.h"
#include <time.h>

int main(int argc, char* argv[])
{
    std::random_device rd;
    unsigned int seed = rd();
    printf("seed: %u\n", seed);
    srand(seed);
    // generate prefix for saves
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string prefix;
    prefix.resize(8);
    for (int i = 0; i < prefix.size(); ++i)
    {
        prefix[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    printf("save_prefix: %s\n", prefix.c_str());

    // fixed arguments for now
    if (argc <= 1) {
        printf("Missing puzzle definition argument\n");
        return 1;
    }

    std::string def_file = argv[1];
    std::string hints_file = "";
    if (argc > 2) {
        hints_file = argv[2];
    }

    edge::PuzzleDef def = edge::PuzzleDef::Load(def_file, hints_file);
    edge::Board board(&def);

#if 1 // BACKTRACKER

    edge::Backtracker backtracker(board);
    int i = 0;
    int start = (int)time(0);
    int score = 0;
    int max_score = 0;
    printf("score: %i\n", score);
    while (backtracker.Step()) {

        score = board.GetScore();
        if (score > max_score) {
            max_score = score;
            printf("Reached score: %i\n", max_score);
            if (score > 390) {
                std::stringstream ss;
                ss << prefix << "_save_" << score << ".csv";
                board.Save(ss.str());
            }
        }

        i += 1;
        //int now = (int)time(0);
        //if (now - start >= 1) {
        //    printf("%i iterations/s\n", i);
        //    i = 0;
        //    start = now;
        //}
    }

    printf("SOLVED!\n");
    std::stringstream ss;
    ss << prefix << "_save_" << "solved" << ".csv";
    board.Save(ss.str());
#endif

#if 0 // swapper

    std::string load_file = "";
    if (argc > 3) {
        load_file = argv[3];
        board.Load(load_file);
        // some save files have missing pieces,
        // but swapper expects filled board
        for (auto& piece : def.GetCorners())
        {
            if (!board.GetLocations()[piece.id]) {
                for (auto& coord : board.GetCornersCoords()) {
                    auto loc = board.GetLocation(coord.first, coord.second);
                    if (!loc->ref) {
                        board.PutPiece(piece.id, loc->x, loc->y, 0);
                    }
                }
            }
        }
        for (auto& piece : def.GetEdges())
        {
            if (!board.GetLocations()[piece.id]) {
                for (auto& coord : board.GetEdgesCoords()) {
                    auto loc = board.GetLocation(coord.first, coord.second);
                    if (!loc->ref) {
                        board.PutPiece(piece.id, loc->x, loc->y, 0);
                    }
                }
            }
        }
        for (auto& piece : def.GetInner())
        {
            if (!board.GetLocations()[piece.id]) {
                for (auto& coord : board.GetInnersCoords()) {
                    auto loc = board.GetLocation(coord.first, coord.second);
                    if (!loc->ref) {
                        board.PutPiece(piece.id, loc->x, loc->y, 0);
                    }
                }
            }
        }
        board.AdjustDirBorder();
    }
    else {
        board.Randomize();
        board.AdjustDirBorder();
        board.AdjustDirInner();
    }

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
                ss << prefix << "_save_" << score << ".csv";
                board.Save(ss.str());
            }
        }
    }

#endif

    return 0;
}