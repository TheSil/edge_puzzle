#include <fstream>
#include <algorithm>
#include "Backtracker.h"

using namespace edge::backtracker;

const int ANY_COLOR = 0xFF;

Backtracker::Backtracker(Board& board, std::set<std::pair<int, int>>* pieces_map, bool find_all,
    const std::string& rotations_file)
    : board(board), state(State::SEARCHING),
    find_all(find_all), connecting(true),
    highest_score(0)
{
    //for (int x = 0; x < board.GetPuzzleDef()->GetHeight(); ++x) {
    //    for (int y = 0; y < board.GetPuzzleDef()->GetWidth(); ++y) {
    //        if (!pieces_map || pieces_map->find(std::pair<int, int>(x, y)) != pieces_map->end()) {
    //            unvisited.insert(board.GetLocation(x, y));
    //        }
    //    }
    //}

    if (pieces_map) {
        find_all = true;
    }

    pieces_count = board.GetPuzzleDef()->GetPieceCount();
    stack.visited.top().forbidden.resize(pieces_count);

    std::map<int, int> rotations;
    if (!rotations_file.empty()) {
        std::ifstream file(rotations_file);
        std::string line;
        std::vector<int> vals;

        while (getline(file, line)) {
            vals.clear();
            ParseNumberLine(line, vals);
            vals.resize(2, 0);
            rotations[vals[0]] = vals[1];
        }

        // check consistency of rotations
        ColorAxisCounts checker;
        checker.Init(board.GetPuzzleDef());
        for (auto& item : rotations) {
            PieceRef* ref = board.GetRef(item.first, item.second);
            checker.Place(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH));
        }

        if (checker.IsFinished()) {
            printf("Rotations are OK\n");
        }
        else {
            printf("Rotations are INCONSISTENT!\n");
        }       
    }

    for (auto& piece : board.GetPuzzleDef()->GetAll()) {
        if (!rotations.empty())
        {
            unplaced_pieces.insert(board.GetRef(piece.second.id,rotations[piece.second.id]));
        }
        else {
            for (int dir = 0; dir < 4; ++dir) {
                unplaced_pieces.insert(board.GetRef(piece.second.id,dir));
            }
        }
    }

    stats.Init(board);
#ifdef ROTATION_CHECK
    rot_checker.Init(board.GetPuzzleDef());
#endif

    // hints
    if (true/*rotations_file.empty()*/) // for now, this is how we ignore hints argument if rotations are being checked
    {
        for (auto& hint : board.GetPuzzleDef()->GetHints()) {
            int dir = (hint.dir != -1) ? hint.dir : 0;
            board.PutPiece(hint.id, hint.x, hint.y, dir);
#ifdef ROTATION_CHECK
            rot_checker.Place(board.GetLocation(hint.x, hint.y)->ref->GetPattern(0),
                board.GetLocation(hint.x, hint.y)->ref->GetPattern(1),
                board.GetLocation(hint.x, hint.y)->ref->GetPattern(2),
                board.GetLocation(hint.x, hint.y)->ref->GetPattern(3));
#endif

            std::vector< PieceRef* > to_erase;

            to_erase.clear();
            for (auto& ref : unplaced_pieces) {
                if (ref->GetId() == hint.id) {
                    to_erase.push_back(ref);
                }
            }
            for (auto& ref : to_erase) {
                unplaced_pieces.erase(ref);
            }

            auto loc = board.GetLocation(hint.x, hint.y);
            stack.visited.push(Stack::LevelInfo(pieces_count));
            stack.start_size++;

            path.push_back(loc);
            connected_locations.push_back(std::vector<Board::Loc*>());

            int prev_score = scores.empty() ? 0 : scores.back();
            int neighbours = 0;
            for (int i = 0; i < 4; ++i) {
                neighbours += (loc->neighbours[i] && loc->neighbours[i]->ref) ? 1 : 0;
            }
            scores.push_back(prev_score + neighbours);
        }
    }

    highest_score = static_cast<int>(stack.visited.size());
    board.AdjustDirBorder();

    // create fast access structure for finding all pieces matching
    // given list of patterns
    // TBD - following need refactor + fix to work with specific pieces rotation provided
    for (auto& piece : board.GetPuzzleDef()->GetInner()) {
        for (int dir = 0; dir < 4; ++dir) {
            if (rotations.empty() || rotations[piece.id] == dir)
            {
                auto ref = board.GetRef(piece.id, dir);
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);

                neighbour_table[EncodePatterns(ANY_COLOR, ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ANY_COLOR, ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ANY_COLOR, ref->GetPattern(NORTH))].push_back(ref);
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), ANY_COLOR)].push_back(ref);

                neighbour_table[EncodePatterns(ANY_COLOR, ANY_COLOR, ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
                neighbour_table[EncodePatterns(ANY_COLOR, ref->GetPattern(SOUTH), ANY_COLOR, ref->GetPattern(NORTH))].push_back(ref);
                neighbour_table[EncodePatterns(ANY_COLOR, ref->GetPattern(SOUTH), ref->GetPattern(WEST), ANY_COLOR)].push_back(ref);
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ANY_COLOR, ANY_COLOR, ref->GetPattern(NORTH))].push_back(ref);
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ANY_COLOR, ref->GetPattern(WEST), ANY_COLOR)].push_back(ref);
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ANY_COLOR, ANY_COLOR)].push_back(ref);

                neighbour_table[EncodePatterns(ANY_COLOR, ANY_COLOR, ANY_COLOR, ref->GetPattern(NORTH))].push_back(ref);
                neighbour_table[EncodePatterns(ANY_COLOR, ANY_COLOR, ref->GetPattern(WEST), ANY_COLOR)].push_back(ref);
                neighbour_table[EncodePatterns(ANY_COLOR, ref->GetPattern(SOUTH), ANY_COLOR, ANY_COLOR)].push_back(ref);
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ANY_COLOR, ANY_COLOR, ANY_COLOR)].push_back(ref);

                neighbour_table[EncodePatterns(ANY_COLOR, ANY_COLOR, ANY_COLOR, ANY_COLOR)].push_back(ref);
            }
        }
    }


    for (auto& piece : board.GetPuzzleDef()->GetCorners()) 
    {
        // corners are of form something, something, 0, 0
        {
            if (rotations.empty() || rotations[piece.id] == 0)
            {
                auto ref = board.GetRef(piece.id, 0); // something, something, 0, 0
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);

                neighbour_table[EncodePatterns(ANY_COLOR, ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ANY_COLOR, ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);

                neighbour_table[EncodePatterns(ANY_COLOR, ANY_COLOR, ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
            }
        }

        {
            if (rotations.empty() || rotations[piece.id] == 1)
            {
                auto ref = board.GetRef(piece.id, 1); // 0, something, something, 0
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);

                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ANY_COLOR, ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ANY_COLOR, ref->GetPattern(NORTH))].push_back(ref);

                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ANY_COLOR, ANY_COLOR, ref->GetPattern(NORTH))].push_back(ref);
            }
        }

        {
            if (rotations.empty() || rotations[piece.id] == 2)
            {
                auto ref = board.GetRef(piece.id, 2); // 0, 0, something, something
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);

                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ANY_COLOR, ref->GetPattern(NORTH))].push_back(ref);
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), ANY_COLOR)].push_back(ref);

                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ANY_COLOR, ANY_COLOR)].push_back(ref);
            }
        }

        {
            if (rotations.empty() || rotations[piece.id] == 3)
            {
                auto ref = board.GetRef(piece.id, 3); // something, 0, 0, something
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);

                neighbour_table[EncodePatterns(ANY_COLOR, ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), ANY_COLOR)].push_back(ref);

                neighbour_table[EncodePatterns(ANY_COLOR, ref->GetPattern(SOUTH), ref->GetPattern(WEST), ANY_COLOR)].push_back(ref);
            }
        }

    }

    for (auto& piece : board.GetPuzzleDef()->GetEdges())
    {
        // edges are of form something, something, something, 0
        {
            if (rotations.empty() || rotations[piece.id] == 0)
            {
                auto ref = board.GetRef(piece.id, 0); // something, something, something, 0
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);

                neighbour_table[EncodePatterns(ANY_COLOR, ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ANY_COLOR, ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ANY_COLOR, ref->GetPattern(NORTH))].push_back(ref);

                neighbour_table[EncodePatterns(ANY_COLOR, ANY_COLOR, ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
                neighbour_table[EncodePatterns(ANY_COLOR, ref->GetPattern(SOUTH), ANY_COLOR, ref->GetPattern(NORTH))].push_back(ref);
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ANY_COLOR, ANY_COLOR, ref->GetPattern(NORTH))].push_back(ref);

                neighbour_table[EncodePatterns(ANY_COLOR, ANY_COLOR, ANY_COLOR, ref->GetPattern(NORTH))].push_back(ref);
            }
        }

        {
            if (rotations.empty() || rotations[piece.id] == 1)
            {
                auto ref = board.GetRef(piece.id, 1); // 0, something, something, something 
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);

                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ANY_COLOR, ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ANY_COLOR, ref->GetPattern(NORTH))].push_back(ref);
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), ANY_COLOR)].push_back(ref);

                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ANY_COLOR, ANY_COLOR, ref->GetPattern(NORTH))].push_back(ref);
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ANY_COLOR, ref->GetPattern(WEST), ANY_COLOR)].push_back(ref);
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ANY_COLOR, ANY_COLOR)].push_back(ref);

                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ANY_COLOR, ANY_COLOR, ANY_COLOR)].push_back(ref);
            }
        }

        {
            if (rotations.empty() || rotations[piece.id] == 2)
            {
                auto ref = board.GetRef(piece.id, 2); // something, 0, something, something 
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);

                neighbour_table[EncodePatterns(ANY_COLOR, ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ANY_COLOR, ref->GetPattern(NORTH))].push_back(ref);
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), ANY_COLOR)].push_back(ref);

                neighbour_table[EncodePatterns(ANY_COLOR, ref->GetPattern(SOUTH), ANY_COLOR, ref->GetPattern(NORTH))].push_back(ref);
                neighbour_table[EncodePatterns(ANY_COLOR, ref->GetPattern(SOUTH), ref->GetPattern(WEST), ANY_COLOR)].push_back(ref);
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ANY_COLOR, ANY_COLOR)].push_back(ref);

                neighbour_table[EncodePatterns(ANY_COLOR, ref->GetPattern(SOUTH), ANY_COLOR, ANY_COLOR)].push_back(ref);
            }
        }

        {
            if (rotations.empty() || rotations[piece.id] == 3)
            {
                auto ref = board.GetRef(piece.id, 3); // something, something, 0, something 
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);

                neighbour_table[EncodePatterns(ANY_COLOR, ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ANY_COLOR, ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), ANY_COLOR)].push_back(ref);

                neighbour_table[EncodePatterns(ANY_COLOR, ANY_COLOR, ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
                neighbour_table[EncodePatterns(ANY_COLOR, ref->GetPattern(SOUTH), ref->GetPattern(WEST), ANY_COLOR)].push_back(ref);
                neighbour_table[EncodePatterns(ref->GetPattern(EAST), ANY_COLOR, ref->GetPattern(WEST), ANY_COLOR)].push_back(ref);

                neighbour_table[EncodePatterns(ANY_COLOR, ANY_COLOR, ref->GetPattern(WEST), ANY_COLOR)].push_back(ref);
            }
        }
    }

    // shuffle the neighbour table to give this particular run bit of randomness
    for (auto& item : neighbour_table) {
        std::random_shuffle(item.second.begin(), item.second.end());
    }

    // set path

    // row scan
#if 1
    for (int x = 0; x < board.GetPuzzleDef()->GetHeight(); ++x) {
        for (int y = 0; y < board.GetPuzzleDef()->GetWidth(); ++y) {
            auto loc = board.GetLocation(x, y);
            if (!loc->hint) {
                path.push_back(loc);
            }
        }
    }
#endif

#if 0
    // column scan
    for (int y = 0; y < board.GetPuzzleDef()->GetWidth(); ++y) {
        for (int x = 0; x < board.GetPuzzleDef()->GetHeight(); ++x) {
            auto loc = board.GetLocation(x, y);
            if (!loc->hint) {
                path.push_back(loc);
            }
        }
    }
#endif

#if 0
    // row scan, no jumping
    for (int x = 0; x < board.GetPuzzleDef()->GetHeight(); ++x) {
        for (int y = 0; y < board.GetPuzzleDef()->GetWidth(); ++y) {
            auto loc = board.GetLocation(x, y);
            if (!loc->hint) {
                path.push_back(loc);
            }
        }

        if (x % 2 == 1) {
            std::reverse(path.end() - board.GetPuzzleDef()->GetWidth(), path.end());
        }
    }

#endif

#if 0
    // first corners, then row scan
    for (int x = 0; x < board.GetPuzzleDef()->GetHeight(); ++x) {
        for (int y = 0; y < board.GetPuzzleDef()->GetWidth(); ++y) {
            auto loc = board.GetLocation(x, y);
            if (!loc->hint && loc->type == Board::LocType::CORNER) {
                path.push_back(loc);
            }
        }
    }
    for (int x = 0; x < board.GetPuzzleDef()->GetHeight(); ++x) {
        for (int y = 0; y < board.GetPuzzleDef()->GetWidth(); ++y) {
            auto loc = board.GetLocation(x, y);
            if (!loc->hint && loc->type != Board::LocType::CORNER) {
                path.push_back(loc);
            }
        }
    }
#endif

#if 0
    // diagonals
    for (int y0 = 0; y0 < 2 * board.GetPuzzleDef()->GetWidth(); ++y0) {
        for (int t = 0; t <= y0; ++t) {
            int x = t;
            int y = y0 - t;
            if (y < board.GetPuzzleDef()->GetWidth() && x < board.GetPuzzleDef()->GetHeight()) {
                auto loc = board.GetLocation(x, y);
                if (!loc->hint) {
                    path.push_back(loc);
                }
            }
        }
    }
#endif

#if 0
    // diagonals around corner hints first, then row scan rest
    for (int y0 = 0; y0 < board.GetPuzzleDef()->GetWidth() / 2; ++y0) {
        for (int t = 0; t <= y0; ++t) {
            int x = t;
            int y = y0 - t;
            if (y < board.GetPuzzleDef()->GetWidth() && x < board.GetPuzzleDef()->GetHeight()) {
                auto loc = board.GetLocation(x, y);
                if (!loc->hint) {
                    path.push_back(loc);
                }
            }
        }
    }

    for (int y0 = 0; y0 < board.GetPuzzleDef()->GetWidth() / 2; ++y0) {
        for (int t = 0; t <= y0; ++t) {
            int x = t;
            int y = board.GetPuzzleDef()->GetWidth() - 1 - (y0 - t);
            if (y < board.GetPuzzleDef()->GetWidth() && x < board.GetPuzzleDef()->GetHeight()) {
                auto loc = board.GetLocation(x, y);
                if (!loc->hint) {
                    path.push_back(loc);
                }
            }
        }
    }

    for (int y0 = 0; y0 < board.GetPuzzleDef()->GetWidth() / 2; ++y0) {
        for (int t = 0; t <= y0; ++t) {
            int x = board.GetPuzzleDef()->GetHeight() - 1 - t;
            int y = y0 - t;
            if (y < board.GetPuzzleDef()->GetWidth() && x < board.GetPuzzleDef()->GetHeight()) {
                auto loc = board.GetLocation(x, y);
                if (!loc->hint) {
                    path.push_back(loc);
                }
            }
        }
    }

    for (int y0 = 0; y0 < board.GetPuzzleDef()->GetWidth() / 2; ++y0) {
        for (int t = 0; t <= y0; ++t) {
            int x = board.GetPuzzleDef()->GetHeight() - 1 - t;
            int y = board.GetPuzzleDef()->GetWidth() - 1 - (y0 - t);
            if (y < board.GetPuzzleDef()->GetWidth() && x < board.GetPuzzleDef()->GetHeight()) {
                auto loc = board.GetLocation(x, y);
                if (!loc->hint) {
                    path.push_back(loc);
                }
            }
        }
    }

    for (int x = 0; x < board.GetPuzzleDef()->GetHeight(); ++x) {
        for (int y = 0; y < board.GetPuzzleDef()->GetWidth(); ++y) {
            auto loc = board.GetLocation(x, y);
            if (!loc->hint) {
                auto it = std::find(path.begin(), path.end(), loc);
                if (it == path.end()) {
                    path.push_back(loc);
                }
            }
        }
    }

#endif

#if 0
    // going from outer to inner
    for (int t = 0; t < board.GetPuzzleDef()->GetWidth() / 2; ++t) {
        int x, y;

        // (t,t) -> (t,w-t-2)
        x = t;
        for (y = t; y <= board.GetPuzzleDef()->GetWidth() - t - 2; ++y) {
            auto loc = board.GetLocation(x, y);
            if (!loc->hint) {
                path.push_back(loc);
            }
        }

        // (t,w-t-1) -> (h-t-2,w-t-1)
        y = board.GetPuzzleDef()->GetWidth() - t - 1;
        for (x = t; x <= board.GetPuzzleDef()->GetHeight() - t - 2; ++x) {
            auto loc = board.GetLocation(x, y);
            if (!loc->hint) {
                path.push_back(loc);
            }
        }

        // (h-t-1,w-t-1) -> (h-t-1,t+1)
        x = board.GetPuzzleDef()->GetHeight() - t - 1;
        for (y = board.GetPuzzleDef()->GetWidth() - t - 1; y >= t + 1; --y) {
            auto loc = board.GetLocation(x, y);
            if (!loc->hint) {
                path.push_back(loc);
            }
        }

        // (h-t-1,t) -> (t+1,t)
        y = t;
        for (x = board.GetPuzzleDef()->GetHeight() - t - 1; x >= t + 1; --x) {
            auto loc = board.GetLocation(x, y);
            if (!loc->hint) {
                path.push_back(loc);
            }
        }
    }
#endif

#if 0
    // 2 rows at a time (diagonally)
    // 1 2 4 6 ...
    // 3 5 7 ...
    for (int x0 = 0; x0 < board.GetPuzzleDef()->GetHeight(); x0 += 2) {
        for (int y0 = 0; y0 < board.GetPuzzleDef()->GetWidth() + 1; ++y0) {
            for (int t = 0; t <= y0; ++t) {
                if (t >= 2) break;
                int x = x0 + t;
                int y = y0 - t;
                if (y < board.GetPuzzleDef()->GetWidth() && x < board.GetPuzzleDef()->GetHeight()) {
                    auto loc = board.GetLocation(x, y);
                    if (!loc->hint) {
                        path.push_back(loc);
                    }
                }
            }
        }
    }
#endif

#if 0
    // row scan, second half reverted
    for (int x = 0; x < board.GetPuzzleDef()->GetHeight(); ++x) {
        for (int y = 0; y < board.GetPuzzleDef()->GetWidth(); ++y) {
            auto loc = board.GetLocation(x, y);
            if (!loc->hint) {
                path.push_back(loc);
            }
        }
    }
    std::reverse(path.begin() + path.size()/2, path.end());
#endif

#if 0
    // going from inner to outer - reversed
    for (int t = 0; t < board.GetPuzzleDef()->GetWidth() / 2; ++t) {
        int x, y;

        // (t,t) -> (t,w-t-2)
        x = t;
        for (y = t; y <= board.GetPuzzleDef()->GetWidth() - t - 2; ++y) {
            auto loc = board.GetLocation(x, y);
            if (!loc->hint) {
                path.push_back(loc);
            }
        }

        // (t,w-t-1) -> (h-t-2,w-t-1)
        y = board.GetPuzzleDef()->GetWidth() - t - 1;
        for (x = t; x <= board.GetPuzzleDef()->GetHeight() - t - 2; ++x) {
            auto loc = board.GetLocation(x, y);
            if (!loc->hint) {
                path.push_back(loc);
            }
        }

        // (h-t-1,w-t-1) -> (h-t-1,t+1)
        x = board.GetPuzzleDef()->GetHeight() - t - 1;
        for (y = board.GetPuzzleDef()->GetWidth() - t - 1; y >= t + 1; --y) {
            auto loc = board.GetLocation(x, y);
            if (!loc->hint) {
                path.push_back(loc);
            }
        }

        // (h-t-1,t) -> (t+1,t)
        y = t;
        for (x = board.GetPuzzleDef()->GetHeight() - t - 1; x >= t + 1; --x) {
            auto loc = board.GetLocation(x, y);
            if (!loc->hint) {
                path.push_back(loc);
            }
        }
    }
    std::reverse(path.begin(), path.end());
#endif

#if 0
    // around the corner
    for (int y0 = 0; y0 < board.GetPuzzleDef()->GetWidth(); ++y0) {
        for (int x = 0; x < y0; ++x) {
            int y = y0;
            auto loc = board.GetLocation(x, y);
            if (!loc->hint) {
                path.push_back(loc);
            }
        }

        int x = y0;
        for (int y = y0; y >= 0; y--) {
            auto loc = board.GetLocation(x, y);
            if (!loc->hint) {
                path.push_back(loc);
            }
        }
    }
#endif

#if 0
    // around the corner - no jumps
    for (int y0 = 0; y0 < board.GetPuzzleDef()->GetWidth(); ++y0) {
        
        for (int x = 0; x < y0; ++x) {
            int y = y0;
            auto loc = board.GetLocation(x, y);
            if (!loc->hint) {
                path.push_back(loc);
            }
        }

        int x = y0;
        for (int y = y0; y >= 0; y--) {
            auto loc = board.GetLocation(x, y);
            if (!loc->hint) {
                path.push_back(loc);
            }
        }

        if (y0 % 2 == 0) {
            std::reverse(path.end() - 2*y0-1, path.end());
        }
    }

#endif


    // row scan 2x2 blocks
#if 0
    for (int x = 0; x < board.GetPuzzleDef()->GetHeight(); x+=2) {
        for (int y = 0; y < board.GetPuzzleDef()->GetWidth(); y+=2) {
            auto loc = board.GetLocation(x, y);
            if (!loc->hint) {
                path.push_back(loc);
            }
            if (y + 1 < board.GetPuzzleDef()->GetWidth())
            {
                loc = board.GetLocation(x, y + 1);
                if (!loc->hint) {
                    path.push_back(loc);
                }
            }
            if (x + 1 < board.GetPuzzleDef()->GetHeight()) {
                loc = board.GetLocation(x + 1, y);
                if (!loc->hint) {
                    path.push_back(loc);
                }
                if (y + 1 < board.GetPuzzleDef()->GetWidth())
                {
                    loc = board.GetLocation(x + 1, y + 1);
                    if (!loc->hint) {
                        path.push_back(loc);
                    }
                }
            }
        }
    }
#endif

#if 0
    // border, then row scan
    int t = 0; // t = 0 corresponds to first iteration of outer from inner, giving a border
    int x, y;

    // (t,t) -> (t,w-t-2)
    x = t;
    for (y = t; y <= board.GetPuzzleDef()->GetWidth() - t - 2; ++y) {
        auto loc = board.GetLocation(x, y);
        if (!loc->hint) {
            path.push_back(loc);
        }
    }

    // (t,w-t-1) -> (h-t-2,w-t-1)
    y = board.GetPuzzleDef()->GetWidth() - t - 1;
    for (x = t; x <= board.GetPuzzleDef()->GetHeight() - t - 2; ++x) {
        auto loc = board.GetLocation(x, y);
        if (!loc->hint) {
            path.push_back(loc);
        }
    }

    // (h-t-1,w-t-1) -> (h-t-1,t+1)
    x = board.GetPuzzleDef()->GetHeight() - t - 1;
    for (y = board.GetPuzzleDef()->GetWidth() - t - 1; y >= t + 1; --y) {
        auto loc = board.GetLocation(x, y);
        if (!loc->hint) {
            path.push_back(loc);
        }
    }

    // (h-t-1,t) -> (t+1,t)
    y = t;
    for (x = board.GetPuzzleDef()->GetHeight() - t - 1; x >= t + 1; --x) {
        auto loc = board.GetLocation(x, y);
        if (!loc->hint) {
            path.push_back(loc);
        }
    }

    // inner row scan
    for (int x = 1; x < board.GetPuzzleDef()->GetHeight() - 1; ++x) {
        for (int y = 1; y < board.GetPuzzleDef()->GetWidth() - 1; ++y) {
            auto loc = board.GetLocation(x, y);
            if (!loc->hint) {
                path.push_back(loc);
            }
        }
    }

#endif

#if 0
    // inner row scan
    for (int x = 1; x < board.GetPuzzleDef()->GetHeight() - 1; ++x) {
        for (int y = 1; y < board.GetPuzzleDef()->GetWidth() - 1; ++y) {
            auto loc = board.GetLocation(x, y);
            if (!loc->hint) {
                path.push_back(loc);
            }
        }
    }

    // inner row scan, then border 
    int t = 0; // t = 0 corresponds to first iteration of outer from inner, giving a border
    int x, y;

    // (t,t) -> (t,w-t-2)
    x = t;
    for (y = t; y <= board.GetPuzzleDef()->GetWidth() - t - 2; ++y) {
        auto loc = board.GetLocation(x, y);
        if (!loc->hint) {
            path.push_back(loc);
        }
    }

    // (t,w-t-1) -> (h-t-2,w-t-1)
    y = board.GetPuzzleDef()->GetWidth() - t - 1;
    for (x = t; x <= board.GetPuzzleDef()->GetHeight() - t - 2; ++x) {
        auto loc = board.GetLocation(x, y);
        if (!loc->hint) {
            path.push_back(loc);
        }
    }

    // (h-t-1,w-t-1) -> (h-t-1,t+1)
    x = board.GetPuzzleDef()->GetHeight() - t - 1;
    for (y = board.GetPuzzleDef()->GetWidth() - t - 1; y >= t + 1; --y) {
        auto loc = board.GetLocation(x, y);
        if (!loc->hint) {
            path.push_back(loc);
        }
    }

    // (h-t-1,t) -> (t+1,t)
    y = t;
    for (x = board.GetPuzzleDef()->GetHeight() - t - 1; x >= t + 1; --x) {
        auto loc = board.GetLocation(x, y);
        if (!loc->hint) {
            path.push_back(loc);
        }
    }

#endif

#if 0
    // around the corner for up to 8x8, then row scan for mainder
    for (int y0 = 0; y0 < std::min(board.GetPuzzleDef()->GetWidth(), 8); ++y0) {
        for (int x = 0; x < y0; ++x) {
            int y = y0;
            auto loc = board.GetLocation(x, y);
            if (!loc->hint) {
                path.push_back(loc);
            }
        }

        int x = y0;
        for (int y = y0; y >= 0; y--) {
            auto loc = board.GetLocation(x, y);
            if (!loc->hint) {
                path.push_back(loc);
            }
        }
    }

    for (int x = 0; x < board.GetPuzzleDef()->GetHeight(); ++x) {
        for (int y = 0; y < board.GetPuzzleDef()->GetWidth(); ++y) {
            auto loc = board.GetLocation(x, y);
            if (!loc->hint) {
                if (find(path.begin(), path.end(), loc) == path.end()) {
                    path.push_back(loc);
                }
                
            }
        }
    }


#endif

    // row scan 4x4 blocks
#if 0
    for (int x0 = 0; x0 < board.GetPuzzleDef()->GetHeight(); x0 += 4) {
        for (int y0 = 0; y0 < board.GetPuzzleDef()->GetWidth(); y0 += 4) {
            // around the corner
            for (int y2 = 0; y2 < 4; ++y2) {
                for (int x2 = 0; x2 < y2; ++x2) {
                    int x = x2 + x0;
                    int y = y2 + y0;
                    if (x < board.GetPuzzleDef()->GetHeight() && y < board.GetPuzzleDef()->GetWidth()) {
                        auto loc = board.GetLocation(x, y);
                        if (!loc->hint) {
                            path.push_back(loc);
                        }
                    }
                }

                int x2 = y2;
                for (int y3 = y2; y3 >= 0; y3--) {
                    int x = x2 + x0;
                    int y = y3 + y0;
                    if (x < board.GetPuzzleDef()->GetHeight() && y < board.GetPuzzleDef()->GetWidth()) {
                        auto loc = board.GetLocation(x, y);
                        if (!loc->hint) {
                            path.push_back(loc);
                        }
                    }
                }
            }

        }
    }
#endif

    // row, column, row, ... scan
#if 0
    for (int t = 0; t < board.GetPuzzleDef()->GetWidth(); ++t)
    {
        int x = t; // row scan
        for (int y = 0; y < board.GetPuzzleDef()->GetWidth(); ++y) {
            auto loc = board.GetLocation(x, y);
            if (!loc->hint) {
                if (find(path.begin(), path.end(), loc) == path.end()) {
                    path.push_back(loc);
                }
            }
        }

        int y = t;
        for (int x = 0; x < board.GetPuzzleDef()->GetHeight(); ++x) {
            auto loc = board.GetLocation(x, y);
            if (!loc->hint) {
                if (find(path.begin(), path.end(), loc) == path.end()) {
                    path.push_back(loc);
                }
            }
        }
    }
#endif

    // simple consistency check, each location must be present in path 
    // exactly once
    if (!path.empty())
    {
        std::set<Board::Loc*> path_locs;
        for (auto& loc : path) {
            path_locs.insert(loc);
        }
        if (path_locs.size() != board.GetPuzzleDef()->GetHeight() * board.GetPuzzleDef()->GetWidth()) {
            throw std::exception("Not all locations visited in path!");
        }
    }

    int r = 0;

}

int Backtracker::EncodePatterns(int east, int south, int west, int north)
{
    int encoded = east;

    encoded *= ANY_COLOR + 1;
    encoded += south;

    encoded *= ANY_COLOR + 1;
    encoded += west;

    encoded *= ANY_COLOR + 1;
    encoded += north;

    return encoded;
}

bool Backtracker::Step()
{
    switch (state)
    {
    case State::SEARCHING:
    {
        if (stack.visited.size() - 1 == pieces_count) {
            for (auto& callback : on_solve) {
                callback->Call(board);
            }

            if (!find_all) {
                // everything already placed, solved...
                state = State::FINISHED;
                return false;
            }

            state = State::BACKTRACKING;
            return true;
        }

        // this branch will be called at most once per number of pieces, not time critical
        if (connected_locations.size() < stack.visited.size()) {
            // we have not created a map of connected locations for this location, do this now
            std::set<Board::Loc*> connected; // set to avoid duplicates at first

            auto& locations_map = board.GetLocations();
            for (int x = 0; x < board.GetPuzzleDef()->GetHeight(); ++x) {
                for (int y = 0; y < board.GetPuzzleDef()->GetWidth(); ++y) {
                    auto loc = board.GetLocation(x, y);
                    if (!loc->ref) { // nothing here yet

                        auto& east_loc = loc->neighbours[EAST];
                        auto& south_loc = loc->neighbours[SOUTH];
                        auto& west_loc = loc->neighbours[WEST];
                        auto& north_loc = loc->neighbours[NORTH];

                        int neighbour_count = 0;
                        neighbour_count += (east_loc && east_loc->ref) ? 1 : 0;
                        neighbour_count += (south_loc && south_loc->ref) ? 1 : 0;
                        neighbour_count += (west_loc && west_loc->ref) ? 1 : 0;
                        neighbour_count += (north_loc && north_loc->ref) ? 1 : 0;

                        if (neighbour_count != 0)
                        {
                            connected.insert(loc);
                        }
                    }
                }
            }

            // convert set to vector for faster iteration later
            std::vector<Board::Loc*> connected_vec;
            for (auto& loc : connected) {
                connected_vec.push_back(loc);
            }
            connected_locations.push_back(connected_vec);
        }

        // check whether there are some connecting spots which cannot be filled by anything...
        auto& locations_map = board.GetLocations();
        for(auto& loc : connected_locations[stack.visited.size() - 1])
        {
            auto& east_loc = loc->neighbours[EAST];
            auto& south_loc = loc->neighbours[SOUTH];
            auto& west_loc = loc->neighbours[WEST];
            auto& north_loc = loc->neighbours[NORTH];

            int key = EncodePatterns(!east_loc ? 0 : (east_loc->ref ? east_loc->ref->GetPattern(WEST) : ANY_COLOR),
                !south_loc ? 0 : (south_loc->ref ? south_loc->ref->GetPattern(NORTH) : ANY_COLOR),
                !west_loc ? 0 : (west_loc->ref ? west_loc->ref->GetPattern(EAST) : ANY_COLOR),
                !north_loc ? 0 : (north_loc->ref ? north_loc->ref->GetPattern(SOUTH) : ANY_COLOR));

            auto it = neighbour_table.find(key);
            if (it == neighbour_table.end())
            { // no piece found that can match this combination of pattern
                state = State::BACKTRACKING;
                return true;
            }

            bool has_feasible = false;
            for (auto& piece : it->second) {
                if (!locations_map[piece->GetId()]) { // not yet placed
                    has_feasible = true;
                    break;
                }
            }

            if (!has_feasible) {
                // there is a position where nothing can be placed, backtrack
                state = State::BACKTRACKING;
                return true;
            }
        }

        // this branch will be called at most once per number of pieces, not time critical
        if (path.size() < stack.visited.size()) {
            // path not defined, we create our own as we go

            // we check all connected pieces, and find such that it contains least feasible possibilities
            auto& locations_map = board.GetLocations();
            int min_feasible_count = -1;
            std::vector<Board::Loc*> min_locs;
            for (int x = 0; x < board.GetPuzzleDef()->GetHeight(); ++x) {
                for (int y = 0; y < board.GetPuzzleDef()->GetWidth(); ++y) {
                    auto loc = board.GetLocation(x, y);
                    if (!loc->ref) { // nothing here yet

                        auto& east_loc = loc->neighbours[EAST];
                        auto& south_loc = loc->neighbours[SOUTH];
                        auto& west_loc = loc->neighbours[WEST];
                        auto& north_loc = loc->neighbours[NORTH];

#if 1
                        int neighbour_count = 0;
                        neighbour_count += (east_loc && east_loc->ref) ? 1 : 0;
                        neighbour_count += (south_loc && south_loc->ref) ? 1 : 0;
                        neighbour_count += (west_loc && west_loc->ref) ? 1 : 0;
                        neighbour_count += (north_loc && north_loc->ref) ? 1 : 0;

                        if (neighbour_count == 0 && path.size() > 0) {
                            // only add locations that are connected
                            continue;
                        }
#endif

                        int key = EncodePatterns(!east_loc ? 0 : (east_loc->ref ? east_loc->ref->GetPattern(WEST) : ANY_COLOR),
                            !south_loc ? 0 : (south_loc->ref ? south_loc->ref->GetPattern(NORTH) : ANY_COLOR),
                            !west_loc ? 0 : (west_loc->ref ? west_loc->ref->GetPattern(EAST) : ANY_COLOR),
                            !north_loc ? 0 : (north_loc->ref ? north_loc->ref->GetPattern(SOUTH) : ANY_COLOR));

                        auto it = neighbour_table.find(key);
                        if (it == neighbour_table.end())
                        { // no piece found that can match this combination of pattern
                            state = State::BACKTRACKING;
                            return true;
                        }

                        int feasible_count = 0;
                        for (auto& piece : it->second) {
                            if (!locations_map[piece->GetId()]) { // not yet placed
                                feasible_count += 1;
                            }
                        }

                        if (min_feasible_count == -1 || feasible_count < min_feasible_count) {
                            min_feasible_count = feasible_count;
                            min_locs.clear();
                            min_locs.push_back(loc);
                        }
                        else if (feasible_count == min_feasible_count)
                        {
                            min_locs.push_back(loc);
                        }
                    }
                }
            }

            int idx = rand() % min_locs.size();
            path.push_back(min_locs[idx]);

            // debug
            printf("path updated: ");
            for (auto& loc : path) {
                printf(",(%i, %i)", loc->x, loc->y);
            }
            printf("\n");
        }

        PieceRef* selected_piece = nullptr;
        Board::Loc* selected_loc = nullptr;

        int best_score = CheckFeasible(selected_loc, selected_piece);
        if (best_score <= 0) {
            // impossible to place anything here... backtrack
            state = State::BACKTRACKING;

            return true;
        }

        Place(selected_loc, selected_piece);
        unplaced_pieces.erase(selected_piece);

        // if inconsistent rotations, backtrack...
#ifdef ROTATION_CHECK
        if (!rot_checker.CanBeFinished(selected_piece->GetPattern(0)) ||
            !rot_checker.CanBeFinished(selected_piece->GetPattern(1)) ||
            !rot_checker.CanBeFinished(selected_piece->GetPattern(2)) ||
            !rot_checker.CanBeFinished(selected_piece->GetPattern(3)) ) {
            LDEBUG("Inconsistent rotation, initating backtrack...\n");
            state = State::BACKTRACKING;
        }
#endif

        return true;
    }
    break;
    case State::BACKTRACKING:
    {
        if (Backtrack()) {
            state = State::SEARCHING;
        }
        else {
            // we should now unwrap beyond hint piece if there is any 
            // this forces all pieces beyound it to be counted properly
            if (stack.visited.size() > 1) {
                stats.Update(static_cast<int>(stack.visited.size()) - 1);
            }

            state = State::FINISHED;
        }
    }
    break;
    case State::FINISHED:
    {
        return false;
    }
    break;
    default:
        break;
    }

    return true;
}

int Backtracker::CheckFeasible(Board::Loc*& feasible_location,
    PieceRef*& feasible_piece)
{
    int best_score = -1;

    PieceRef* selected = nullptr;
    Board::Loc* selected_loc = nullptr;
    auto& forbidden_map = stack.visited.top().forbidden;
    auto& locations_map = board.GetLocations();

    // TBD we should visit unvisited in random order too ...
    auto& loc = path[stack.visited.size() - 1];

    auto& east_loc = loc->neighbours[EAST];
    auto& south_loc = loc->neighbours[SOUTH];
    auto& west_loc = loc->neighbours[WEST];
    auto& north_loc = loc->neighbours[NORTH];

    int key = EncodePatterns(!east_loc ? 0 : (east_loc->ref ? east_loc->ref->GetPattern(WEST) : ANY_COLOR),
        !south_loc ? 0 : (south_loc->ref ? south_loc->ref->GetPattern(NORTH) : ANY_COLOR),
        !west_loc ? 0 : (west_loc->ref ? west_loc->ref->GetPattern(EAST) : ANY_COLOR),
        !north_loc ? 0 : (north_loc->ref ? north_loc->ref->GetPattern(SOUTH) : ANY_COLOR));

    auto it = neighbour_table.find(key);
    if (it == neighbour_table.end())
    {
        return 0;
    }

    int feasible_count = 0;
    PieceRef* repre = nullptr;
    for (auto& piece : it->second) {
        if (!locations_map[piece->GetId()]) {
            if (!forbidden_map[piece->GetId()][piece->GetDir()]){
                repre = piece;
                feasible_count += 1;
            }
        }
    }

    if (feasible_count == 0) {
        // impossible to place anything here, end asap
        return 0;
    }

    if ((best_score == -1 || feasible_count < best_score)) {
        best_score = feasible_count;
        selected = repre;
        selected_loc = loc;
    }

    feasible_location = selected_loc;
    feasible_piece = selected;
    return best_score;
}

void Backtracker::Place(Board::Loc* loc, PieceRef* ref)
{
    LDEBUG("Placing %i at (%i, %i) [%i, %i, %i, %i]  dir=%i stack_size=%i\n",
        ref->GetId(), loc->x, loc->y, 
        ref->GetPattern(0), ref->GetPattern(1), ref->GetPattern(2), ref->GetPattern(3),
        ref->GetDir(), static_cast<int>(stack.visited.size()) + 1);
    board.PutPiece(loc, ref);
#ifdef ROTATION_CHECK
    rot_checker.Place(ref->GetPattern(0),
        ref->GetPattern(1),
        ref->GetPattern(2),
        ref->GetPattern(3));
#endif
    switch (loc->type)
    {
    case Board::LocType::CORNER:
        stats.UpdateUnplacedCorners(-1);
        break;
    case Board::LocType::EDGE:
        stats.UpdateUnplacedEdges(-1);
        break;
    case Board::LocType::INNER:
        stats.UpdateUnplacedInner(-1);
        break;
    default:
        break;
    }

    stack.visited.push(Stack::LevelInfo(pieces_count));

    // update scores cache (specific for position in path)
    if (scores.size() < stack.visited.size() - 1) {
        int prev_score = scores.empty() ? 0 : scores.back();
        int neighbours = 0;
        for (int i = 0; i < 4; ++i) {
            neighbours += (loc->neighbours[i] && loc->neighbours[i]->ref) ? 1 : 0;
        }
        int new_score = prev_score + neighbours;
        scores.push_back(new_score);

        if (new_score > highest_score) {
            highest_score = new_score;

            for (auto& callback : on_new_best) {
                callback->Call(board);
            }
        }
    }    
}

bool Backtracker::Backtrack()
{
    if (stack.IsEmpty()) {
        return false;
    }

    Board::Loc* removing = path[stack.visited.size() - 2];
    stack.visited.pop();
    int stack_pos = static_cast<int>(stack.visited.size());

    // update statistics
    stats.Update(stack_pos);

    LDEBUG("Removing %i from (%i, %i) [%i, %i, %i, %i] stack_size=%i\n",
        removing->ref->GetId(), 
        removing->x, removing->y, 
        removing->ref->GetPattern(0), removing->ref->GetPattern(1), removing->ref->GetPattern(2), removing->ref->GetPattern(3),
        static_cast<int>(stack.visited.size()));

    stack.visited.top().forbidden[removing->ref->GetId()][removing->ref->GetDir()] = true;

    unplaced_pieces.insert(removing->ref);
    if (removing->type == Board::LocType::CORNER) {
        stats.UpdateUnplacedCorners(1);
    }
    else if (removing->type == Board::LocType::EDGE) {
        stats.UpdateUnplacedEdges(1);
    }
    else {
        stats.UpdateUnplacedInner(1);
    }
#ifdef ROTATION_CHECK
    rot_checker.Unplace(removing->ref->GetPattern(0),
        removing->ref->GetPattern(1),
        removing->ref->GetPattern(2),
        removing->ref->GetPattern(3));
#endif
    board.RemovePiece(removing);

    return true;
}

Stats& Backtracker::GetStats()
{
    return stats;
}

void Backtracker::RegisterOnSolve(CallbackOnSolve* callback)
{
    on_solve.push_back(callback);
}

void Backtracker::RegisterOnNewBest(CallbackOnSolve* callback)
{
    on_new_best.push_back(callback);
}
