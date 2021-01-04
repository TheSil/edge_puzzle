#include <numeric>
#include <algorithm>
#include <array>
#include "Swapper.h"

using namespace edge;

Swapper::Swapper(Board& board)
    : board(board),
    state(State::QUICK_SWAPPING), max_score(0), score_before(0),
    quick_swapping_counter(0), recovering_counter(0)
{
    for (auto& pieceDef : this->board.GetPuzzleDef()->GetCorners()) {
        swappable_corners.push_back(pieceDef.id);
    }

    for (auto& pieceDef : this->board.GetPuzzleDef()->GetEdges()) {
        swappable_edges.push_back(pieceDef.id);
    }

    for (auto& pieceDef : this->board.GetPuzzleDef()->GetInner()) {
        swappable_inners.push_back(pieceDef.id);
    }

    // remove hints
    for (auto& hintDef : this->board.GetPuzzleDef()->GetHints()) {
        swappable_corners.erase(
            std::remove(swappable_corners.begin(),
                swappable_corners.end(),
                hintDef.id),
            swappable_corners.end());

        swappable_edges.erase(
            std::remove(swappable_edges.begin(),
                swappable_edges.end(),
                hintDef.id),
            swappable_edges.end());

        swappable_inners.erase(
            std::remove(swappable_inners.begin(),
                swappable_inners.end(),
                hintDef.id),
            swappable_inners.end());
    }

    max_score = board.GetScore();
}

void Swapper::DoSwap()
{
    switch (state)
    {
    case State::QUICK_SWAPPING:
    {
        DoQuickSwaps();
        int score = board.GetScore();
        if (score > max_score) {
            LINFO("Best score improved to %i\n", score);
            max_score = score;
            quick_swapping_counter = 3;
            board_backup = board.Backup();
        }
        else {
            quick_swapping_counter -= 1;
            if (quick_swapping_counter <= 0) {
                LDEBUG("QUICK_SWAPPING not successful, switching to RANDOM_SHUFFLING\n");
                state = State::RANDOM_SHUFFLING;
                quick_swapping_counter = 3;
            }
        }
    }
    break;
    case State::RANDOM_SHUFFLING:
    {
        recovering_counter = 2;
        Shuffle();
        score_before = board.GetScore();
        state = State::RANDOM_RECOVERING;
    }
    break;
    case State::RANDOM_RECOVERING:
    {
        // we have been shuffled, and trying to recover
        DoQuickSwaps();
        int score = board.GetScore();
        if (score > max_score) {
            // shuffle succeeded, try regular quick swapping again
            LDEBUG("RANDOM_RECOVERING successful, reached %i > %i, trying QUICK_SWAPPING again\n",
                score, max_score);
            state = State::QUICK_SWAPPING;
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
                if (score < max_score && !board_backup.board.empty()) {
                    board.Restore(board_backup);
                }
                state = State::RANDOM_SHUFFLING;
            }
        }
        score_before = score;
    }
    break;
    default:
        break;
    }

}

bool Swapper::DoQuickSwaps()
{
    int before = board.GetScore();
    int max_after = before;
    std::array<PieceType, 3> seq = { PieceType::CORNERS , PieceType::EDGES , PieceType::INNER };
    std::random_shuffle(seq.begin(), seq.end());
    std::vector<
        std::pair<Board::Loc*,
        Board::Loc*>> same_score_pieces_pairs;

    for (auto type : seq) {
        bool found = false;
        switch (type)
        {
        case PieceType::CORNERS:
            found = DoQuickSwapsCorners(max_after, same_score_pieces_pairs);
            break;
        case PieceType::EDGES:
            found = DoQuickSwapsEdges(max_after, same_score_pieces_pairs);
            break;
        case PieceType::INNER:
            found = DoQuickSwapsInners(max_after, same_score_pieces_pairs);
            break;
        default:
            break;
        }
        if (found) {
            board.AdjustDirBorder();
            return true;
        }
    }

    LDEBUG("Can't find anything else...\n");
    if (!same_score_pieces_pairs.empty()) {
        std::random_shuffle(same_score_pieces_pairs.begin(),
            same_score_pieces_pairs.end());

        const int max_pairs_swapped = 10;
        for (size_t k = 0; k < same_score_pieces_pairs.size() && k < max_pairs_swapped; ++k) {
            auto& pair = same_score_pieces_pairs[k];
            LDEBUG("... exchanging pieces (%i, %i) with (%i, %i) with same result "
                "to give chance of swing into another possibilities\n",
                pair.first->x, pair.first->y, pair.second->x, pair.second->y);
            board.SwapLocations(same_score_pieces_pairs[k].first,
                same_score_pieces_pairs[k].second);
        }

        board.AdjustDirBorder();
        board.AdjustDirInner();
        return true;
    }

    LDEBUG("... and no pieces with same score!\n");
    return false;
}

bool Swapper::DoQuickSwapsCorners(int score_to_beat, std::vector<
    std::pair<Board::Loc*,
    Board::Loc*>>&same_score_pieces_pairs)
{
    auto& cont = swappable_corners;
    std::vector<size_t> idx(cont.size());
    std::iota(idx.begin(), idx.end(), 0);
    std::vector<size_t> vals(cont.size());
    auto& locs = board.GetLocations();
    for (size_t i = 0; i < cont.size(); ++i) {
        vals[i] = board.GetScore(locs[cont[i]]);
    }
    std::stable_sort(idx.begin(), idx.end(),
        [&vals](size_t i1, size_t i2) {return vals[i1] < vals[i2]; });
    for (size_t idx1 = 1; idx1 < idx.size(); ++idx1)
    {
        auto& loc1 = locs[cont[idx[idx1]]];
        for (size_t idx2 = 0; idx2 < idx1; ++idx2)
        {
            auto& loc2 = locs[cont[idx[idx2]]];
            board.SwapLocations(loc1, loc2);
            board.AdjustDirBorder();
            int after = board.GetScore();
            if (after > score_to_beat)
            {
                return true;
            }

            if (after == score_to_beat)
            {
                same_score_pieces_pairs.push_back(
                    std::pair<Board::Loc*,
                    Board::Loc* >(loc1, loc2));
            }

            board.SwapLocations(loc1, loc2);
        }
    }

    return false;
}

bool Swapper::DoQuickSwapsEdges(int score_to_beat, std::vector<
    std::pair<Board::Loc*,
    Board::Loc*>>&same_score_pieces_pairs)
{
    auto& cont = swappable_edges;
    std::vector<size_t> idx(cont.size());
    std::iota(idx.begin(), idx.end(), 0);
    std::vector<size_t> vals(cont.size());
    auto& locs = board.GetLocations();
    for (size_t i = 0; i < cont.size(); ++i) {
        vals[i] = board.GetScore(locs[cont[i]]);
    }
    std::stable_sort(idx.begin(), idx.end(),
        [&vals](size_t i1, size_t i2) {return vals[i1] < vals[i2]; });
    board.AdjustDirBorder();
    auto score_before = board.GetScore();
    for (size_t idx1 = 1; idx1 < idx.size(); ++idx1)
    {
        auto& loc1 = locs[cont[idx[idx1]]];
        for (size_t idx2 = 0; idx2 < idx1; ++idx2)
        {
            auto& loc2 = locs[cont[idx[idx2]]];

            int piece_score_before = board.GetScore(loc1);
            piece_score_before += board.GetScore(loc2);
            if (HaveCommonEdge(loc1, loc2)) {
                piece_score_before -= 1;
            }

            int orig_dir1 = loc1->ref->GetDir();
            int orig_dir2 = loc2->ref->GetDir();
            board.SwapLocations(loc1, loc2);
            board.AdjustDirBorderSingle(loc1);
            board.AdjustDirBorderSingle(loc2);

            int piece_score_after = board.GetScore(loc1);
            piece_score_after += board.GetScore(loc2);
            if (HaveCommonEdge(loc1, loc2)) {
                piece_score_after -= 1;
            }

            if (piece_score_after > piece_score_before)
            {
                return true;
            }

            // convert local score to global
            auto diff = piece_score_after - piece_score_before;
            if (score_before + diff == score_to_beat)
            {
                same_score_pieces_pairs.push_back(
                    std::pair<Board::Loc*,
                    Board::Loc* >(loc1, loc2));
            }

            board.SwapLocations(loc1, loc2);
            board.ChangeDir(loc1, orig_dir1);
            board.ChangeDir(loc2, orig_dir2);
        }
    }

    return false;
}

bool Swapper::DoQuickSwapsInners(int score_to_beat, std::vector<
    std::pair<Board::Loc*,
    Board::Loc*>>&same_score_pieces_pairs)
{
    auto& cont = swappable_inners;
    std::vector<size_t> idx(cont.size());
    std::iota(idx.begin(), idx.end(), 0);
    std::vector<size_t> vals(cont.size());
    auto& locs = board.GetLocations();
    for (size_t i = 0; i < cont.size(); ++i) {
        vals[i] = board.GetScore(locs[cont[i]]);
    }
    std::stable_sort(idx.begin(), idx.end(),
        [&vals](size_t i1, size_t i2) {return vals[i1] < vals[i2]; });
    for (size_t idx1 = 1; idx1 < idx.size(); ++idx1)
    {
        auto loc1 = locs[cont[idx[idx1]]];
        for (size_t idx2 = 0; idx2 < idx1 && vals[idx[idx2]] < 4; ++idx2)
        {
            auto loc2 = locs[cont[idx[idx2]]];
            int orig_dir1 = loc1->ref->GetDir();
            int orig_dir2 = loc2->ref->GetDir();
            int piece_score_before = board.GetScore(loc1);
            piece_score_before += board.GetScore(loc2);
            if (HaveCommonEdge(loc1, loc2)) {
                piece_score_before -= 1;
            }

            board.SwapLocations(loc1, loc2);
            int piece_score_best_after = piece_score_before;

            for (int dir1 = 0; dir1 < 4; ++dir1) {
                for (int dir2 = 0; dir2 < 4; ++dir2) {
                    board.ChangeDir(loc1, dir1);
                    board.ChangeDir(loc2, dir2);
                    int piece_score_after = board.GetScore(loc1);
                    piece_score_after += board.GetScore(loc2);
                    if (HaveCommonEdge(loc1, loc2)) {
                        piece_score_after -= 1;
                    }

                    if (piece_score_after > piece_score_best_after) {
                        LDEBUG("Switching (%i, %i) <-> (%i, %i), score %i to %i\n",
                            loc1->x, loc1->y, loc2->x, loc2->y,
                            piece_score_before, piece_score_after);
                        piece_score_best_after = piece_score_after;
                        board.AdjustDirInner();
                        return true;
                    }

                    if (piece_score_after == piece_score_before) {
                        same_score_pieces_pairs.push_back(
                            std::pair<Board::Loc*,
                            Board::Loc* >(loc1, loc2));
                    }

                }
            }

            board.SwapLocations(loc1, loc2);
            board.ChangeDir(loc1, orig_dir1);
            board.ChangeDir(loc2, orig_dir2);
        }
    }

    return false;
}

bool Swapper::HaveCommonEdge(Board::Loc* loc1,
    Board::Loc* loc2)
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
    if (rand() % board.GetPuzzleDef()->GetPieceCount() < board.GetInnersCoords().size()) {
        LDEBUG("shuffling random inner pieces...\n");
        auto& ids = swappable_inners;
        Shuffle(ids, 5);
    }
    else {
        LDEBUG("shuffling random edge pieces...\n");
        auto& ids = swappable_edges;
        Shuffle(ids, 10);
    }
    board.AdjustDirBorder();
    board.AdjustDirInner();
}

void Swapper::Shuffle(std::vector< int >& ids, int count)
{
    std::vector<int> indicies(ids.size());
    std::iota(indicies.begin(), indicies.end(), 0);
    std::random_shuffle(indicies.begin(), indicies.end());
    auto& locs = board.GetLocations();
    auto first = std::move(locs[ids[indicies[0]]]->ref);
    for (int i = 0; i < count - 1; ++i) {
        locs[ids[indicies[i]]]->ref = std::move(locs[ids[indicies[i + 1]]]->ref);
    }
    locs[ids[indicies[count - 1]]]->ref = std::move(first);
}

