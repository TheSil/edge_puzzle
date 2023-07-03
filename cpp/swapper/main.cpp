#include <random>
#include <sstream>
#include "PuzzleDef.h"
#include "Board.h"
#include "Swapper.h"
#include <time.h>
#include <Windows.h>

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

    std::string rotations_file = "";
    if (argc > 3) {
        rotations_file = argv[3];
    }

    edge::PuzzleDef def = edge::PuzzleDef::Load(def_file, hints_file);
    edge::Board board(&def);

    while (true)
    {
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

        int i = 0;
        int start = (int)time(0);
        const int restart_delay = 10;
        int restart_time = (int)time(0) + restart_delay;
        int minimal_save_score = 300;//456;
        int score = board.GetScore();
        int max_score = score;
        printf("score: %i\n", score);
        std::string last_save = "";


        edge::Swapper swapper(board);
        while (true) {
            swapper.DoSwap();
            score = board.GetScore();
            if (board.GetScore() > max_score) {
                max_score = score;
                if (score > minimal_save_score) {
                    //try {
                    //    remove(last_save.c_str());
                    //}
                    //catch (...) {

                    //}
                    std::stringstream ss;
                    ss << prefix << "_swapper_save_" << score << ".csv";
                    last_save = ss.str();
                    board.Save(last_save);
                }

                // score improved, delay restart timer
                restart_time = (int)time(0) + restart_delay;
            }

            if (time(0) > restart_time /*&& max_score < minimal_save_score*/) {
                printf("score did not change too long, restarting\n");
                break;
            }

            i += 1;
            //if (i % 5 == 0) {
            Sleep(1);
            //}
            //int now = (int)time(0);
            //if (now - start >= 1) {
            //    printf("%i iterations/s\n", i);
            //    i = 0;
            //    start = now;
            //}
        }

    }

    return 0;
}