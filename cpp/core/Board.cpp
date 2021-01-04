#include <algorithm>
#include <fstream>
#include <memory>
#include "Board.h"

using namespace edge;

template <int FIRST, int SECOND>
int ScoreBetween(const Board::Loc* loc) {
    auto second = loc->neighbours[FIRST];
    if (!loc || !loc->ref || !second || !second->ref) return 0;
    return (loc->ref->GetPattern(FIRST) == second->ref->GetPattern(SECOND)) ? 1 : 0;
}

Board::Loc::Loc() : ref(nullptr), hint(nullptr), x(0), y(0), type(Board::LocType::UNKNOWN)
{
    // need to be relinked
    neighbours[0] = 0;
    neighbours[1] = 0;
    neighbours[2] = 0;
    neighbours[3] = 0;
}

Board::Loc::Loc(const Loc& other) : ref(other.ref), hint(other.hint), x(other.x), y(other.y), type(other.type)
{
    // neighbours are ignored, it is always empty during copy/assignment, must be relinked
    neighbours[0] = 0;
    neighbours[1] = 0;
    neighbours[2] = 0;
    neighbours[3] = 0;
}

Board::Loc& Board::Loc::operator= (const Board::Loc& other)
{
    // neighbours are ignored, it is always empty during copy/assignment, must be relinked
    if (this != &other)
    {
        ref = other.ref;
        hint = other.hint;
        x = other.x;
        y = other.y;
        type = other.type;

        neighbours[0] = 0;
        neighbours[1] = 0;
        neighbours[2] = 0;
        neighbours[3] = 0;
    }

    return *this;
}

Board::Board(const PuzzleDef* def)
    : def(def)
{
    state.refs.resize(def->GetPieceCount() + 1);
    for (int id = 1; id < state.refs.size(); ++id) {
        const auto& piece_def = def->GetPieceDef(id);
        for (int dir = 0; dir < 4; ++dir) {
            state.refs[id][dir] = std::make_unique<PieceRef>(piece_def, dir);
        }
    }

    state.board.resize(def->GetHeight());
    for (size_t x = 0; x < state.board.size(); ++x)
    {
        state.board[x].resize(def->GetWidth());
        auto& row = state.board[x];
        for (size_t y = 0; y < row.size(); ++y) {
            row[y].x = static_cast<int>(x);
            row[y].y = static_cast<int>(y);
        }
    }

    for (const auto& piece : def->GetCorners()) {
        corners_ids.push_back(piece.id);
    }

    for (const auto& piece : def->GetEdges()) {
        edges_ids.push_back(piece.id);
    }

    for (const auto& piece : def->GetInner()) {
        inner_ids.push_back(piece.id);
    }

    for (auto& hint : def->GetHints()) {
        state.board[hint.x][hint.y].hint = &hint;
    }

    corners.clear();
    edges.clear();
    inner.clear();
    top_edges.clear();
    bottom_edges.clear();
    left_edges.clear();
    right_edges.clear();

    corners.push_back(std::pair<int, int>(0, 0));
    corners.push_back(std::pair<int, int>(0, def->GetWidth() - 1));
    corners.push_back(std::pair<int, int>(def->GetHeight() - 1, 0));
    corners.push_back(std::pair<int, int>(def->GetHeight() - 1, def->GetWidth() - 1));

    state.board[0][0].type = LocType::CORNER;
    state.board[0][def->GetWidth() - 1].type = LocType::CORNER;
    state.board[def->GetHeight() - 1][0].type = LocType::CORNER;
    state.board[def->GetHeight() - 1][def->GetWidth() - 1].type = LocType::CORNER;

    for (int k = 1; k < def->GetWidth() - 1; ++k) {
        top_edges.push_back(std::pair<int, int>(0, k));
        bottom_edges.push_back(std::pair<int, int>(def->GetHeight() - 1, k));
        edges.push_back(std::pair<int, int>(0, k));
        edges.push_back(std::pair<int, int>(def->GetHeight() - 1, k));
        state.board[0][k].type = LocType::EDGE;
        state.board[def->GetHeight() - 1][k].type = LocType::EDGE;
    }
    for (int k = 1; k < def->GetHeight() - 1; ++k) {
        left_edges.push_back(std::pair<int, int>(k, 0));
        right_edges.push_back(std::pair<int, int>(k, def->GetWidth() - 1));
        edges.push_back(std::pair<int, int>(k, 0));
        edges.push_back(std::pair<int, int>(k, def->GetWidth() - 1));
        state.board[k][0].type = LocType::EDGE;
        state.board[k][def->GetWidth() - 1].type = LocType::EDGE;
    }

    for (int i = 1; i < def->GetHeight() - 1; ++i) {
        for (int j = 1; j < def->GetWidth() - 1; ++j) {
            inner.push_back(std::pair<int, int>(i, j));
            state.board[i][j].type = LocType::INNER;
        }
    }

    UpdateLinks();
    UpdateIds();
}

void Board::Save(const std::string& filename)
{
    std::ofstream file(filename);
    for (int x = 0; x < def->GetHeight(); ++x) {
        for (int y = 0; y < def->GetWidth(); ++y) {
            if (state.board[x][y].ref) {
                file << x << "," 
                    << y << "," 
                    << state.board[x][y].ref->GetId() << "," 
                    << state.board[x][y].ref->GetDir()
                    << std::endl;
            }
        }
    }
}

void Board::Load(const std::string& filename)
{
    std::ifstream file(filename);
    std::string line;
    std::vector<int> vals;

    while (getline(file, line)) {
        vals.clear();
        ParseNumberLine(line, vals);
        vals.resize(4, 0);
        PutPiece(vals[2], vals[0], vals[1], vals[3]);
    }
}

Board::State Board::Backup()
{
    return State(this->state);
}

void Board::Restore(State& state)
{
    this->state = state;
    UpdateLinks();
    UpdateIds();
}

void Board::UpdateLinks()
{
    for (int i = 0; i < def->GetHeight(); ++i) {
        for (int j = 0; j < def->GetWidth(); ++j) {
            state.board[i][j].neighbours[NORTH] = (i > 0) ? &state.board[i - 1][j] : nullptr;
            state.board[i][j].neighbours[SOUTH] = (i < def->GetHeight() - 1) ? &state.board[i + 1][j] : nullptr;
            state.board[i][j].neighbours[WEST] = (j > 0) ? &state.board[i][j - 1] : nullptr;
            state.board[i][j].neighbours[EAST] = (j < def->GetWidth() - 1) ? &state.board[i][j + 1] : nullptr;
        }
    }
}

void Board::UpdateIds()
{
    auto pieces_count = def->GetPieceCount();
    state.locations_per_id.resize(pieces_count + 1); // id's are one indexed

    for (int i = 0; i < def->GetHeight(); ++i) {
        for (int j = 0; j < def->GetWidth(); ++j) {
            if (state.board[i][j].ref)
            {
                state.locations_per_id[state.board[i][j].ref->GetId()] = &state.board[i][j];
            }
        }
    }
}

void Board::Randomize()
{
    auto corners_copy = def->GetCorners();
    auto edges_copy = def->GetEdges();
    auto inner_copy = def->GetInner();

    std::random_shuffle(corners_copy.begin(), corners_copy.end());
    std::random_shuffle(edges_copy.begin(), edges_copy.end());
    std::random_shuffle(inner_copy.begin(), inner_copy.end());

    auto corners_it = corners_copy.begin();
    auto edges_it = edges_copy.begin();
    auto inner_it = inner_copy.begin();

    state.board[0][0].ref = GetRef((corners_it++)->id, EAST);
    state.board[0][def->GetWidth() - 1].ref = GetRef((corners_it++)->id, SOUTH);
    state.board[def->GetHeight() - 1][0].ref = GetRef((corners_it++)->id, NORTH);
    state.board[def->GetHeight() - 1][def->GetWidth() - 1].ref = GetRef((corners_it++)->id, WEST);

    for (auto dest : top_edges) {
        state.board[dest.first][dest.second].ref = GetRef((edges_it++)->id, EAST);
    }
    for (auto dest : bottom_edges) {
        state.board[dest.first][dest.second].ref = GetRef((edges_it++)->id, WEST);
    }
    for (auto dest : left_edges) {
        state.board[dest.first][dest.second].ref = GetRef((edges_it++)->id, NORTH);
    }
    for (auto dest : right_edges) {
        state.board[dest.first][dest.second].ref = GetRef((edges_it++)->id, SOUTH);
    }
    for (auto dest : inner) {
        state.board[dest.first][dest.second].ref = GetRef((inner_it++)->id, rand() % 4);
    }

    UpdateIds();

    for (int x = 0; x < def->GetHeight(); ++x) {
        for (int y = 0; y < def->GetWidth(); ++y) {
            auto& loc = state.board[x][y];
            if (loc.hint) {
                int hint_id = loc.hint->id;
                auto& current_hint_loc = state.locations_per_id[hint_id];
                SwapLocations(current_hint_loc, &loc);
                if (loc.hint->dir != -1) {
                    ChangeDir(&loc, loc.hint->dir);
                }
            }
        }
    }
}

void Board::AdjustDirBorderSafe(Board::Loc* loc, int dir)
{
    if (loc->ref) ChangeDir(loc, dir);
}

void Board::AdjustDirBorder()
{
    AdjustDirBorderSafe(&state.board[0][0], EAST);
    AdjustDirBorderSafe(&state.board[0][def->GetWidth() - 1], SOUTH);
    AdjustDirBorderSafe(&state.board[def->GetHeight() - 1][0], NORTH);
    AdjustDirBorderSafe(&state.board[def->GetHeight() - 1][def->GetWidth() - 1], WEST);

    for (auto dest : top_edges) {
        AdjustDirBorderSafe(&state.board[dest.first][dest.second],EAST);
    }
    for (auto dest : bottom_edges) {
        AdjustDirBorderSafe(&state.board[dest.first][dest.second],WEST);
    }
    for (auto dest : left_edges) {
        AdjustDirBorderSafe(&state.board[dest.first][dest.second],NORTH);
    }
    for (auto dest : right_edges) {
        AdjustDirBorderSafe(&state.board[dest.first][dest.second],SOUTH);
    }
}

void Board::AdjustDirBorderSingle(Board::Loc* loc)
{
    if ((loc->x == 0) && (loc->y == 0))  AdjustDirBorderSafe(loc, EAST);
    if ((loc->x == 0) && (loc->y == def->GetWidth() - 1))  AdjustDirBorderSafe(loc, SOUTH);
    if ((loc->x == def->GetHeight() - 1) && (loc->y == 0))  AdjustDirBorderSafe(loc, NORTH);
    if ((loc->x == def->GetHeight() - 1) && (loc->y == def->GetWidth() - 1))  AdjustDirBorderSafe(loc, WEST);
    if (loc->x == 0 && (0 != loc->y) && (loc->y != def->GetWidth() - 1)) AdjustDirBorderSafe(loc, EAST);
    if (loc->x == def->GetHeight() - 1 && (0 != loc->y) && (loc->y != def->GetWidth() - 1)) AdjustDirBorderSafe(loc, WEST);
    if (loc->y == 0 && (0 != loc->x) && (loc->x != def->GetHeight() - 1)) AdjustDirBorderSafe(loc, NORTH);
    if (loc->y == def->GetWidth() - 1 && (0 != loc->x) && (loc->x != def->GetHeight() - 1)) AdjustDirBorderSafe(loc, SOUTH);
}

void Board::AdjustDirInner()
{
    bool did_change = true;
    int k = 0;
    while (did_change) {
        did_change = false;
        for (auto dest : inner) {
            if (AdjustDirInner(&state.board[dest.first][dest.second]))
            {
                did_change = true;
            }
        }
        k += 1;
    }
}

void Board::PutPiece(int id, int x, int y, int dir)
{
    if (state.locations_per_id[id]) {
        // alrady placed, swap positions
        SwapLocations(state.locations_per_id[id], &state.board[x][y]);
        ChangeDir(&state.board[x][y], dir);
    } else {
        state.board[x][y].ref = GetRef(id, dir);
    }
    state.locations_per_id[id] = &state.board[x][y];
}

void Board::PutPiece(Board::Loc* loc, PieceRef* ref)
{
    if (state.locations_per_id[ref->GetId()]) {
        // alrady placed, swap positions
        SwapLocations(state.locations_per_id[ref->GetId()], loc);
        ChangeDir(loc, ref->GetDir());
    }
    else {
        loc->ref = ref;
    }
    state.locations_per_id[ref->GetId()] = loc;
}

void Board::RemovePiece(Board::Loc* loc)
{
    state.locations_per_id[loc->ref->GetId()] = nullptr;
    loc->ref = nullptr;
}

const PuzzleDef* Board::GetPuzzleDef() const
{
    return def;
}

int Board::GetScore() const
{
    int score = 0;
    for (int x = 0; x < def->GetHeight(); ++x) {
        for (int y = 0; y < def->GetWidth() - 1; ++y) {
            score += ScoreBetween<EAST, WEST>(&state.board[x][y]);
        }
    }
    for (int x = 0; x < def->GetHeight() - 1; ++x) {
        for (int y = 0; y < def->GetWidth(); ++y) {
            score += ScoreBetween<SOUTH, NORTH>(&state.board[x][y]);
        }
    }
    return score;
}

int Board::GetScore(Loc* loc)
{
    int score = 0;
    score += ScoreBetween<EAST, WEST>(loc);
    score += ScoreBetween<WEST, EAST>(loc);
    score += ScoreBetween<SOUTH, NORTH>(loc);
    score += ScoreBetween<NORTH, SOUTH>(loc);
    return score;
}

std::vector< std::pair<int, int> >& Board::GetCornersCoords()
{
    return corners;
}

std::vector< std::pair<int, int> >& Board::GetEdgesCoords()
{
    return edges;
}

std::vector< std::pair<int, int> >& Board::GetInnersCoords()
{
    return inner;
}

std::vector< Board::Loc* >& Board::GetLocations()
{
    return state.locations_per_id;
}

void Board::SwapLocations(Board::Loc* loc1,
    Board::Loc* loc2)
{
    auto& locs = GetLocations();
    locs[0] = nullptr; // used only to swap zero for non-existent pieces
    auto id1 = (loc1->ref) ? loc1->ref->GetId() : 0;
    auto id2 = (loc2->ref) ? loc2->ref->GetId() : 0;
    std::swap(locs[id1], locs[id2]);
    std::swap(loc1->ref, loc2->ref);
}

Board::Loc* Board::GetLocation(int x, int y)
{
    return &state.board[x][y];
}

PieceRef* Board::GetRef(int id, int dir)
{
    return state.refs[id][dir].get();
}

void Board::ChangeDir(Board::Loc* loc, int dir)
{
    int id = loc->ref->GetId();
    loc->ref = GetRef(id, dir);
}

bool Board::AdjustDirInner(Loc* loc)
{
    if (!loc->ref || loc->hint) {
        return false;
    }

    int start_dir = loc->ref->GetDir();
    int best_dir = 0, best_score = 0, dir_offset = 0;
    for (dir_offset = 0; dir_offset < 4; ++dir_offset) {
        int dir = (start_dir + dir_offset) % 4;
        ChangeDir(loc,dir);
        int score = GetScore(loc);
        if (score > best_score) {
            best_score = score;
            best_dir = dir;
        }
    }
    ChangeDir(loc, best_dir);

    return (best_dir != start_dir);
}