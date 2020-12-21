#include <sstream>
#include <algorithm>
#include "Backtracker.h"

using namespace edge;

Backtracker::Backtracker(Board& board, std::set<std::pair<int, int>>* pieces_map, bool find_all)
    : board(board), best_score(-1), backtrack_to(0),
    backtracked_position(nullptr), find_all(find_all), connecting(true),
    counter(0), finalizing_threshold(90), enable_finalizing(false),
    constraint_reducing(false),
    best_unplaced_container(nullptr)
{
    for (int x = 0; x < board.GetPuzzleDef()->GetHeight(); ++x) {
        for (int y = 0; y < board.GetPuzzleDef()->GetWidth(); ++y) {
            if (!pieces_map || pieces_map->find(std::pair<int,int>(x,y)) != pieces_map->end())
            unvisited.insert(board.GetLocation(x, y));

        }
    }

    if (pieces_map) {
        find_all = true;
    }

    for (auto& piece : board.GetPuzzleDef()->GetCorners()) {
        for (int dir = 0; dir < 4; ++dir) {
            unplaced_corners.insert(
                std::make_shared<PieceRef>(piece, dir));
        }
    }

    for (auto& piece : board.GetPuzzleDef()->GetEdges()) {
        for (int dir = 0; dir < 4; ++dir) {
            unplaced_edges.insert(
                std::make_shared<PieceRef>(piece, dir));
        }
    }

    for (auto& piece : board.GetPuzzleDef()->GetInner()) {
        for (int dir = 0; dir < 4; ++dir) {
            unplaced_inner.insert(
                std::make_shared<PieceRef>(piece, dir));
        }
    }

    for (int x = 0; x < board.GetPuzzleDef()->GetHeight(); ++x) {
        for (int y = 0; y < board.GetPuzzleDef()->GetWidth(); ++y) {
            forbidden[board.GetLocation(x, y)].clear();
        }
    }

    // hints
    for (auto& hint : board.GetPuzzleDef()->GetHints()) {
        int dir = (hint.dir != -1) ? hint.dir : 0;
        board.PutPiece(hint.id, hint.x, hint.y, dir);

        std::vector< std::shared_ptr<PieceRef> > to_erase;

        to_erase.clear();
        for (auto& ref : unplaced_corners) {
            if (ref->GetId() == hint.id) {
                to_erase.push_back(ref);
            }
        }
        for (auto& ref : to_erase) {
            unplaced_corners.erase(ref);
        }

        to_erase.clear();
        for (auto& ref : unplaced_edges) {
            if (ref->GetId() == hint.id) {
                to_erase.push_back(ref);
            }
        }
        for (auto& ref : to_erase) {
            unplaced_edges.erase(ref);
        }

        to_erase.clear();
        for (auto& ref : unplaced_inner) {
            if (ref->GetId() == hint.id) {
                to_erase.push_back(ref);
            }
        }
        for (auto& ref : to_erase) {
            unplaced_inner.erase(ref);
        }

        auto loc = board.GetLocation(hint.x, hint.y);
        unvisited.erase(loc);
        visited.push(loc);
    }

    board.AdjustDirBorder();
}

bool Backtracker::Step()
{
    switch (state)
    {
    case State::SEARCHING:
    {
        if (unvisited.empty()) {
            if (find_all) {
                counter += 1;
                printf("%i-th solution found\n", counter);
            }
            else {
                // everything already placed, solved...
                state = State::SOLVED;
                return false;
            }
        }

        CheckFeasible();

        if (best_score <= 0) {
            // impossible to place anything here... backtrack
            backtrack_to = static_cast<int>(visited.size()) - 1;
            if (unvisited.size() < finalizing_threshold &&
                enable_finalizing) {
                state = State::FINALIZING;
                // decrease threshold a bit
                if (finalizing_threshold > 70) {
                    finalizing_threshold -= 1;
                    // TBD debug log...
                }

            }
            else {
                state = State::BACKTRACKING;
            }

            return true;
        }

        Board::BoardLoc* selected_loc = nullptr;
        std::shared_ptr <PieceRef> selected_piece_ref = nullptr;

        // from the set of selected pieces, select one with least number
        // of possibilities(most constrained)
        std::map<int, std::tuple<int, Board::BoardLoc*, std::shared_ptr<PieceRef> > > counts;
        for (auto& loc : best_feasible_locations) {
            for (auto piece_ref : *feasible_pieces[loc->x][loc->y]) {
                std::get<0>(counts[piece_ref->GetId()]) += 1;
                std::get<1>(counts[piece_ref->GetId()]) = loc;
                std::get<2>(counts[piece_ref->GetId()]) = piece_ref;;
            }
        }

        // TBD: this whole selecting of maximum seems obfuscate, simplify...
        int max_count = -1;
        std::shared_ptr<PieceRef> max_ref = nullptr;
        Board::BoardLoc* max_loc = nullptr;
        for (auto& item : counts) {
            int count = std::get<0>(item.second);
            if (max_count == -1 || count > max_count) {
                max_count = count;
                max_loc = std::get<1>(item.second);
                max_ref = std::get<2>(item.second);
            }
        }

        selected_loc = max_loc;
        selected_piece_ref = max_ref;

        Place(selected_loc, selected_piece_ref);
        return true;
    }
    break;
    case State::BACKTRACKING:
    {
        Backtrack();
        if (backtrack_to == visited.size()) {
            backtrack_to = 0;
            backtracked_position = nullptr;
            state = State::SEARCHING;
        }
    }
    break;
    case State::FINALIZING:
    {
        CheckFeasible(true);
        if (best_score == -1) {
            state = State::BACKTRACKING;
        }
        else {
            int most_neighbours = -1;
            Board::BoardLoc* best_location = nullptr;
            std::shared_ptr <PieceRef> best_piece_ref = nullptr;
            for (auto& loc : best_feasible_locations) {
                int count = 0;
                for (int i = 0; i < 4; ++i) {
                    count += (loc->neighbours[i]) ? 1 : 0;
                }
                if (count > most_neighbours) {
                    most_neighbours = count;
                    // TBD: here it seems python gets accidentaly random elements instead?
                    best_piece_ref = *feasible_pieces[loc->x][loc->y]->begin();
                    best_location = loc;
                    if (most_neighbours == 4) {
                        break;
                    }

                }
            }
            Place(best_location, best_piece_ref);
        }
    }
    break;
    case State::SOLVED:
    {
        return false;
    }
    break;
    default:
        break;
    }

    return true;
}

void Backtracker::CheckFeasible(bool ignore_impossible)
{
    best_score = -1;
    best_unplaced_container = nullptr;

    feasible_pieces.clear();
    feasible_pieces.resize(board.GetPuzzleDef()->GetHeight());
    for (auto& row : feasible_pieces) {
        row.resize(board.GetPuzzleDef()->GetWidth());
    }

    std::set< std::shared_ptr<PieceRef> >* possible = nullptr;
    for (auto loc : unvisited) {
        feasible_pieces[loc->x][loc->y] = std::make_unique< std::vector<
            std::shared_ptr<PieceRef> > >();

        if (board.IsCorner(loc->x, loc->y)) {
            possible = &unplaced_corners;
        }
        else if (board.IsEdge(loc->x, loc->y)) {
            possible = &unplaced_edges;
        }
        else {
            possible = &unplaced_inner;
        }

        if ((connecting || board.IsInner(loc->x, loc->y)) && !visited.empty()) {
            // for inner pieces, check if they have any neighbours, otherwise we
            // are wasting time computing those
            int neighbours = 0;
            for (int i = 0; i < 4; ++i) {
                neighbours += (loc->neighbours[i] && loc->neighbours[i]->ref) ? 1 : 0;
            }
            if (neighbours == 0) {
                feasible_pieces[loc->x][loc->y] = nullptr;
                continue;
            }
        }

        for (auto& piece : *possible) {
            if (!board.GetLocations()[piece->GetId()]) {
                bool allowed = true;
                for (auto& item : forbidden[loc]) {
                    auto & forbidden_pieces = item.second;
                    if (forbidden_pieces.find(piece) != forbidden_pieces.end()) {
                        allowed = false;
                        break;
                    }
                }
                if (allowed) {
                    if (CanBePlacedAt(loc, piece)) {
                        feasible_pieces[loc->x][loc->y]->push_back(piece);
                    }
                }
            }
        }
    }

    if (!ignore_impossible && constraint_reducing) {
        throw std::exception("NYI");
        // graph matching algoritmh should go here

    }

    // some commented slow constraint checking here in 
    // original python code

    best_feasible_locations.clear();

    for (auto loc : unvisited) {
        if (!feasible_pieces[loc->x][loc->y]) {
            continue;
        }

        if (board.IsCorner(loc->x, loc->y)) {
            possible = &unplaced_corners;
        }
        else if (board.IsEdge(loc->x, loc->y)) {
            possible = &unplaced_edges;
        }
        else {
            possible = &unplaced_inner;
        }

        int score = static_cast<int>(feasible_pieces[loc->x][loc->y]->size());
        if (score == best_score && best_unplaced_container == possible) {
            best_feasible_locations.push_back(loc);
        }
        else if ((best_score == -1 || score < best_score) &&
            (!ignore_impossible || score > 0)) {
            best_score = score;
            best_feasible_locations.clear();
            best_feasible_locations.push_back(loc);
            best_unplaced_container = possible;
        }

        if (best_score == 0) {
            // impossible to place anything here...
            break;
        }
    }

}

bool Backtracker::CanBePlacedAt(Board::BoardLoc* loc, std::shared_ptr<PieceRef> ref)
{
    if (loc->neighbours[NORTH] && loc->neighbours[NORTH]->ref)
    {
        if (ref->GetPattern(NORTH) !=
            loc->neighbours[NORTH]->ref->GetPattern(SOUTH)) {
            return false;
        }
    }

    if (loc->neighbours[SOUTH] && loc->neighbours[SOUTH]->ref)
    {
        if (ref->GetPattern(SOUTH) !=
            loc->neighbours[SOUTH]->ref->GetPattern(NORTH)) {
            return false;
        }
    }

    if (loc->neighbours[WEST] && loc->neighbours[WEST]->ref)
    {
        if (ref->GetPattern(WEST) !=
            loc->neighbours[WEST]->ref->GetPattern(EAST)) {
            return false;
        }
    }

    if (loc->neighbours[EAST] && loc->neighbours[EAST]->ref)
    {
        if (ref->GetPattern(EAST) !=
            loc->neighbours[EAST]->ref->GetPattern(WEST)) {
            return false;
        }
    }

    // corners/edges special checks
    if (loc->x == 0) {
        if (loc->y == 0) {
            if (ref->GetDir() != EAST) {
                return false;
            }
        }
        else if (loc->y == board.GetPuzzleDef()->GetWidth() - 1) {
            if (ref->GetDir() != SOUTH) {
                return false;
            }
        }
        else {
            if (ref->GetDir() != EAST) {
                return false;
            }
        }
    }
    else if (loc->x == board.GetPuzzleDef()->GetHeight() - 1) {
        if (loc->y == 0) {
            if (ref->GetDir() != NORTH) {
                return false;
            }
        }
        else if (loc->y == board.GetPuzzleDef()->GetWidth() - 1) {
            if (ref->GetDir() != WEST) {
                return false;
            }
        }
        else {
            if (ref->GetDir() != WEST) {
                return false;
            }
        }
    }
    else if (loc->y == 0) {
        if (ref->GetDir() != NORTH) {
            return false;
        }
    }
    else if (loc->y == board.GetPuzzleDef()->GetWidth() - 1) {
        if (ref->GetDir() != SOUTH) {
            return false;
        }
    }

    return true;
}

void Backtracker::Place(Board::BoardLoc* loc, std::shared_ptr<PieceRef> ref)
{
    LDEBUG("Placing %i at (%i, %i) dir=%i stack_size=%i\n",
        ref->GetId(), loc->x, loc->y, ref->GetDir(), static_cast<int>(visited.size()) + 1);
    board.PutPiece(loc, ref);
    best_unplaced_container->erase(ref);
    unvisited.erase(loc);
    visited.push(loc);
    if (backtracked_position && backtracked_position != loc) {
        throw std::exception("Removing backtracking position when placing piece?!?");
    }
    backtracked_position = nullptr;
}

void Backtracker::Backtrack()
{
    Board::BoardLoc* removing = visited.top();
    visited.pop();
    unvisited.insert(removing);
    int stack_pos = static_cast<int>(visited.size());
    LDEBUG("Removing %i from (%i, %i) stack_size=%i\n",
        removing->ref->GetId(), removing->x, removing->y, static_cast<int>(visited.size()));

    forbidden[removing][stack_pos].insert(removing->ref);

    if (board.IsCorner(removing->x, removing->y)) {
        unplaced_corners.insert(removing->ref);
    }
    else if (board.IsEdge(removing->x, removing->y)) {
        unplaced_edges.insert(removing->ref);
    }
    else {
        unplaced_inner.insert(removing->ref);
    }
    board.RemovePiece(removing);

    // remove all forbidden due to later position
    for (auto& pair : forbidden) {
        auto itr = pair.second.begin();
        while (itr != pair.second.end())
        {
            if (itr->first > stack_pos) {
                itr = pair.second.erase(itr);
            }
            else {
                ++itr;
            }
        }
    }

    backtracked_position = removing;
}

// TBD - needs more works, arbitrary precision computations...
int Backtracker::Power(int base, int exponent)
{
    return 1;
}

int Backtracker::GetCounter()
{
    return counter;
}