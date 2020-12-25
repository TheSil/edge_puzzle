#include <random>
#include <sstream>
#include "PuzzleDef.h"
#include "Board.h"
#include "Swapper.h"
#include "Backtracker.h"
#include <time.h>
#include <Windows.h>

class Solved : public edge::backtracker::CallbackOnSolve {
public:
    Solved(const std::string& prefix) : prefix(prefix), counter(0)
    {
    }
    
    void call(edge::Board& board)
    {
        printf("SOLVED!\n");
        std::stringstream ss;
        ss << prefix << "_save_" << "solved_" << ++counter << ".csv";
        board.Save(ss.str());
    }

private:
    std::string prefix;
    int counter;
};

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

    std::set<std::pair<int, int>>* pMap = nullptr;
    //tested fields map
    //std::set<std::pair<int, int>> map;
    //for (int x = 0; x < def.GetHeight(); ++x) {
    //    for (int y = 0; y < def.GetWidth(); ++y) {
    //        if (x <= 2 || y <= 2 || x >= def.GetHeight() - 3 || y >= def.GetWidth() - 3)
    //            map.insert(std::pair<int,int>(x,y));
    //    }
    //}
    //pMap = &map;

    Solved solved_callback(prefix);
    edge::backtracker::Backtracker backtracker(board, pMap, true);
    backtracker.RegisterOnSolve(&solved_callback);

    int i = 0;
    int start = (int)time(0);
    int start_absolute = start;
    int score = 0;
    int max_score = 0;
    int prev_counter = 0;
    printf("score: %i\n", score);
    std::string last_save = "";
    while (backtracker.Step()) {

        score = board.GetScore();
        if (score > max_score) {
            max_score = score;
            printf("Reached score: %i\n", max_score);
            if (score > 330/*420*/) {
                std::stringstream ss;
                try {
                    remove(last_save.c_str());
                }
                catch (...) {

                }
                ss << prefix << "_backtracker_save_" << score << ".csv";
                last_save = ss.str();
                board.Save(last_save);
            }
        }

        i += 1;
        //if (i % 5 == 0) {
        //    Sleep(1);
        //}
        if (backtracker.GetCounter() != prev_counter) {
            prev_counter = backtracker.GetCounter();
            printf("counter: %i \n", prev_counter);
        }

        int now = (int)time(0);
        if (now - start >= 1) {
            std::string explAbsLast, explAbs, explMax, explRatio;
            backtracker.GetStats().GetExploredAbsLast().PrintExp(explAbsLast);
            backtracker.GetStats().GetExploredAbs().PrintExp(explAbs);
            backtracker.GetStats().GetExploredRatio().PrintExp(explRatio);
            backtracker.GetStats().GetExploredMax().PrintExp(explMax);

            printf("max_score: %i, curr_score: %i, iters: %i, "
                "explAbsLast: %s, explAbs: %s, explRatio: %s, explMax: %s\n", 
                max_score, score, i, explAbsLast.c_str(), explAbs.c_str(), explRatio.c_str(), explMax.c_str());

            Sleep(10);
            i = 0;
            start = now;
        }
    }

    printf("finished in %i sec\n", (int)time(0) - start_absolute);
    std::string explAbsLast, explAbs, explMax, explRatio;
    backtracker.GetStats().GetExploredAbsLast().PrintExp(explAbsLast);
    backtracker.GetStats().GetExploredAbs().PrintExp(explAbs);
    backtracker.GetStats().GetExploredRatio().PrintExp(explRatio);
    backtracker.GetStats().GetExploredMax().PrintExp(explMax);

    printf("max_score: %i, curr_score: %i, iters: %i, "
        "explAbsLast: %s, explAbs: %s, explRatio: %s, explMax: %s\n",
        max_score, score, i, explAbsLast.c_str(), explAbs.c_str(), explRatio.c_str(), explMax.c_str());

#endif

#if 0 // swapper


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
        int minimal_save_score = 456;
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
                    try {
                        remove(last_save.c_str());
                    }
                    catch (...) {

                    }
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

#endif

    return 0;
}