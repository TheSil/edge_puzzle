#include <sstream>
#include <algorithm>
#include "Backtracker.h"

using namespace edge;

Backtracker::Backtracker(Board& board)
    : board(board), best_score(-1), backtrack_to(0),
    backtracked_position(nullptr), find_all(false), connecting(true),
    counter(0), finalizing_threshold(90), enable_finalizing(true),
    constraint_reducing(false),
    best_unplaced_container(nullptr), best_unplaced_container_ids(nullptr),
    explored_max(0)
{
    fact.resize(4 * board.GetPuzzleDef()->GetPieceCount() + 1);
    for (int i = 0; i < fact.size(); ++i) {
        fact[i] = 0; // TBD actual factorial values
    }

    for (int x = 0; x < board.GetPuzzleDef()->GetHeight(); ++x) {
        for (int y = 0; y < board.GetPuzzleDef()->GetWidth(); ++y) {
            // TBD some logic with pieces_map here in python code
            unvisited.insert(board.GetLocation(x, y));
        }
    }

    for (auto& piece : board.GetPuzzleDef()->GetCorners()) {
        for (int dir = 0; dir < 4; ++dir) {
            unplaced_corners.insert(
                std::shared_ptr<PieceRef>(new PieceRef(piece, dir)));
        }
        unplaced_corners_ids.insert(piece.id);
    }

    for (auto& piece : board.GetPuzzleDef()->GetEdges()) {
        for (int dir = 0; dir < 4; ++dir) {
            unplaced_edges.insert(
                std::shared_ptr<PieceRef>(new PieceRef(piece, dir)));
        }
        unplaced_edges_ids.insert(piece.id);
    }

    for (auto& piece : board.GetPuzzleDef()->GetInner()) {
        for (int dir = 0; dir < 4; ++dir) {
            unplaced_inner.insert(
                std::shared_ptr<PieceRef>(new PieceRef(piece, dir)));
        }
        unplaced_inner_ids.insert(piece.id);
    }

    for (int x = 0; x < board.GetPuzzleDef()->GetHeight(); ++x) {
        for (int y = 0; y < board.GetPuzzleDef()->GetWidth(); ++y) {
            forbidden[board.GetLocation(x, y)].clear();
        }
    }

    explored.resize(board.GetPuzzleDef()->GetPieceCount(), 0);
    explored_max = fact[4] *
        fact[board.GetPuzzleDef()->GetEdges().size()] *
        fact[board.GetPuzzleDef()->GetInner().size()] *
        (Power(4, (int)board.GetPuzzleDef()->GetInner().size()));

    // hints
    if (0) {
    }

    // grid file
    if (0) {
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
        if (!grid_scores.empty())
        {
            throw std::exception("NYI");
        }
        else {
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
        }

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
        feasible_pieces[loc->x][loc->y] = std::unique_ptr< std::set<
            std::shared_ptr<PieceRef> > >(new std::set<
                std::shared_ptr<PieceRef> >);

        if (board.IsCorner(loc->x, loc->y)) {
            possible = &unplaced_corners;
        }
        else if (board.IsEdge(loc->x, loc->y)) {
            possible = &unplaced_edges;
        }
        else {
            possible = &unplaced_inner;
        }

        if (connecting && !visited.empty() ||
            board.IsInner(loc->x, loc->y)) {
            // for inner pieces, check if they have any neighbours, otherwise we
            // are wasting time computing those
            int neighbours = 0;
            for (int i = 0; i < 4; ++i) {
                neighbours += (loc->neighbours[i]) ? 1 : 0;
            }
            if (neighbours == 0) {
                feasible_pieces[loc->x][loc->y] = nullptr;
                continue;
            }
        }

        feasible_possibilities = *possible;
        for (auto& item : forbidden[loc]) {
            std::set< std::shared_ptr<PieceRef> > result;
            std::set_difference(feasible_possibilities.begin(),
                feasible_possibilities.end(),
                item.second.begin(),
                item.second.end(),
                std::inserter(result, result.end()));
            result.swap(feasible_possibilities);
        }

        for (auto& item : feasible_possibilities) {
            if (placed_ids.find(item->GetId()) == placed_ids.end()) {
                if (CanBePlacedAt(loc, item)) {
                    feasible_pieces[loc->x][loc->y]->insert(item);
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

        std::set<int>* possible_ids = nullptr;
        for (auto loc : unvisited) {
            if (board.IsCorner(loc->x, loc->y)) {
                possible = &unplaced_corners;
                possible_ids = &unplaced_corners_ids;
            }
            else if (board.IsEdge(loc->x, loc->y)) {
                possible = &unplaced_edges;
                possible_ids = &unplaced_edges_ids;
            }
            else {
                possible = &unplaced_inner;
                possible_ids = &unplaced_inner_ids;
            }

            if (!feasible_pieces[loc->x][loc->y]) {
                continue;
            }

            int score = static_cast<int>(feasible_pieces[loc->x][loc->y]->size());
            if (score == best_score && best_unplaced_container == possible) {
                best_feasible_locations.push_back(loc);
            }
            else if ((best_score == -1 || score < best_score) &&
                (!ignore_impossible || score > 0)) {
                best_score = score;
                //auto p = *feasible_pieces[loc->x][loc->y]->begin();
                best_feasible_locations.clear();
                best_feasible_locations.push_back(loc);
                best_unplaced_container = possible;
                best_unplaced_container_ids = possible_ids;
            }

            if (best_score == 0) {
                // impossible to place anything here...
                break;
            }
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
    best_unplaced_container_ids->erase(ref->GetId());
    unvisited.erase(loc);
    visited.push(loc);
    if (backtracked_position && backtracked_position != loc) {
        throw std::exception("Removing backtracking position when placing piece?!?");
    }
    backtracked_position = nullptr;
    placed_ids.insert(ref->GetId());
}

void Backtracker::Backtrack()
{
    Board::BoardLoc* removing = visited.top();
    visited.pop();
    unvisited.insert(removing);
    int stack_pos = static_cast<int>(visited.size());
    LDEBUG("Removing %i from (%i, %i) stack_size=%i\n",
        removing->ref->GetId(), removing->x, removing->y, static_cast<int>(visited.size()));

    // updated explored statistics
    explored[stack_pos] += fact[unplaced_corners_ids.size()] *
        fact[unplaced_edges_ids.size()] *
        fact[unplaced_inner_ids.size()] *
        Power(4, (int)unplaced_inner_ids.size());
    explored[stack_pos + 1] = 0;

    forbidden[removing][stack_pos].insert(removing->ref);

    if (board.IsCorner(removing->x, removing->y)) {
        unplaced_corners.insert(removing->ref);
        unplaced_corners_ids.insert(removing->ref->GetId());
    }
    else if (board.IsEdge(removing->x, removing->y)) {
        unplaced_edges.insert(removing->ref);
        unplaced_edges_ids.insert(removing->ref->GetId());
    }
    else {
        unplaced_inner.insert(removing->ref);
        unplaced_inner_ids.insert(removing->ref->GetId());
    }
    placed_ids.erase(removing->ref->GetId());
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