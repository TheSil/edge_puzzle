#include <algorithm>
#include "Board.h"

using namespace edge;

template <int FIRST, int SECOND>
int ScoreBetween(const Board::BoardLoc* loc) {
    auto second = loc->neighbours[FIRST];
    if (!loc || !second) return 0;
    return (loc->ref->GetPattern(FIRST) == second->ref->GetPattern(SECOND)) ? 1 : 0;
}

Board::Board(const PuzzleDef* def)
    : def(def)
{
    board.resize(def->GetHeight());
    for (size_t x = 0; x < board.size(); ++x)
    {
        board[x].resize(def->GetWidth());
        auto& row = board[x];
        for (size_t y = 0; y < row.size(); ++y) {
            row[y].x = x;
            row[y].y = y;
        }
    }

    UpdateLinks();
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

    board[0][0].ref =
        std::unique_ptr<PieceRef>(new PieceRef(*corners_it++, EAST));
    board[0][def->GetWidth() - 1].ref =
        std::unique_ptr<PieceRef>(new PieceRef(*corners_it++, SOUTH));
    board[def->GetHeight() - 1][0].ref =
        std::unique_ptr<PieceRef>(new PieceRef(*corners_it++, NORTH));
    board[def->GetHeight() - 1][def->GetWidth() - 1].ref =
        std::unique_ptr<PieceRef>(new PieceRef(*corners_it++, WEST));

    for (auto dest : top_edges) {
        dest->ref = std::unique_ptr<PieceRef>(new PieceRef(*edges_it++, EAST));
    }
    for (auto dest : bottom_edges) {
        dest->ref = std::unique_ptr<PieceRef>(new PieceRef(*edges_it++, WEST));
    }
    for (auto dest : left_edges) {
        dest->ref = std::unique_ptr<PieceRef>(new PieceRef(*edges_it++, NORTH));
    }
    for (auto dest : right_edges) {
        dest->ref = std::unique_ptr<PieceRef>(new PieceRef(*edges_it++, SOUTH));
    }
    for (auto dest : inner) {
        dest->ref = std::unique_ptr<PieceRef>(new PieceRef(*inner_it++, rand() % 4));
    }
}

void Board::AdjustDirBorder()
{
    board[0][0].ref->SetDir(EAST);
    board[0][def->GetWidth() - 1].ref->SetDir(SOUTH);
    board[def->GetHeight() - 1][0].ref->SetDir(NORTH);
    board[def->GetHeight() - 1][def->GetWidth() - 1].ref->SetDir(WEST);

    for (auto dest : top_edges) {
        dest->ref->SetDir(EAST);
    }
    for (auto dest : bottom_edges) {
        dest->ref->SetDir(WEST);
    }
    for (auto dest : left_edges) {
        dest->ref->SetDir(NORTH);
    }
    for (auto dest : right_edges) {
        dest->ref->SetDir(SOUTH);
    }
}

void Board::AdjustDirInner()
{
    bool did_change = true;
    int k = 0;
    while (did_change) {
        did_change = false;
        for (auto dest : inner) {
            if (AdjustDirInner(dest))
            {
                did_change = true;
            }
        }
        k += 1;
    }
}

int Board::GetScore() const
{
    int score = 0;
    for (int x = 0; x < def->GetHeight(); ++x) {
        for (int y = 0; y < def->GetWidth() - 1; ++y) {
            score += ScoreBetween<EAST, WEST>(&board[x][y]);
        }
    }
    for (int x = 0; x < def->GetHeight() - 1; ++x) {
        for (int y = 0; y < def->GetWidth(); ++y) {
            score += ScoreBetween<SOUTH, NORTH>(&board[x][y]);
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

std::unique_ptr<Board> Board::Clone() const
{
    std::unique_ptr<Board> copy = std::unique_ptr<Board>(new Board(def));
    for (int x = 0; x < def->GetHeight(); ++x) {
        for (int y = 0; y < def->GetWidth(); ++y) {
            copy->board[x][y].ref = std::unique_ptr<PieceRef>(new PieceRef(*board[x][y].ref));
        }
    }
    copy->UpdateLinks();
    return copy;
}

std::vector< Board::BoardLoc* >& Board::GetCorners()
{
    return corners;
}

std::vector< Board::BoardLoc* >& Board::GetEdges()
{
    return edges;
}

std::vector< Board::BoardLoc* >& Board::GetInners()
{
    return inner;
}

void Board::UpdateLinks()
{
    corners.clear();
    top_edges.clear();
    bottom_edges.clear();
    left_edges.clear();
    right_edges.clear();
    edges.clear();
    inner.clear();

    corners.push_back(&board[0][0]);
    corners.push_back(&board[0][def->GetWidth() - 1]);
    corners.push_back(&board[def->GetHeight() - 1][0]);
    corners.push_back(&board[def->GetHeight() - 1][def->GetWidth() - 1]);

    for (int k = 1; k < def->GetWidth() - 1; ++k) {
        top_edges.push_back(&board[0][k]);
        bottom_edges.push_back(&board[def->GetHeight() - 1][k]);
        edges.push_back(&board[0][k]);
        edges.push_back(&board[def->GetHeight() - 1][k]);
    }
    for (int k = 1; k < def->GetHeight() - 1; ++k) {
        left_edges.push_back(&board[k][0]);
        right_edges.push_back(&board[k][def->GetWidth() - 1]);
        edges.push_back(&board[k][0]);
        edges.push_back(&board[k][def->GetWidth() - 1]);
    }

    for (int i = 1; i < def->GetHeight() - 1; ++i) {
        for (int j = 1; j < def->GetWidth() - 1; ++j) {
            inner.push_back(&board[i][j]);
        }
    }

    for (int i = 0; i < def->GetHeight(); ++i) {
        for (int j = 0; j < def->GetWidth(); ++j) {
            board[i][j].neighbours[NORTH] = (i > 0) ? &board[i - 1][j] : nullptr;
            board[i][j].neighbours[SOUTH] = (i < def->GetHeight() - 1) ? &board[i + 1][j] : nullptr;
            board[i][j].neighbours[WEST] = (j > 0) ? &board[i][j - 1] : nullptr;
            board[i][j].neighbours[EAST] = (j < def->GetWidth() - 1) ? &board[i][j + 1] : nullptr;
        }
    }
}

bool Board::AdjustDirInner(BoardLoc* loc)
{
    if (!loc->ref) {
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