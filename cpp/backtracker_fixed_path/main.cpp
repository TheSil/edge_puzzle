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
        if (true){
        //if (score > 330/*420*/) {
            std::stringstream ss;
            try {
                remove(last_save.c_str());
            }
            catch (...) {

            }
            ss << score << "_" << prefix << "_backtracker_save.csv";
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
        printf("SOLVED! (%i)\n", counter + 1);
        if (counter < 20)
        {// safety mechanism, do not save more than certain number of solutions...
            std::stringstream ss;
            ss << prefix << "_save_" << "solved_" << counter+1 << ".csv";
            board.Save(ss.str());
        }
        ++counter;

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
    //static const char alphanum[] =
    //    "0123456789"
    //    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    //    "abcdefghijklmnopqrstuvwxyz";
    std::string prefix;
    std::stringstream ss;
    ss << seed;
    prefix = ss.str();
    //prefix.resize(8);
    //for (int i = 0; i < prefix.size(); ++i)
    //{
    //    prefix[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    //}
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

    bool restarting = false; // disable to avoid restarting
    int restart_under_score = 400;
    int restart_seconds = 2 * 60;

    while (true) {
        std::random_device rd;
        unsigned int seed = rd();
        //seed = 1060869576; // debug
        printf("seed: %u\n", seed);
        srand(seed);
        // generate prefix for saves
        //static const char alphanum[] =
        //    "0123456789"
        //    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        //    "abcdefghijklmnopqrstuvwxyz";
        std::string prefix;
        std::stringstream ss;
        ss << seed;
        prefix = ss.str();
        //prefix.resize(8);
        //for (int i = 0; i < prefix.size(); ++i)
        //{
        //    prefix[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
        //}
        printf("save_prefix: %s\n", prefix.c_str());

        edge::PuzzleDef def = edge::PuzzleDef::Load(def_file, hints_file);
        edge::Board board(&def);

        std::set<std::pair<int, int>>* pMap = nullptr;

        Solved solved_callback(prefix);
        NewBest newbest_callback(prefix);
        edge::backtracker::Backtracker backtracker(board, pMap, true, rotations_file);
        backtracker.RegisterOnSolve(&solved_callback);
        backtracker.RegisterOnNewBest(&newbest_callback);

        int i = 0;
        long long total = 0;
        int start = (int)time(0);
        int start_absolute = start;
        int score = 0;
        int max_score = 0;
        printf("score: %i\n", score);

        bool keep_going = true;
        while (keep_going) {
            total += 1;
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
                printf("max_score: %i, iters: %lli (+%i) "
                    "expl: %s/%s (%s +%s)\n",
                    newbest_callback.max_score, total, i, explAbs.c_str(), explMax.c_str(), explRatio.c_str(), explAbsLast.c_str());

                Sleep(10);
                i = 0;
                start = now;
            }

            if (restarting) {
                if ((int)time(0) - start_absolute >= restart_seconds) {
                    score = board.GetScore();
                    if (score < restart_under_score) {
                        printf("Score %i below threshold %i, restarting...\n", score, restart_under_score);

                        break;
                    }
                    // otherwise ignore this branch completely and complete with this one indefinitely
                    restarting = false;
                }
            }

            keep_going = backtracker.Step();
        }

        if (!keep_going) {
            printf("finished in %i sec, total iterations: %lli\n", (int)time(0) - start_absolute, total);
            std::string explAbsLast, explAbs, explMax, explRatio;
            backtracker.GetStats().GetExploredAbsLast().PrintExp(explAbsLast);
            backtracker.GetStats().GetExploredAbs().PrintExp(explAbs);
            backtracker.GetStats().GetExploredRatio().PrintExp(explRatio);
            backtracker.GetStats().GetExploredMax().PrintExp(explMax);

            printf("max_score: %i, iters: %i, "
                "explAbsLast: %s, explRatio: %s, explMax: %s\n",
                max_score, i, explAbsLast.c_str(), explRatio.c_str(), explMax.c_str());
            break;
        }
    }

    return 0;
}