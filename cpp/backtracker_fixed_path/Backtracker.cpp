#include <fstream>
#include "Backtracker.h"

using namespace edge::backtracker;

const int ANY_COLOR = 0xFF;

Backtracker::Backtracker(Board& board, std::set<std::pair<int, int>>* pieces_map, bool find_all,
    const std::string& rotations_file)
    : board(board),
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
    rot_checker.Init(board.GetPuzzleDef());

    // hints
    for (auto& hint : board.GetPuzzleDef()->GetHints()) {
        int dir = (hint.dir != -1) ? hint.dir : 0;
        board.PutPiece(hint.id, hint.x, hint.y, dir);
        rot_checker.Place(board.GetLocation(hint.x, hint.y)->ref->GetPattern(0),
            board.GetLocation(hint.x, hint.y)->ref->GetPattern(1), 
            board.GetLocation(hint.x, hint.y)->ref->GetPattern(2), 
            board.GetLocation(hint.x, hint.y)->ref->GetPattern(3));

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
        random_shuffle(item.second.begin(), item.second.end());
    }

    // set path

    // row scan
    for (int x = 0; x < board.GetPuzzleDef()->GetHeight(); ++x) {
        for (int y = 0; y < board.GetPuzzleDef()->GetWidth(); ++y) {
            auto loc = board.GetLocation(x, y);
            if (!loc->hint) {
                path.push_back(loc);
            }
        }
    }

    // first corners, then row scan
    //for (int x = 0; x < board.GetPuzzleDef()->GetHeight(); ++x) {
    //    for (int y = 0; y < board.GetPuzzleDef()->GetWidth(); ++y) {
    //        auto loc = board.GetLocation(x, y);
    //        if (!loc->hint && loc->type == Board::LocType::CORNER) {
    //            path.push_back(loc);
    //        }
    //    }
    //}
    //for (int x = 0; x < board.GetPuzzleDef()->GetHeight(); ++x) {
    //    for (int y = 0; y < board.GetPuzzleDef()->GetWidth(); ++y) {
    //        auto loc = board.GetLocation(x, y);
    //        if (!loc->hint && loc->type != Board::LocType::CORNER) {
    //            path.push_back(loc);
    //        }
    //    }
    //}

    // going from outer to inner
    //for (int t = 0; t < board.GetPuzzleDef()->GetWidth()/2; ++t) {
    //    int x, y;

    //    // (t,t) -> (t,w-t-2)
    //    x = t;
    //    for (y = t; y <= board.GetPuzzleDef()->GetWidth() - t - 2; ++y) {
    //        auto loc = board.GetLocation(x, y);
    //        if (!loc->hint) {
    //            path.push_back(loc);
    //        }
    //    }

    //    // (t,w-t-1) -> (h-t-2,w-t-1)
    //    y = board.GetPuzzleDef()->GetWidth() - t - 1;
    //    for (x = t; x <= board.GetPuzzleDef()->GetHeight() - t - 2; ++x) {
    //        auto loc = board.GetLocation(x, y);
    //        if (!loc->hint) {
    //            path.push_back(loc);
    //        }
    //    }        

    //    // (h-t-1,w-t-1) -> (h-t-1,t+1)
    //    x = board.GetPuzzleDef()->GetHeight() - t - 1;
    //    for (y = board.GetPuzzleDef()->GetWidth() - t - 1; y >= t+1; --y) {
    //        auto loc = board.GetLocation(x, y);
    //        if (!loc->hint) {
    //            path.push_back(loc);
    //        }
    //    }

    //    // (h-t-1,t) -> (t+1,t)
    //    y = t;
    //    for (x = board.GetPuzzleDef()->GetHeight() - t - 1; x >= t+1; --x) {
    //        auto loc = board.GetLocation(x, y);
    //        if (!loc->hint) {
    //            path.push_back(loc);
    //        }
    //    }
    //}


    // column scan
    //for (int y = 0; y < board.GetPuzzleDef()->GetWidth(); ++y) {
    //    for (int x = 0; x < board.GetPuzzleDef()->GetHeight(); ++x) {
    //        auto loc = board.GetLocation(x, y);
    //        if (!loc->hint) {
    //            path.push_back(loc);
    //        }
    //    }
    //}

    // diagonals
    //for (int y0 = 0; y0 < 2*board.GetPuzzleDef()->GetWidth(); ++y0) {
    //    for (int t = 0; t <= y0; ++t) {
    //        int x = t;
    //        int y = y0 - t;
    //        if (y < board.GetPuzzleDef()->GetWidth() && x < board.GetPuzzleDef()->GetHeight()) {
    //            auto loc = board.GetLocation(x, y);
    //            if (!loc->hint) {
    //                path.push_back(loc);
    //            }
    //        }
    //    }
    //}

    // 2 rows at a time (digonally)
    // 1 2 4 6 ...
    // 3 5 7 ...
    //for (int x0 = 0; x0 < board.GetPuzzleDef()->GetHeight(); x0 += 2) {
    //    for (int y0 = 0; y0 < board.GetPuzzleDef()->GetWidth() + 1; ++y0) {
    //        for (int t = 0; t <= y0; ++t) {
    //            if (t >= 2) break;
    //            int x = x0 + t;
    //            int y = y0 - t;
    //            if (y < board.GetPuzzleDef()->GetWidth() && x < board.GetPuzzleDef()->GetHeight()) {
    //                auto loc = board.GetLocation(x, y);
    //                if (!loc->hint) {
    //                    path.push_back(loc);
    //                }
    //            }
    //        }
    //    }
    //}

    int x = 0;

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
        if (stack.visited.size() - 1 == path.size()) {
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
        if (!rot_checker.CanBeFinished(selected_piece->GetPattern(0)) ||
            !rot_checker.CanBeFinished(selected_piece->GetPattern(1)) ||
            !rot_checker.CanBeFinished(selected_piece->GetPattern(2)) ||
            !rot_checker.CanBeFinished(selected_piece->GetPattern(3)) ) {
            LDEBUG("Inconsistent rotation, initating backtrack...\n");
            state = State::BACKTRACKING;
        }

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
    rot_checker.Place(ref->GetPattern(0),
        ref->GetPattern(1),
        ref->GetPattern(2),
        ref->GetPattern(3));
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

    int prev_score = stack.visited.top().score;
    stack.visited.push(Stack::LevelInfo(pieces_count));
    int neighbours = 0;
    for (int i = 0; i < 4; ++i) {
        neighbours += (loc->neighbours[i] && loc->neighbours[i]->ref) ? 1 : 0;
    }
    int new_score = prev_score + neighbours;
    stack.visited.top().score = new_score;


    if (new_score > highest_score) {
        highest_score = new_score;

        for (auto& callback : on_new_best) {
            callback->Call(board);
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
    rot_checker.Unplace(removing->ref->GetPattern(0),
        removing->ref->GetPattern(1),
        removing->ref->GetPattern(2),
        removing->ref->GetPattern(3));
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
