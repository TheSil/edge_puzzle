#include <numeric>
#include <algorithm>
#include <array>
#include "Swapper.h"

using namespace edge;

Swapper::Swapper(const Board& board)
    : board(board.Clone()), board_backup(nullptr),
    state(QUICK_SWAPPING), max_score(0), score_before(0),
    quick_swapping_counter(0), recovering_counter(0)
{
}

void Swapper::DoSwap()
{
    switch (state)
    {
    case edge::Swapper::QUICK_SWAPPING:
    {
        DoQuickSwaps();
        int score = board->GetScore();
        if (score > max_score) {
            LINFO("Best score improved to %i\n", score);
            max_score = score;
            quick_swapping_counter = 3;
            board_backup = board->Clone();
        }
        else {
            quick_swapping_counter -= 1;
            if (quick_swapping_counter <= 0) {
                LDEBUG("QUICK_SWAPPING not successful, switching to RANDOM_SHUFFLING\n");
                state = RANDOM_SHUFFLING;
                quick_swapping_counter = 3;
            }
        }
    }
    break;
    case edge::Swapper::RANDOM_SHUFFLING:
    {
        recovering_counter = 2;
        Shuffle();
        score_before = board->GetScore();
        state = RANDOM_RECOVERING;
    }
    break;
    case edge::Swapper::RANDOM_RECOVERING:
    {
        // we have been shuffled, and trying to recover
        DoQuickSwaps();
        int score = board->GetScore();
        if (score > max_score) {
            // shuffle succeeded, try regular quick swapping again
            LDEBUG("RANDOM_RECOVERING successful, reached %i > %i, trying QUICK_SWAPPING again\n",
                score, max_score);
            state = QUICK_SWAPPING;
        }
        else if (score > score_before) {
            LDEBUG("RANDOM_RECOVERING locally improving %i -> %i, we keep shuffling\n",
                score_before, score);
            recovering_counter = 2;
        }
        else {
            recovering_counter -= 1;
            if (recovering_counter >= 0) {
                LDEBUG("RANDOM_RECOVERING not successful, going back to RANDOM_SHUFFLING\n");
                if (score < max_score) {
                    board = board_backup->Clone();
                }
                state = RANDOM_SHUFFLING;
            }
        }
        score_before = score;
    }
    break;
    default:
        break;
    }

}

const Board* Swapper::GetCurrentBoard()
{
    return board.get();
}

bool Swapper::DoQuickSwaps()
{
    int before = board->GetScore();
    int max_after = before;
    std::array<int, 3> seq = { CORNERS , EDGES , INNER };
    std::random_shuffle(seq.begin(), seq.end());
    std::vector<
        std::pair<Board::BoardLoc*,
        Board::BoardLoc*>> same_score_pieces_pairs;

    for (auto type : seq) {
        bool found = false;
        switch (type)
        {
        case CORNERS:
            found = DoQuickSwapsCorners(max_after, same_score_pieces_pairs);
            break;
        case EDGES:
            found = DoQuickSwapsEdges(max_after, same_score_pieces_pairs);
            break;
        case INNER:
            found = DoQuickSwapsInners(max_after, same_score_pieces_pairs);
            break;
        default:
            break;
        }
        if (found) {
            board->AdjustDirBorder();
            return true;
        }
    }

    LDEBUG("Can't find anything else...\n");
    if (!same_score_pieces_pairs.empty()) {
        std::random_shuffle(same_score_pieces_pairs.begin(),
            same_score_pieces_pairs.end());

        const int max_pairs_swapped = 10;
        for (size_t k = 0; k < same_score_pieces_pairs.size() && k < max_pairs_swapped; ++k) {
            LDEBUG("... exchanging pieces (%i, %i) with (%i, %i) with same result "
                "to give chance of swing into another possibilities\n",
                pair.first->x, pair.first->y, pair.second->x, pair.second->y);
            std::swap(same_score_pieces_pairs[k].first->ref,
                same_score_pieces_pairs[k].second->ref);
        }

        board->AdjustDirBorder();
        board->AdjustDirInner();
        return true;
    }

    LDEBUG("... and no pieces with same score!\n");
    return false;
}

bool Swapper::DoQuickSwapsCorners(int score_to_beat, std::vector<
    std::pair<Board::BoardLoc*,
    Board::BoardLoc*>>&same_score_pieces_pairs)
{
    auto& cont = board->GetCorners();
    std::vector<size_t> idx(cont.size());
    std::iota(idx.begin(), idx.end(), 0);
    std::vector<size_t> vals(cont.size());
    for (size_t i = 0; i < cont.size(); ++i) {
        vals[i] = board->GetScore(cont[i]);
    }
    std::stable_sort(idx.begin(), idx.end(),
        [&vals](size_t i1, size_t i2) {return vals[i1] < vals[i2]; });
    for (size_t idx1 = 1; idx1 < idx.size(); ++idx1)
    {
        for (size_t idx2 = 0; idx2 < idx1; ++idx2)
        {
            std::swap(cont[idx[idx1]]->ref, cont[idx[idx2]]->ref);
            board->AdjustDirBorder();
            int after = board->GetScore();
            if (after > score_to_beat)
            {
                return true;
            }

            if (after == score_to_beat)
            {
                same_score_pieces_pairs.push_back(
                    std::pair<Board::BoardLoc*,
                    Board::BoardLoc* >(cont[idx[idx1]], cont[idx[idx2]]));
            }

            std::swap(cont[idx[idx1]]->ref, cont[idx[idx2]]->ref);
        }
    }

    return false;
}

bool Swapper::DoQuickSwapsEdges(int score_to_beat, std::vector<
    std::pair<Board::BoardLoc*,
    Board::BoardLoc*>>&same_score_pieces_pairs)
{
    auto& cont = board->GetEdges();
    std::vector<size_t> idx(cont.size());
    std::iota(idx.begin(), idx.end(), 0);
    std::vector<size_t> vals(cont.size());
    for (size_t i = 0; i < cont.size(); ++i) {
        vals[i] = board->GetScore(cont[i]);
    }
    std::stable_sort(idx.begin(), idx.end(),
        [&vals](size_t i1, size_t i2) {return vals[i1] < vals[i2]; });
    for (size_t idx1 = 1; idx1 < idx.size(); ++idx1)
    {
        for (size_t idx2 = 0; idx2 < idx1; ++idx2)
        {
            std::swap(cont[idx[idx1]]->ref, cont[idx[idx2]]->ref);
            board->AdjustDirBorder();
            int after = board->GetScore();
            if (after > score_to_beat)
            {
                return true;
            }

            if (after == score_to_beat)
            {
                same_score_pieces_pairs.push_back(
                    std::pair<Board::BoardLoc*,
                    Board::BoardLoc* >(cont[idx[idx1]], cont[idx[idx2]]));
            }

            std::swap(cont[idx[idx1]]->ref, cont[idx[idx2]]->ref);
        }
    }

    return false;
}

bool Swapper::DoQuickSwapsInners(int score_to_beat, std::vector<
    std::pair<Board::BoardLoc*,
    Board::BoardLoc*>>&same_score_pieces_pairs)
{
    auto& cont = board->GetInners();
    std::vector<size_t> idx(cont.size());
    std::iota(idx.begin(), idx.end(), 0);
    std::vector<size_t> vals(cont.size());
    for (size_t i = 0; i < cont.size(); ++i) {
        vals[i] = board->GetScore(cont[i]);
    }
    std::stable_sort(idx.begin(), idx.end(),
        [&vals](size_t i1, size_t i2) {return vals[i1] < vals[i2]; });
    for (size_t idx1 = 1; idx1 < idx.size(); ++idx1)
    {
        auto loc1 = cont[idx[idx1]];
        for (size_t idx2 = 0; idx2 < idx1 && vals[idx[idx2]] < 4; ++idx2)
        {
            auto loc2 = cont[idx[idx2]];
            int orig_dir1 = loc1->ref->GetDir();
            int orig_dir2 = loc2->ref->GetDir();
            int piece_score_before = board->GetScore(loc1);
            piece_score_before += board->GetScore(loc2);
            if (HaveCommonEdge(loc1, loc2)) {
                piece_score_before -= 1;
            }

            std::swap(loc1->ref, loc2->ref);
            int piece_score_best_after = piece_score_before;

            for (int dir1 = 0; dir1 < 4; ++dir1) {
                for (int dir2 = 0; dir2 < 4; ++dir2) {
                    loc1->ref->SetDir(dir1);
                    loc2->ref->SetDir(dir2);
                    int piece_score_after = board->GetScore(loc1);
                    piece_score_after += board->GetScore(loc2);
                    if (HaveCommonEdge(loc1, loc2)) {
                        piece_score_after -= 1;
                    }

                    if (piece_score_after > piece_score_best_after) {
                        LDEBUG("Switching (%i, %i) <-> (%i, %i), score %i to %i\n",
                            cont[idx[idx1]]->x, cont[idx[idx1]]->y, cont[idx[idx2]]->x, cont[idx[idx2]]->y,
                            piece_score_before, piece_score_after);
                        piece_score_best_after = piece_score_after;
                        board->AdjustDirInner();
                        return true;
                    }

                    if (piece_score_after == piece_score_before) {
                        same_score_pieces_pairs.push_back(
                            std::pair<Board::BoardLoc*,
                            Board::BoardLoc* >(loc1, loc2));
                    }

                }
            }

            std::swap(loc1->ref, loc2->ref);
            loc1->ref->SetDir(orig_dir1);
            loc2->ref->SetDir(orig_dir2);
        }
    }

    return false;
}

bool Swapper::HaveCommonEdge(Board::BoardLoc* loc1,
    Board::BoardLoc* loc2)
{
    if (loc1->y == loc2->y) {
        if (loc1->x + 1 == loc2->x) {
            // inner2 is below inner1
            if (loc1->ref->GetPattern(SOUTH) == loc2->ref->GetPattern(NORTH)) {
                return true;
            }
        }
        else if (loc2->x + 1 == loc1->x) {
            // inner1 is below inner2
            if (loc1->ref->GetPattern(NORTH) == loc2->ref->GetPattern(SOUTH)) {
                return true;
            }
        }
    }
    else if (loc1->x == loc2->x) {
        if (loc1->y + 1 == loc2->y) {
            // inner2 is right to inner1
            if (loc1->ref->GetPattern(EAST) == loc2->ref->GetPattern(WEST)) {
                return true;
            }
        }
        else if (loc2->y + 1 == loc1->y) {
            // inner1 is right to inner2
            if (loc1->ref->GetPattern(WEST) == loc2->ref->GetPattern(EAST)) {
                return true;
            }
        }
    }
    return false;
}

void Swapper::Shuffle()
{
    // shuffle random pieces
    if (rand() % 2 == 1) {
        LDEBUG("shuffling random inner pieces...\n");
        auto& locations = board->GetInners();
        std::vector<int> indicies(locations.size());
        std::iota(indicies.begin(), indicies.end(), 0);
        std::random_shuffle(indicies.begin(), indicies.end());
        auto first = std::move(locations[indicies[0]]->ref);
        for (int i = 0; i < 5 - 1; ++i) {
            locations[indicies[i]]->ref = std::move(locations[indicies[i + 1]]->ref);
        }
        locations[indicies[5 - 1]]->ref = std::move(first);
    }
    else {
        LDEBUG("shuffling random edge pieces...\n");
        auto& locations = board->GetEdges();
        std::vector<int> indicies(locations.size());
        std::iota(indicies.begin(), indicies.end(), 0);
        std::random_shuffle(indicies.begin(), indicies.end());
        auto first = std::move(locations[indicies[0]]->ref);
        for (int i = 0; i < 10 - 1; ++i) {
            locations[indicies[i]]->ref = std::move(locations[indicies[i + 1]]->ref);
        }
        locations[indicies[10 - 1]]->ref = std::move(first);
    }
    board->AdjustDirBorder();
    board->AdjustDirInner();
}
