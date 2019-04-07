#pragma once

#include "Fathom/src/tbprobe.h"
#include "movegenerator.hpp"
#include "movelist.hpp"
#include "position.hpp"

#include <array>
#include <string>

namespace goldfish::tb
{
enum class Outcome
{
    LOSS         = TB_LOSS,
    BLESSED_LOSS = TB_BLESSED_LOSS,
    DRAW         = TB_DRAW,
    CURSED_WIN   = TB_CURSED_WIN,
    WIN          = TB_WIN,
    FAILED_PROBE = TB_GET_WDL(TB_RESULT_FAILED)
};

// Detect compatability with fathom tablebase definitions.
static_assert(static_cast<int>(Outcome::LOSS) == 0);
static_assert(static_cast<int>(Outcome::BLESSED_LOSS) == 1);
static_assert(static_cast<int>(Outcome::DRAW) == 2);
static_assert(static_cast<int>(Outcome::CURSED_WIN) == 3);
static_assert(static_cast<int>(Outcome::WIN) == 4);

constexpr int outcome_to_int(const Outcome o)
{
    return static_cast<int>(o) - static_cast<int>(Outcome::DRAW);
}

// Holds the maximum number of pieces seen in tablebase (after initialize is called).
extern unsigned MAX_MAN;

/*
 * Initialize the tablebase.
 *
 * PARAMETERS:
 * - path:
 *   The tablebase PATH string.
 *   Should be the path to the folder containing all *.rtbz and *rtbw files.
 *
 * RETURN:
 * - true=succes, false=failed.
 *   Only returns false if there was an error loading the files, _NO_ error if
 *   files are not found.
 *   The TB_LARGEST global will also be initialized. If no tablebase files are
 *   found, then `true' is returned and TB_LARGEST is set to zero.
 */
bool initialize(std::string path);

/*
 * Probe the Win-Draw-Loss (WDL) table.
 *
 * PARAMETERS:
 * - pos
 *   The current position.
 *
 * RETURN:
 * - Outcome value
 *
 * NOTES:
 * - This function is thread safe assuming TB_NO_THREADS is disabled.
 */
inline Outcome probe_outcome(const Position& pos)
{
    // Tablebase only for positions without castling and with reset clock.
    // Also check that we have a tablebase at all, and that there are few enough pieces.
    if (pos.halfmove_clock || pos.castling_rights || !MAX_MAN
        || MAX_MAN < tb_pop_count(pos.get_pieces(Color::WHITE)
                                  | pos.get_pieces(Color::BLACK)))
        return Outcome::FAILED_PROBE;

    const Outcome outcome = static_cast<Outcome>(
        tb_probe_wdl_impl(pos.get_pieces(Color::WHITE),
                          pos.get_pieces(Color::BLACK),
                          pos.get_pieces(PieceType::KING),
                          pos.get_pieces(PieceType::QUEEN),
                          pos.get_pieces(PieceType::ROOK),
                          pos.get_pieces(PieceType::BISHOP),
                          pos.get_pieces(PieceType::KNIGHT),
                          pos.get_pieces(PieceType::PAWN),
                          pos.enpassant_square == Square::NO_SQUARE
                              ? 0
                              : Bitboard::to_bit_square(pos.enpassant_square),
                          pos.active_color == Color::WHITE));
    return outcome;
}

struct TableResult
{
private:
    unsigned probe_;

public:
    TableResult() : probe_(TB_RESULT_FAILED) {}
    explicit TableResult(unsigned probe) : probe_(probe) {}
    constexpr bool     failed() const { return probe_ == TB_RESULT_FAILED; }
    constexpr bool     checkmate() const { return probe_ == TB_RESULT_CHECKMATE; }
    constexpr bool     stalemate() const { return probe_ == TB_RESULT_STALEMATE; }
    constexpr unsigned distance_to_zero() const { return TB_GET_DTZ(probe_); }
    constexpr Outcome  outcome() const
    {
        return static_cast<Outcome>(TB_GET_WDL(probe_));
    }
    constexpr bool move_equal_to(const Move& m) const
    {
        return Moves::get_origin_square(m)
                   == Square(Bitboard::to_x88_square(TB_GET_FROM(probe_)))
               && Moves::get_target_square(m)
                      == Square(Bitboard::to_x88_square(TB_GET_TO(probe_)))
               && ((Moves::get_promotion(m) == PieceType::NO_PIECE_TYPE
                    && TB_GET_PROMOTES(probe_) == TB_PROMOTES_NONE)
                   || (Moves::get_promotion(m) == PieceType::QUEEN
                       && TB_GET_PROMOTES(probe_) == TB_PROMOTES_QUEEN)
                   || (Moves::get_promotion(m) == PieceType::ROOK
                       && TB_GET_PROMOTES(probe_) == TB_PROMOTES_ROOK)
                   || (Moves::get_promotion(m) == PieceType::BISHOP
                       && TB_GET_PROMOTES(probe_) == TB_PROMOTES_BISHOP)
                   || (Moves::get_promotion(m) == PieceType::KNIGHT
                       && TB_GET_PROMOTES(probe_) == TB_PROMOTES_KNIGHT));
    }
};

static_assert(sizeof(TableResult) == sizeof(unsigned));

/*
 * Probe the Distance-To-Zero (DTZ) table.
 *
 * PARAMETERS:
 * - pos
 *   The current position.
 * - moves
 *   List of legal moves
 *
 * RETURN:
 * - A TableResult instance
 * - moves list will be modified such that if there are n moves that preserve
 *   the best case outcome, these moves will be the first n entries in the list,
 *   and the remaining moves deleted. As such, any move chosen after this call
 *   will be guaranteed to not lose points.
 *
 * NOTES:
 * - Engines can use this function to probe at the root.  This function should
 *   not be used during search.
 * - This function is NOT thread safe.  For engines this function should only
 *   be called once at the root per search.
 */
template <typename Entry>
inline TableResult probe_root(const Position& pos, MoveList<Entry>& moves)
{
    if (pos.castling_rights || !MAX_MAN)
        return TableResult(TB_RESULT_FAILED);

    std::array<unsigned, TB_MAX_MOVES> results;

    TableResult result = TableResult {
        tb_probe_root_impl(pos.get_pieces(Color::WHITE),
                           pos.get_pieces(Color::BLACK),
                           pos.get_pieces(PieceType::KING),
                           pos.get_pieces(PieceType::QUEEN),
                           pos.get_pieces(PieceType::ROOK),
                           pos.get_pieces(PieceType::BISHOP),
                           pos.get_pieces(PieceType::KNIGHT),
                           pos.get_pieces(PieceType::PAWN),
                           pos.halfmove_clock,
                           pos.enpassant_square == Square::NO_SQUARE
                               ? 0
                               : Bitboard::to_bit_square(pos.enpassant_square),
                           pos.active_color == Color::WHITE,
                           results.data())};

    if (result.failed())
        return result;

#ifndef NDEBUG
    // results should be terminated by a TB_RESULT_FAILED
    // cannot just check results[root_moves.size], as root_moves
    // might have been shortened already by a previous probe.
    bool results_properly_terminated = false;
    for (unsigned i = 0; i < results.size(); ++i)
    {
        if (results[i] == TB_RESULT_FAILED)
        {
            results_properly_terminated = true;
            break;
        }
    }
    assert(results_properly_terminated);
#endif

    int optimal_move_count = 0;
    for (const auto& raw : results)
    {
        const TableResult tbres {raw};
        if (tbres.outcome() == result.outcome())
        {
            for (int j = optimal_move_count; j < moves.size; ++j)
            {
                if (tbres.move_equal_to(moves.entries[j].move))
                {
                    std::swap(moves.entries[optimal_move_count], moves.entries[j]);
                    ++optimal_move_count;
                    break;
                }
            }
        }
    }
    moves.size = optimal_move_count;

    return result;
}

/*
 * Utility version of the above, helpful for easy testing when move ordering is
 * unimportant.
 */
inline TableResult probe_root(const Position& pos)
{
    MoveGenerator mg;
    auto          moves = mg.get_legal_moves(pos, 1, pos.is_check());
    return probe_root(pos, moves);
}

}  // namespace goldfish::tb
