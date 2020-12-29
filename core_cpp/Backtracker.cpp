#include <sstream>
#include <algorithm>
#include <fstream>
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

    int placeable_corners = 4;
    int placeable_edges = static_cast<int>(board.GetPuzzleDef()->GetEdges().size());
    int placeable_inners = static_cast<int>(board.GetPuzzleDef()->GetInner().size());

    // discount hint pieces as they are not placeable
    for (auto& piece : board.GetPuzzleDef()->GetHints()) {
        switch (board.GetLocation(piece.x, piece.y)->type)
        {
        case Board::LocType::CORNER:
            placeable_corners -= 1;
            break;
        case Board::LocType::EDGE:
            placeable_edges -= 1;
            break;
        case Board::LocType::INNER:
            placeable_inners -= 1;
            break;
        default:
            break;
        }
    }    

    mpz_init(explored_max);
    mpz_set(explored_max, factorial[placeable_corners]);
    mpz_mul(explored_max, explored_max, factorial[placeable_edges]);
    mpz_mul(explored_max, explored_max, factorial[placeable_inners]);
    mpz_t power;
    mpz_init(power);
    mpz_set_ui(power, 4);
    mpz_pow_ui(power, power, placeable_inners);
    mpz_mul(explored_max, explored_max, power);
    mpz_clear(power);

    mpz_init(absLast);
    mpz_set_ui(absLast, 0);

    // unplaced count, needed for explored statistics 
    unplaced_corners_ids_count = placeable_corners;
    unplaced_edges_ids_count = static_cast<int>(placeable_edges);
    unplaced_inner_ids_count = static_cast<int>(placeable_inners);
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

Backtracker::Backtracker(Board& board, std::set<std::pair<int, int>>* pieces_map, bool find_all,
    const std::string& rotations_file)
    : board(board),
    find_all(find_all), connecting(true),
    counter(0), finalizing_threshold(90),
    highest_stack_pos(0)
{
    height = board.GetPuzzleDef()->GetHeight();
    width = board.GetPuzzleDef()->GetWidth();

    for (int x = 0; x < height; ++x) {
        for (int y = 0; y < width; ++y) {
            if (!pieces_map || pieces_map->find(std::pair<int, int>(x, y)) != pieces_map->end()) {
                unvisited.insert(board.GetLocation(x, y));
            }
        }
    }

    if (pieces_map) {
        find_all = true;
    }

    refs.resize(board.GetPuzzleDef()->GetPieceCount() + 1);
    for (int id = 1; id < refs.size(); ++id) {
        const auto& def = board.GetPuzzleDef()->GetPieceDef(id);
        for (int dir = 0; dir < 4; ++dir) {
            refs[id][dir] = std::make_shared<PieceRef>(def, dir);
        } 
    }

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
            unplaced_pieces.insert(refs[piece.second.id][rotations[piece.second.id]]);
        }
        else {
            for (int dir = 0; dir < 4; ++dir) {
                unplaced_pieces.insert(refs[piece.second.id][rotations[piece.second.id]]);
            }
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
        for (auto& ref : unplaced_pieces) {
            if (ref->GetId() == hint.id) {
                to_erase.push_back(ref);
            }
        }
        for (auto& ref : to_erase) {
            unplaced_pieces.erase(ref);
        }

        auto loc = board.GetLocation(hint.x, hint.y);
        unvisited.erase(loc);
        stack.visited.push(Stack::LevelInfo(loc));
        stack.start_size++;
    }

    highest_stack_pos = static_cast<int>(stack.visited.size());
    board.AdjustDirBorder();

    // create fast access structure for finding all pieces matching
    // given list of patterns
    // TBD - following need refactor + fix to work with specific pieces rotation provided
    color_count = board.GetPuzzleDef()->GetColorCount(); 
    color_count += 1; // add 0 as a color
    int any_color = color_count; // make last color encode the "ANY" matching pattern
    for (auto& piece : board.GetPuzzleDef()->GetInner()) 
    {
        for (int dir = 0; dir < 4; ++dir) {
            auto& ref = refs[piece.id][dir];
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);

            neighbour_table[EncodePatterns(any_color, ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), any_color, ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), any_color, ref->GetPattern(NORTH))].push_back(ref);
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), any_color)].push_back(ref);

            neighbour_table[EncodePatterns(any_color, any_color, ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
            neighbour_table[EncodePatterns(any_color, ref->GetPattern(SOUTH), any_color, ref->GetPattern(NORTH))].push_back(ref);
            neighbour_table[EncodePatterns(any_color, ref->GetPattern(SOUTH), ref->GetPattern(WEST), any_color)].push_back(ref);
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), any_color, any_color, ref->GetPattern(NORTH))].push_back(ref);
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), any_color, ref->GetPattern(WEST), any_color)].push_back(ref);
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), any_color, any_color)].push_back(ref);

            neighbour_table[EncodePatterns(any_color, any_color, any_color, ref->GetPattern(NORTH))].push_back(ref);
            neighbour_table[EncodePatterns(any_color, any_color, ref->GetPattern(WEST), any_color)].push_back(ref);
            neighbour_table[EncodePatterns(any_color, ref->GetPattern(SOUTH), any_color, any_color)].push_back(ref);
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), any_color, any_color, any_color)].push_back(ref);

            neighbour_table[EncodePatterns(any_color, any_color, any_color, any_color)].push_back(ref);
        }
    }


    for (auto& piece : board.GetPuzzleDef()->GetCorners()) 
    {
        // corners are of form something, something, 0, 0
        {
            auto& ref = refs[piece.id][0]; // something, something, 0, 0
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);

            neighbour_table[EncodePatterns(any_color, ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), any_color, ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);

            neighbour_table[EncodePatterns(any_color, any_color, ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
        }

        {
            auto& ref = refs[piece.id][1]; // 0, something, something, 0
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);

            neighbour_table[EncodePatterns(ref->GetPattern(EAST), any_color, ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), any_color, ref->GetPattern(NORTH))].push_back(ref);

            neighbour_table[EncodePatterns(ref->GetPattern(EAST), any_color, any_color, ref->GetPattern(NORTH))].push_back(ref);
        }

        {
            auto& ref = refs[piece.id][2]; // 0, 0, something, something
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);

            neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), any_color, ref->GetPattern(NORTH))].push_back(ref);
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), any_color)].push_back(ref);

            neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), any_color, any_color)].push_back(ref);
        }

        {
            auto& ref = refs[piece.id][3]; // something, 0, 0, something
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);

            neighbour_table[EncodePatterns(any_color, ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), any_color)].push_back(ref);

            neighbour_table[EncodePatterns(any_color, ref->GetPattern(SOUTH), ref->GetPattern(WEST), any_color)].push_back(ref);
        }

    }

    for (auto& piece : board.GetPuzzleDef()->GetEdges())
    {
        // edges are of form something, something, something, 0
        {
            auto& ref = refs[piece.id][0]; // something, something, something, 0
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);

            neighbour_table[EncodePatterns(any_color, ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), any_color, ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), any_color, ref->GetPattern(NORTH))].push_back(ref);

            neighbour_table[EncodePatterns(any_color, any_color, ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
            neighbour_table[EncodePatterns(any_color, ref->GetPattern(SOUTH), any_color, ref->GetPattern(NORTH))].push_back(ref);
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), any_color, any_color, ref->GetPattern(NORTH))].push_back(ref);

            neighbour_table[EncodePatterns(any_color, any_color, any_color, ref->GetPattern(NORTH))].push_back(ref);
        }

        {
            auto& ref = refs[piece.id][1]; // 0, something, something, something 
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);

            neighbour_table[EncodePatterns(ref->GetPattern(EAST), any_color, ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), any_color, ref->GetPattern(NORTH))].push_back(ref);
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), any_color)].push_back(ref);

            neighbour_table[EncodePatterns(ref->GetPattern(EAST), any_color, any_color, ref->GetPattern(NORTH))].push_back(ref);
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), any_color, ref->GetPattern(WEST), any_color)].push_back(ref);
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), any_color, any_color)].push_back(ref);

            neighbour_table[EncodePatterns(ref->GetPattern(EAST), any_color, any_color, any_color)].push_back(ref);
        }

        {
            auto& ref = refs[piece.id][2]; // something, 0, something, something 
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);

            neighbour_table[EncodePatterns(any_color, ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), any_color, ref->GetPattern(NORTH))].push_back(ref);
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), any_color)].push_back(ref);

            neighbour_table[EncodePatterns(any_color, ref->GetPattern(SOUTH), any_color, ref->GetPattern(NORTH))].push_back(ref);
            neighbour_table[EncodePatterns(any_color, ref->GetPattern(SOUTH), ref->GetPattern(WEST), any_color)].push_back(ref);
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), any_color, any_color)].push_back(ref);

            neighbour_table[EncodePatterns(any_color, ref->GetPattern(SOUTH), any_color, any_color)].push_back(ref);
        }

        {
            auto& ref = refs[piece.id][3]; // something, something, 0, something 
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);

            neighbour_table[EncodePatterns(any_color, ref->GetPattern(SOUTH), ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), any_color, ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), ref->GetPattern(SOUTH), ref->GetPattern(WEST), any_color)].push_back(ref);

            neighbour_table[EncodePatterns(any_color, any_color, ref->GetPattern(WEST), ref->GetPattern(NORTH))].push_back(ref);
            neighbour_table[EncodePatterns(any_color, ref->GetPattern(SOUTH), ref->GetPattern(WEST), any_color)].push_back(ref);
            neighbour_table[EncodePatterns(ref->GetPattern(EAST), any_color, ref->GetPattern(WEST), any_color)].push_back(ref);

            neighbour_table[EncodePatterns(any_color, any_color, ref->GetPattern(WEST), any_color)].push_back(ref);
        }
    }
}

int Backtracker::EncodePatterns(int east, int south, int west, int north)
{
    int encoded = east;

    encoded *= (color_count + 1);
    encoded += south;

    encoded *= (color_count + 1);
    encoded += west;

    encoded *= (color_count + 1);
    encoded += north;

    return encoded;
}

bool Backtracker::Step()
{
    switch (state)
    {
    case State::SEARCHING:
    {
        if (unvisited.empty()) {
            for (auto& callback : on_solve) {
                callback->Call(board);
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

        std::vector<
            std::unique_ptr< std::vector<
            std::shared_ptr<PieceRef> > > > feasible_pieces;
        std::vector<Board::Loc*> best_feasible_locations;
        int best_score = CheckFeasible(feasible_pieces, best_feasible_locations);

        if (best_score <= 0) {
            // impossible to place anything here... backtrack
            stack.backtrack_to = static_cast<int>(stack.visited.size()) - 1;
            state = State::BACKTRACKING;

            return true;
        }

        // select the piece randomly...
        Board::Loc* selected_loc = best_feasible_locations[rand() % best_feasible_locations.size()];
        auto& feasible = feasible_pieces[height * selected_loc->x + selected_loc->y];
        std::shared_ptr<PieceRef>& selected_piece_ref = feasible->at(rand() % feasible->size());

        Place(selected_loc, selected_piece_ref);
        unplaced_pieces.erase(selected_piece_ref);

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

int Backtracker::CheckFeasible(std::vector<
    std::unique_ptr< std::vector<
    std::shared_ptr<PieceRef> > > >& feasible_pieces, 
    std::vector<Board::Loc*>& best_feasible_locations)
{
    int best_score = -1;

    feasible_pieces.clear();
    feasible_pieces.resize(height * width);
    best_feasible_locations.clear();

    int any_color = color_count; // make last color encode the "ANY" matching pattern
    for (auto loc : unvisited) {
        if ((connecting || loc->type == Board::LocType::INNER) && !stack.IsEmpty()) {
            // for inner pieces, check if they have any neighbours, otherwise we
            // are wasting time computing those
            int neighbours = 0;
            for (int i = 0; i < 4; ++i) {
                neighbours += (loc->neighbours[i] && loc->neighbours[i]->ref) ? 1 : 0;
            }
            if (neighbours == 0) {
                continue;
            }
        }

        feasible_pieces[height * loc->x + loc->y] = std::make_unique< std::vector<
            std::shared_ptr<PieceRef> > >();
        auto& new_feasible = feasible_pieces[height * loc->x + loc->y];

        auto& east_loc = loc->neighbours[EAST];
        auto& south_loc = loc->neighbours[SOUTH];
        auto& west_loc = loc->neighbours[WEST];
        auto& north_loc = loc->neighbours[NORTH];

        int key = EncodePatterns(!east_loc ? 0 : (east_loc->ref ? east_loc->ref->GetPattern(WEST) : any_color),
            !south_loc ? 0 : (south_loc->ref ? south_loc->ref->GetPattern(NORTH) : any_color),
            !west_loc ? 0 : (west_loc->ref ? west_loc->ref->GetPattern(EAST) : any_color),
            !north_loc ? 0 : (north_loc->ref ? north_loc->ref->GetPattern(SOUTH) : any_color));

        auto it = neighbour_table.find(key);
        if (it != neighbour_table.end())
        {
            for (auto& piece : it->second) {
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
                    if (!is_forbidden) {
                        new_feasible->push_back(piece);
                    }
                }

            }
        }

        int score = static_cast<int>(new_feasible->size());
        if (score == 0) {
            // impossible to place anything here, end asap
            return 0;
        }

        if (score == best_score) {
            best_feasible_locations.push_back(loc);
        }
        else if ((best_score == -1 || score < best_score)) {
            best_score = score;
            best_feasible_locations.clear();
            best_feasible_locations.push_back(loc);
        }
    }

    return best_score;
}

void Backtracker::Place(Board::Loc* loc, std::shared_ptr<PieceRef>& ref)
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
    if (stack.visited.size() > highest_stack_pos) {
        highest_stack_pos = static_cast<int>(stack.visited.size());

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

void Backtracker::RegisterOnNewBest(CallbackOnSolve* callback)
{
    on_new_best.push_back(callback);
}

void ColorAxisCounts::Init(const PuzzleDef* def)
{
    for (auto& piece : def->GetAll()) {
        InitColor(piece.second.patterns[0]);
        InitColor(piece.second.patterns[1]);
        InitColor(piece.second.patterns[2]);
        InitColor(piece.second.patterns[3]);
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
