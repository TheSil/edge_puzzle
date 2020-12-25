#include <sstream>
#include <algorithm>
#include "Backtracker.h"

using namespace edge::backtracker;

void Stats::Init(Board& board)
{
    // pre-calculated factorials, 4 times for each rotation
    factorial.resize(4 * board.GetPuzzleDef()->GetPieceCount() + 1);
    factorial[0] = new mpz_t;
    mpz_init(factorial[0]);
    mpz_set_ui(factorial[0], 1);
    for (int i = 1; i < factorial.size(); ++i) {
        factorial[i] = new mpz_t;
        mpz_init(factorial[i]);
        mpz_mul_ui(factorial[i], factorial[i - 1], i);
    }

    explored.resize(board.GetPuzzleDef()->GetPieceCount());
    for (auto& val : explored) {
        val = new mpz_t;
        mpz_init(val);
        mpz_set_ui(val, 0);
    }

    mpz_init(explored_max);
    mpz_set(explored_max, factorial[4]);
    mpz_mul(explored_max, explored_max, factorial[board.GetPuzzleDef()->GetEdges().size()]);
    mpz_mul(explored_max, explored_max, factorial[board.GetPuzzleDef()->GetInner().size()]);
    mpz_t power;
    mpz_init(power);
    mpz_set_ui(power, 4);
    mpz_pow_ui(power, power, board.GetPuzzleDef()->GetInner().size());
    mpz_mul(explored_max, explored_max, power);
    mpz_clear(power);

    mpz_init(absLast);
    mpz_set_ui(absLast, 0);

    // unplaced count, needed for explored statistics 
    unplaced_corners_ids_count = 4;
    unplaced_edges_ids_count = static_cast<int>(board.GetPuzzleDef()->GetEdges().size());
    unplaced_inner_ids_count = static_cast<int>(board.GetPuzzleDef()->GetInner().size());
}

void Stats::Update(int stack_pos)
{
    mpz_t val;
    mpz_init(val);
    mpz_set_ui(val, 1);
    mpz_mul(val,
        val, factorial[unplaced_corners_ids_count]);
    mpz_mul(val,
        val, factorial[unplaced_edges_ids_count]);
    mpz_mul(val,
        val, factorial[unplaced_inner_ids_count]);
    mpz_t power;
    mpz_init(power);
    mpz_set_ui(power, 4);
    mpz_pow_ui(power, power, unplaced_inner_ids_count);
    mpz_mul(val,
        val, power);
    mpz_add(explored[stack_pos - 1],
        explored[stack_pos - 1], val);
    mpz_clear(val);
    mpz_clear(power);

    if (explored.size() > stack_pos) {
        mpz_set_ui(explored[stack_pos], 0);
    }
}


MpfWrapper Stats::GetExploredAbs()
{
    // sum 
    mpz_t sum;
    mpz_init(sum);
    mpz_set_ui(sum, 0);
    for (auto& val : explored) {
        mpz_add(sum, sum, val);
    }

    mpf_t tmp_float;
    mpf_init(tmp_float);
    mpf_set_z(tmp_float, sum);

    MpfWrapper ret(tmp_float);

    mpz_clear(sum);
    mpf_clear(tmp_float);

    return ret;
}

MpfWrapper Stats::GetExploredAbsLast()
{
    // sum 
    mpz_t sum;
    mpz_init(sum);
    mpz_set_ui(sum, 0);
    for (auto& val : explored) {
        mpz_add(sum, sum, val);
    }

    mpz_t diff;
    mpz_init(diff);
    mpz_sub(diff, sum, absLast);

    // persist this call as a last for next evaluation
    mpz_set(absLast, sum);
    mpz_clear(sum);

    mpf_t tmp_float;
    mpf_init(tmp_float);
    mpf_set_z(tmp_float, diff);

    MpfWrapper ret(tmp_float);

    mpz_clear(diff);
    mpf_clear(tmp_float);

    return ret;
}

MpfWrapper Stats::GetExploredMax()
{
    mpf_t tmp_float;
    mpf_init(tmp_float);
    mpf_set_z(tmp_float, explored_max);

    MpfWrapper ret(tmp_float);

    mpf_clear(tmp_float);

    return ret;
}

MpfWrapper Stats::GetExploredRatio()
{
    // sum 
    mpz_t sum;
    mpz_init(sum);
    mpz_set_ui(sum, 0);
    for (auto& val : explored) {
        mpz_add(sum, sum, val);
    }

    mpf_t tmp_float1, tmp_float2;
    mpf_init(tmp_float1);
    mpf_set_z(tmp_float1, sum);
    mpf_init(tmp_float2);
    mpf_set_z(tmp_float2, explored_max);
    mpf_div(tmp_float1, tmp_float1, tmp_float2);
    //char buf[128];
    //gmp_sprintf(buf, "%.2FE", tmp_float1);
    //val = buf;

    MpfWrapper ret(tmp_float1);

    mpf_clear(tmp_float1);
    mpf_clear(tmp_float2);
    mpz_clear(sum);

    return ret;
}

void Stats::UpdateUnplacedCorners(int amount)
{
    unplaced_corners_ids_count += amount;
}

void Stats::UpdateUnplacedEdges(int amount)
{
    unplaced_edges_ids_count += amount;
}

void Stats::UpdateUnplacedInner(int amount)
{
    unplaced_inner_ids_count += amount;
}

Backtracker::Backtracker(Board& board, std::set<std::pair<int, int>>* pieces_map, bool find_all)
    : board(board), best_score(-1),
    find_all(find_all), connecting(true),
    counter(0), finalizing_threshold(90), enable_finalizing(false),
    constraint_reducing(false)
{
    for (int x = 0; x < board.GetPuzzleDef()->GetHeight(); ++x) {
        for (int y = 0; y < board.GetPuzzleDef()->GetWidth(); ++y) {
            if (!pieces_map || pieces_map->find(std::pair<int, int>(x, y)) != pieces_map->end()) {
                unvisited.insert(board.GetLocation(x, y));
            }
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

    stats.Init(board);
    rotChecker.Init(board.GetPuzzleDef());

    // hints
    for (auto& hint : board.GetPuzzleDef()->GetHints()) {
        int dir = (hint.dir != -1) ? hint.dir : 0;
        board.PutPiece(hint.id, hint.x, hint.y, dir);
        rotChecker.Place(board.GetLocation(hint.x, hint.y)->ref->GetPattern(0),
            board.GetLocation(hint.x, hint.y)->ref->GetPattern(1), 
            board.GetLocation(hint.x, hint.y)->ref->GetPattern(2), 
            board.GetLocation(hint.x, hint.y)->ref->GetPattern(3));

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

        if (loc->type == Board::LocType::CORNER) {
            stats.UpdateUnplacedCorners(-1);
        }
        else if (loc->type == Board::LocType::EDGE) {
            stats.UpdateUnplacedEdges(-1);
        }
        else {
            stats.UpdateUnplacedInner(-1);
        }

        unvisited.erase(loc);
        stack.visited.push(Stack::LevelInfo(loc));
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
            for (auto& callback : on_solve) {
                callback->call(board);
            }

            if (find_all) {
                counter += 1;
                printf("%i-th solution found\n", counter);
            }
            else {
                // everything already placed, solved...
                state = State::FINISHED;
                return false;
            }
        }

        std::vector<std::vector<
            std::unique_ptr< std::vector<
            std::shared_ptr<PieceRef> > > > > feasible_pieces;
        std::vector<Board::Loc*> best_feasible_locations;
        std::set< std::shared_ptr<PieceRef> >* best_unplaced_container;
        CheckFeasible(feasible_pieces, best_feasible_locations, best_unplaced_container);

        if (best_score <= 0) {
            // impossible to place anything here... backtrack
            stack.backtrack_to = static_cast<int>(stack.visited.size()) - 1;
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

        Board::Loc* selected_loc = nullptr;
        std::shared_ptr <PieceRef> selected_piece_ref = nullptr;

        // from the set of selected pieces, select one with least number
        // of possibilities(most constrained)
        std::map<int, std::tuple<int, Board::Loc*, std::shared_ptr<PieceRef> > > counts;
        random_shuffle(best_feasible_locations.begin(), best_feasible_locations.end());
        for (auto& loc : best_feasible_locations) {
            random_shuffle(feasible_pieces[loc->x][loc->y]->begin(), feasible_pieces[loc->x][loc->y]->end());
            for (auto piece_ref : *feasible_pieces[loc->x][loc->y]) {
                int id = piece_ref->GetId();
                std::get<0>(counts[id]) += 1;
                std::get<1>(counts[id]) = loc;
                std::get<2>(counts[id]) = piece_ref;
            }
        }

        // TBD: this whole selecting of maximum seems obfuscate, simplify...
        int max_count = -1;
        std::shared_ptr<PieceRef> max_ref = nullptr;
        Board::Loc* max_loc = nullptr;
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
        best_unplaced_container->erase(selected_piece_ref);

        // if inconsistent rotations, backtrack...
        if (!rotChecker.CanBeFinished(selected_piece_ref->GetPattern(0)) ||
            !rotChecker.CanBeFinished(selected_piece_ref->GetPattern(1)) ||
            !rotChecker.CanBeFinished(selected_piece_ref->GetPattern(2)) ||
            !rotChecker.CanBeFinished(selected_piece_ref->GetPattern(3)) ) {
            LDEBUG("Inconsistent rotation, initating backtrack...\n");
            stack.backtrack_to = static_cast<int>(stack.visited.size()) - 1;
            state = State::BACKTRACKING;
        }

        return true;
    }
    break;
    case State::BACKTRACKING:
    {
        if (Backtrack()) {
            if (stack.backtrack_to == stack.visited.size()) {
                stack.backtrack_to = 0;
                state = State::SEARCHING;
            }
        }
        else {
            state = State::FINISHED;
        }
    }
    break;
    case State::FINALIZING:
    {
        std::vector<std::vector<
            std::unique_ptr< std::vector<
            std::shared_ptr<PieceRef> > > > > feasible_pieces;
        std::vector<Board::Loc*> best_feasible_locations;
        std::set< std::shared_ptr<PieceRef> >* best_unplaced_container = nullptr;

        CheckFeasible(feasible_pieces, best_feasible_locations, best_unplaced_container, true);
        if (best_score == -1) {
            state = State::BACKTRACKING;
        }
        else {
            int most_neighbours = -1;
            Board::Loc* best_location = nullptr;
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
            best_unplaced_container->erase(best_piece_ref);
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

void Backtracker::CheckFeasible(std::vector<std::vector<
    std::unique_ptr< std::vector<
    std::shared_ptr<PieceRef> > > > >& feasible_pieces, 
    std::vector<Board::Loc*>& best_feasible_locations,
    std::set< std::shared_ptr<PieceRef> >*& best_unplaced_container,
    bool ignore_impossible)
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

        if (loc->type == Board::LocType::CORNER) {
            possible = &unplaced_corners;
        }
        else if (loc->type == Board::LocType::EDGE) {
            possible = &unplaced_edges;
        }
        else {
            possible = &unplaced_inner;
        }

        if ((connecting || loc->type == Board::LocType::INNER) && !stack.IsEmpty()) {
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
                auto& forbidden_map = stack.visited.top().forbidden;
                auto it = forbidden_map.find(loc);
                bool is_forbidden = false;
                if (it != forbidden_map.end())
                {
                    if (it->second.find(piece) != it->second.end()) {
                        is_forbidden = true;
                    }
                }                
                if (!is_forbidden && CanBePlacedAt(loc, piece)) {
                    feasible_pieces[loc->x][loc->y]->push_back(piece);
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

        if (loc->type == Board::LocType::CORNER) {
            possible = &unplaced_corners;
        }
        else if (loc->type == Board::LocType::EDGE) {
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

bool Backtracker::CanBePlacedAt(Board::Loc* loc, std::shared_ptr<PieceRef> ref)
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

void Backtracker::Place(Board::Loc* loc, std::shared_ptr<PieceRef> ref)
{
    LDEBUG("Placing %i at (%i, %i) [%i, %i, %i, %i]  dir=%i stack_size=%i\n",
        ref->GetId(), loc->x, loc->y, 
        ref->GetPattern(0), ref->GetPattern(1), ref->GetPattern(2), ref->GetPattern(3),
        ref->GetDir(), static_cast<int>(stack.visited.size()) + 1);
    board.PutPiece(loc, ref);
    rotChecker.Place(ref->GetPattern(0),
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

    unvisited.erase(loc);
    stack.visited.push(loc);
}

bool Backtracker::Backtrack()
{
    if (stack.IsEmpty()) {
        return false;
    }

    Board::Loc* removing = stack.visited.top().loc;
    stack.visited.pop();
    unvisited.insert(removing);
    int stack_pos = static_cast<int>(stack.visited.size());

    // update statistics
    stats.Update(stack_pos);

    LDEBUG("Removing %i from (%i, %i) [%i, %i, %i, %i] stack_size=%i\n",
        removing->ref->GetId(), 
        removing->x, removing->y, 
        removing->ref->GetPattern(0), removing->ref->GetPattern(1), removing->ref->GetPattern(2), removing->ref->GetPattern(3),
        static_cast<int>(stack.visited.size()));

    stack.visited.top().forbidden[removing].insert(removing->ref);

    if (removing->type == Board::LocType::CORNER) {
        unplaced_corners.insert(removing->ref);
        stats.UpdateUnplacedCorners(1);
    }
    else if (removing->type == Board::LocType::EDGE) {
        unplaced_edges.insert(removing->ref);
        stats.UpdateUnplacedEdges(1);
    }
    else {
        unplaced_inner.insert(removing->ref);
        stats.UpdateUnplacedInner(1);
    }
    rotChecker.Unplace(removing->ref->GetPattern(0),
        removing->ref->GetPattern(1),
        removing->ref->GetPattern(2),
        removing->ref->GetPattern(3));
    board.RemovePiece(removing);

    return true;
}

int Backtracker::GetCounter()
{
    return counter;
}

Stats& Backtracker::GetStats()
{
    return stats;
}

void Backtracker::RegisterOnSolve(CallbackOnSolve* callback)
{
    on_solve.push_back(callback);
}

void ColorAxisCounts::Init(const PuzzleDef* def)
{
    for (auto& piece : def->GetCorners()) {
        InitColor(piece.patterns[0]);
        InitColor(piece.patterns[1]);
        InitColor(piece.patterns[2]);
        InitColor(piece.patterns[3]);
    }

    for (auto& piece : def->GetEdges()) {
        InitColor(piece.patterns[0]);
        InitColor(piece.patterns[1]);
        InitColor(piece.patterns[2]);
        InitColor(piece.patterns[3]);
    }

    for (auto& piece : def->GetInner()) {
        InitColor(piece.patterns[0]);
        InitColor(piece.patterns[1]);
        InitColor(piece.patterns[2]);
        InitColor(piece.patterns[3]);
    }
}

void ColorAxisCounts::Place(int k0, int k1, int k2, int k3)
{
    colors_horizontal[k0] += 1;
    colors_horizontal[k2] -= 1;
    colors_vertical[k1] += 1;
    colors_vertical[k3] -= 1;
    colors_available[k0] -= 1;
    colors_available[k1] -= 1;
    colors_available[k2] -= 1;
    colors_available[k3] -= 1;
}

void ColorAxisCounts::Unplace(int k0, int k1, int k2, int k3)
{
    colors_available[k0] += 1;
    colors_available[k1] += 1;
    colors_available[k2] += 1;
    colors_available[k3] += 1;
    colors_horizontal[k0] -= 1;
    colors_horizontal[k2] += 1;
    colors_vertical[k1] -= 1;
    colors_vertical[k3] += 1;
}

bool ColorAxisCounts::CanBeFinished(int k)
{
    return colors_available[k] >= ABS(colors_horizontal[k]) + ABS(colors_vertical[k]);
}

void ColorAxisCounts::InitColor(int k)
{
    if (k >= colors_available.size()) {
        colors_available.resize(k + 1);
        colors_horizontal.resize(k + 1);
        colors_vertical.resize(k + 1);
    }
    colors_available[k] += 1;
}
