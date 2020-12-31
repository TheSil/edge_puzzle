#include "Stats.h"

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