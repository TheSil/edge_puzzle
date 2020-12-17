#pragma once

#include "Board.h"

namespace edge {

class Swapper
{
public:
    Swapper(const Board& board);

    void DoSwap();

    const Board* GetCurrentBoard();

private:

    enum class PieceType {
        CORNERS = 0,
        EDGES = 1,
        INNER = 2
    };

    bool DoQuickSwaps();

    bool DoQuickSwapsCorners(int score_to_beat, std::vector<
        std::pair<Board::BoardLoc*,
        Board::BoardLoc*>>&same_score_pieces_pairs);

    bool DoQuickSwapsEdges(int score_to_beat, std::vector<
        std::pair<Board::BoardLoc*,
        Board::BoardLoc*>>&same_score_pieces_pairs);

    bool DoQuickSwapsInners(int score_to_beat, std::vector<
        std::pair<Board::BoardLoc*,
        Board::BoardLoc*>>&same_score_pieces_pairs);

    bool HaveCommonEdge(Board::BoardLoc* loc1,
        Board::BoardLoc* loc2);

    void Shuffle();

    void Shuffle(std::vector< Board::BoardLoc* >& locations, int count);

private:
    enum class State {
        QUICK_SWAPPING = 0,
        RANDOM_SHUFFLING = 1,
        RANDOM_RECOVERING = 2
    };

    std::unique_ptr<Board> board;
    std::unique_ptr<Board> board_backup;
    State state;
    int max_score;
    int score_before;
    int quick_swapping_counter;
    int recovering_counter;
};
}
