#include <algorithm>
#include <fstream>
#include "Board.h"

using namespace edge;

template <int FIRST, int SECOND>
int ScoreBetween(const Board::BoardLoc* loc) {
    auto second = loc->neighbours[FIRST];
    if (!loc || !second) return 0;
    return (loc->ref->GetPattern(FIRST) == second->ref->GetPattern(SECOND)) ? 1 : 0;
}

Board::BoardLoc::BoardLoc() : hint(nullptr), x(0), y(0)
{
    // need to be relinked
    neighbours[0] = 0;
    neighbours[1] = 0;
    neighbours[2] = 0;
    neighbours[3] = 0;
}

Board::BoardLoc::BoardLoc(const BoardLoc& other) : hint(other.hint), x(other.x), y(other.y)
{
    // ref ignored, it is always empty during copy/assignment, must be relinked
    // same for neighbours
    neighbours[0] = 0;
    neighbours[1] = 0;
    neighbours[2] = 0;
    neighbours[3] = 0;
}

Board::BoardLoc& Board::BoardLoc::operator= (const Board::BoardLoc& other)
{
    // ref ignored, it is always empty during copy/assignment, must be relinked
    // same for neighbours
    if (this != &other)
    {
        hint = other.hint;
        x = other.x;
        y = other.y;

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

    for (int k = 1; k < def->GetWidth() - 1; ++k) {
        top_edges.push_back(std::pair<int, int>(0, k));
        bottom_edges.push_back(std::pair<int, int>(def->GetHeight() - 1, k));
        edges.push_back(std::pair<int, int>(0, k));
        edges.push_back(std::pair<int, int>(def->GetHeight() - 1, k));
    }
    for (int k = 1; k < def->GetHeight() - 1; ++k) {
        left_edges.push_back(std::pair<int, int>(k, 0));
        right_edges.push_back(std::pair<int, int>(k, def->GetWidth() - 1));
        edges.push_back(std::pair<int, int>(k, 0));
        edges.push_back(std::pair<int, int>(k, def->GetWidth() - 1));
    }

    for (int i = 1; i < def->GetHeight() - 1; ++i) {
        for (int j = 1; j < def->GetWidth() - 1; ++j) {
            inner.push_back(std::pair<int, int>(i, j));
        }
    }

    UpdateLinks();
    UpdateIds();
}

void Board::Save(const std::string& filename)
{
    std::ofstream file(filename);
    for (int x = 0; x < def->GetHeight(); ++x) {
        for (int y = 0; y < def->GetHeight(); ++y) {
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
    state.locations.resize(pieces_count + 1); // id's are one indexed

    for (int i = 0; i < def->GetHeight(); ++i) {
        for (int j = 0; j < def->GetWidth(); ++j) {
            if (state.board[i][j].ref)
            {
                state.locations[state.board[i][j].ref->GetId()] = &state.board[i][j];
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

    state.board[0][0].ref =
        std::unique_ptr<PieceRef>(new PieceRef(*corners_it++, EAST));
    state.board[0][def->GetWidth() - 1].ref =
        std::unique_ptr<PieceRef>(new PieceRef(*corners_it++, SOUTH));
    state.board[def->GetHeight() - 1][0].ref =
        std::unique_ptr<PieceRef>(new PieceRef(*corners_it++, NORTH));
    state.board[def->GetHeight() - 1][def->GetWidth() - 1].ref =
        std::unique_ptr<PieceRef>(new PieceRef(*corners_it++, WEST));

    for (auto dest : top_edges) {
        state.board[dest.first][dest.second].ref = std::unique_ptr<PieceRef>(new PieceRef(*edges_it++, EAST));
    }
    for (auto dest : bottom_edges) {
        state.board[dest.first][dest.second].ref = std::unique_ptr<PieceRef>(new PieceRef(*edges_it++, WEST));
    }
    for (auto dest : left_edges) {
        state.board[dest.first][dest.second].ref = std::unique_ptr<PieceRef>(new PieceRef(*edges_it++, NORTH));
    }
    for (auto dest : right_edges) {
        state.board[dest.first][dest.second].ref = std::unique_ptr<PieceRef>(new PieceRef(*edges_it++, SOUTH));
    }
    for (auto dest : inner) {
        state.board[dest.first][dest.second].ref = std::unique_ptr<PieceRef>(new PieceRef(*inner_it++, rand() % 4));
    }

    UpdateIds();

    for (int x = 0; x < def->GetHeight(); ++x) {
        for (int y = 0; y < def->GetWidth(); ++y) {
            auto& loc = state.board[x][y];
            if (loc.hint) {
                int hint_id = loc.hint->id;
                auto& current_hint_loc = state.locations[hint_id];
                SwapLocations(current_hint_loc, &loc);
                if (loc.hint->dir != -1) {
                    loc.ref->SetDir((loc.hint->dir));
                }
            }
        }
    }
}

void Board::AdjustDirBorder()
{
    state.board[0][0].ref->SetDir(EAST);
    state.board[0][def->GetWidth() - 1].ref->SetDir(SOUTH);
    state.board[def->GetHeight() - 1][0].ref->SetDir(NORTH);
    state.board[def->GetHeight() - 1][def->GetWidth() - 1].ref->SetDir(WEST);

    for (auto dest : top_edges) {
        state.board[dest.first][dest.second].ref->SetDir(EAST);
    }
    for (auto dest : bottom_edges) {
        state.board[dest.first][dest.second].ref->SetDir(WEST);
    }
    for (auto dest : left_edges) {
        state.board[dest.first][dest.second].ref->SetDir(NORTH);
    }
    for (auto dest : right_edges) {
        state.board[dest.first][dest.second].ref->SetDir(SOUTH);
    }
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

int Board::GetScore(BoardLoc* loc)
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

std::vector< Board::BoardLoc* >& Board::GetLocations()
{
    return state.locations;
}


void Board::SwapLocations(Board::BoardLoc* loc1,
    Board::BoardLoc* loc2)
{
    auto& locs = GetLocations();
    std::swap(locs[loc1->ref->GetId()], locs[loc2->ref->GetId()]);
    std::swap(loc1->ref, loc2->ref);
}

bool Board::AdjustDirInner(BoardLoc* loc)
{
    if (!loc->ref || loc->hint) {
        return false;
    }

    int start_dir = loc->ref->GetDir();
    int best_dir = 0, best_score = 0, dir_offset = 0;
    for (dir_offset = 0; dir_offset < 4; ++dir_offset) {
        int dir = (start_dir + dir_offset) % 4;
        loc->ref->SetDir(dir);
        int score = GetScore(loc);
        if (score > best_score) {
            best_score = score;
            best_dir = dir;
        }
    }
    loc->ref->SetDir(best_dir);

    return (best_dir != start_dir);
}