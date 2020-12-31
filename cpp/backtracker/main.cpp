#include <random>
#include <sstream>
#include "PuzzleDef.h"
#include "Board.h"
#include "Backtracker.h"
#include <time.h>
#include <Windows.h>

class NewBest : public edge::backtracker::CallbackOnSolve {
public:
    NewBest(const std::string& prefix) : prefix(prefix), counter(0), max_score(0)
    {
    }

    void Call(edge::Board& board)
    {
        int score = board.GetScore();
        if (score > max_score) {
            max_score = score;
        }
        printf("New best backstack position reached, score: %i\n", score);
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

    int max_score;

private:
    std::string prefix;
    int counter;
    std::string last_save;    
};

class Solved : public edge::backtracker::CallbackOnSolve {
public:
    Solved(const std::string& prefix) : prefix(prefix), counter(0)
    {
    }
    
    void Call(edge::Board& board)
    {
        printf("SOLVED!\n");
        if (counter < 20)
        {// safety mechanism, do not save more than certain number of solutions...
            std::stringstream ss;
            ss << prefix << "_save_" << "solved_" << ++counter << ".csv";
            board.Save(ss.str());
        }

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

    std::string rotations_file = "";
    if (argc > 3) {
        rotations_file = argv[3];
    }

    edge::PuzzleDef def = edge::PuzzleDef::Load(def_file, hints_file);
    edge::Board board(&def);

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
    NewBest newbest_callback(prefix);
    edge::backtracker::Backtracker backtracker(board, pMap, true, rotations_file);
    backtracker.RegisterOnSolve(&solved_callback);
    backtracker.RegisterOnNewBest(&newbest_callback);

    int i = 0;
    int start = (int)time(0);
    int start_absolute = start;
    int score = 0;
    int max_score = 0;
    printf("score: %i\n", score);
    while (backtracker.Step()) {

        i += 1;
        //if (i % 5 == 0) {
        //    Sleep(1);
        //}

        int now = (int)time(0);
        if (now - start >= 1) {
            std::string explAbsLast, explAbs, explMax, explRatio;
            backtracker.GetStats().GetExploredAbsLast().PrintExp(explAbsLast);
            backtracker.GetStats().GetExploredAbs().PrintExp(explAbs);
            backtracker.GetStats().GetExploredRatio().PrintExp(explRatio);
            backtracker.GetStats().GetExploredMax().PrintExp(explMax);

            score = board.GetScore();
             printf("max_score: %i, curr_score: %i, iters: %i, "
                "explAbsLast: %s, explAbs: %s, explRatio: %s, explMax: %s\n", 
                newbest_callback.max_score, score, i, explAbsLast.c_str(), explAbs.c_str(), explRatio.c_str(), explMax.c_str());

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

    return 0;
}