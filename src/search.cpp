#include "search.hpp"

#include "tb.hpp"
#include "tt.hpp"

#include <iostream>

namespace goldfish
{
// Dynamic futility margin based on depth
constexpr Value futility_margin(Depth d, bool improving)
{
    return Value((static_cast<int>(Value::FUTILITY_MARGIN - 50 * improving)) * d);
}

void update_cutoff(Stack* ss, Move move)
{
    if (ss->killers[0] != move && move != Move::NO_MOVE)
    {
        ss->killers[1] = ss->killers[0];
        ss->killers[0] = move;
    }
}

void Search::check_stop_conditions()
{
    // We will check the stop conditions only if we are using time management,
    // that is if our timer != null.
    if (run_timer && do_time_management)
    {
        if (timer_stopped)
        {
            abort = true;
        }
        else
        {
            // Check if we have only one move to make
            if (root_moves.size == 1)
            {
                abort = true;
            }
            else

                // Check if we have a checkmate
                if (Values::is_checkmate(root_moves.entries[0].value)
                    && current_depth >= Depth(Value::CHECKMATE
                                              - std::abs(root_moves.entries[0].value)))
            {
                abort = true;
            }
        }
    }
}

void Search::update_search(int ply)
{
    total_nodes++;

    if (ply > current_max_depth)
    {
        current_max_depth = Depth(ply);
    }

    if (search_nodes <= total_nodes)
    {
        // Hard stop on number of nodes
        abort = true;
    }

    pv[ply].size = 0;

    protocol.send_status(current_depth,
                         current_max_depth,
                         total_nodes,
                         tb_hits,
                         current_move,
                         current_move_number);
}

void Search::run()
{
    while (true)
    {
        wakeup_signal.acquire();

        if (shutdown)
        {
            break;
        }

        // Do all initialization before releasing the main thread to JCPI
        if (run_timer)
        {
            timer.start(search_time);
        }

        // Populate root move list
        MoveList<MoveEntry>& moves
            = move_generators[0].get_legal_moves(position, 1, position.is_check());
        for (int i = 0; i < moves.size; i++)
        {
            Move move                                       = moves.entries[i].move;
            root_moves.entries[root_moves.size].move        = move;
            root_moves.entries[root_moves.size].pv.moves[0] = move;
            root_moves.entries[root_moves.size].pv.size     = 1;
            root_moves.size++;
        }

        // Go...
        finished_signal.drain_permits();
        stop_signal.drain_permits();
        running = true;
        run_signal.release();

        // Tablebase lookup:
        // If successful, this will sort root_moves such that only the moves
        // that preserves the best case outcome are present in root_moves.
        tb::TableResult tb_entry = tb::probe_root(position, root_moves);
        if (!tb_entry.failed())
        {
            root_in_TB = true;
            tb_hits++;
        }

        // Load contempt value. Options value is in centipawns, so convert to whatever
        // unit Value::PAWN_VALUE defines.
        contempt = Value(((int) UCI::Options["Contempt"]) * Value::PAWN_VALUE / 100);

        //### BEGIN Iterative Deepening
        for (Depth depth = initial_depth; !abort and depth <= search_depth; ++depth)
        {
            current_depth     = depth;
            current_max_depth = Depth::DEPTH_ZERO;
            protocol.send_status(false,
                                 current_depth,
                                 current_max_depth,
                                 total_nodes,
                                 tb_hits,
                                 current_move,
                                 current_move_number);

            search_root(depth, &stacks[2], -Value::INFINITE, Value::INFINITE);

            // Sort the root move list, so that the next iteration begins with the
            // best move first.
            root_moves.sort();

            check_stop_conditions();
        }
        //### ENDOF Iterative Deepening

        if (run_timer)
        {
            timer.stop();
        }

        // Update all stats
        protocol.send_status(true,
                             current_depth,
                             current_max_depth,
                             total_nodes,
                             tb_hits,
                             current_move,
                             current_move_number);

        // Send the best move and ponder move
        Move best_move   = Move::NO_MOVE;
        Move ponder_move = Move::NO_MOVE;
        if (root_moves.size > 0)
        {
            best_move = root_moves.entries[0].move;
            if (root_moves.entries[0].pv.size >= 2)
            {
                ponder_move = root_moves.entries[0].pv.moves[1];
            }
        }

        // Send the best move to the GUI
        protocol.send_best_move(best_move, ponder_move);

        running = false;
        stop_signal.release();
        finished_signal.release();
    }
}

/*
 Principal Variation Search

 Search first move fully, then just check for moves that will
 improve alpha using a 1-point search window. If first move was
 best, then we will save lots of time as bounding is much faster than
 finding exact scores. Given a good ordering (which we have due to
 iterative deepening) the first move will be very good, and lots of
 cutoffs can be made.

 If we find a later move that actually improves alpha, we must search this
 properly to find its value. The idea is that this drawback is smaller than
 the improvements gained.
*/
Value Search::pv_search(Depth  depth,
                        Stack* ss,
                        Value  alpha,
                        Value  beta,
                        int    ply,
                        int    move_number)
{
    if (depth > 1 and move_number > 0)
    {
        Value value = -search(depth - 1, ss + 1, -alpha - 1, -alpha, ply + 1);

        if (value <= alpha)
            return value;
    }

    return -search(depth - 1, ss + 1, -beta, -alpha, ply + 1);
}

Value Search::search_root(Depth depth, Stack* ss, Value alpha, Value beta)
{
    int ply = 0;

    update_search(ply);

    Value best_value = -Value::INFINITE;

    // Abort conditions
    if (abort)
    {
        return best_value;
    }

    if (root_moves.size == 0)
    {
        // The root position is a checkmate or stalemate. We cannot search
        // further. Abort!
        abort = true;
        return position.is_check() ? -Value::CHECKMATE + ply : Value::DRAW;
    }

    // Reset all values, so the best move is pushed to the front
    for (int i = 0; i < root_moves.size; i++)
    {
        root_moves.entries[i].value = -Value::INFINITE;
    }

    for (int i = 0; i < root_moves.size; i++)
    {
        Move move = root_moves.entries[i].move;

        current_move        = move;
        current_move_number = i + 1;
        protocol.send_status(false,
                             current_depth,
                             current_max_depth,
                             total_nodes,
                             tb_hits,
                             current_move,
                             current_move_number);

        position.make_move(move);

        Value value = pv_search(depth, ss, alpha, beta, ply, i);

        position.undo_move(move);

        if (abort)
        {
            return best_value;
        }

        best_value = std::max(best_value, value);

        // Do we have a better value?
        if (value > alpha)
        {
            alpha = value;

            // We found a new best move
            root_moves.entries[i].value = value;
            save_pv(move, pv[ply + 1], root_moves.entries[i].pv);

            protocol.send_move(root_moves.entries[i],
                               current_depth,
                               current_max_depth,
                               total_nodes,
                               tb_hits);

            if (value >= beta)
            {
                update_cutoff(ss, move);
                return value;
            }
        }
    }

    assert(best_value > -Value::INFINITE);
    return best_value;
}

Value Search::search(Depth depth, Stack* ss, Value alpha, Value beta, int ply)
{
    // Abort conditions
    if (abort || ply == Depth::MAX_PLY)
    {
        return Evaluation::evaluate(position);
    }

    // Check insufficient material, repetition and fifty move rule
    if (position.halfmove_clock >= 100 || position.has_insufficient_material()
        || position.is_repetition())
    {
        return contempt;
    }

    Value alpha_orig = alpha;

    // Check TTable before anything else is done.
    auto entry = TT.probe(position.zobrist_key);
    if (entry != nullptr and entry->depth() >= depth)
    {
        const Value tt_value = tt::value_from_tt(entry->value(), ply);
        if (entry->bound() & Bound::LOWER)
        {
            if (tt_value > alpha)
            {
                save_pv(entry->move(), pv[ply + 1], pv[ply]);
                alpha = tt_value;
            }
        }
        if (entry->bound() & Bound::UPPER)
        {
            if (tt_value < beta)
                beta = tt_value;
        }
        if (alpha >= beta)
        {
            update_search(ply);
            update_cutoff(ss, entry->move());
            return tt_value;
        }
    }

    // We are at a leaf/horizon. So calculate that value.
    if (depth <= 0)
    {
        // Descend into quiescent
        return quiescent(ss, alpha, beta, ply);
    }

    update_search(ply);

    // Mate distance pruning:
    // Even if we mate at the next move our score
    // would be at best CHECKMATE - ply, but if alpha is already bigger because
    // a shorter mate was found upward in the tree then there is no need to search
    // because we will never beat the current alpha. Same logic but with reversed
    // signs applies also in the opposite condition of being mated instead of giving
    // mate. In this case return a fail-high score.
    alpha = std::max(-Value::CHECKMATE + ply, alpha);
    beta  = std::min(Value::CHECKMATE - (ply + 1), beta);
    if (alpha >= beta)
        return alpha;

    // Initialize
    Value best_value     = -Value::INFINITE;
    Move  best_move      = Move::NO_MOVE;
    int   searched_moves = 0;

    // Next we check the tablebases:
    const tb::Outcome tb_outcome = tb::probe_outcome(position);
    if (tb_outcome != tb::Outcome::FAILED_PROBE)
    {
        tb_hits++;
        int wdl = tb::outcome_to_int(tb_outcome);

        constexpr int DrawScore = 1;  // Change to 0 to ignore rule50.

        Value value = wdl < -DrawScore
                          ? Value::KNOWN_LOSS + ply
                          : wdl > DrawScore ? Value::KNOWN_WIN - ply
                                            : contempt + 2 * DrawScore * wdl;

        Bound b = wdl < -DrawScore ? Bound::UPPER
                                   : wdl > DrawScore ? Bound::LOWER : Bound::EXACT;

        if (b == Bound::EXACT || (b == Bound::LOWER && value >= beta)
            || (b == Bound::UPPER && value <= alpha))
        {
            // Tablebase result is final, no need to go futher.
            TT.store(position.zobrist_key,
                     tt::value_to_tt(value, ply),
                     b,
                     std::min(Depth::MAX_PLY - 1, depth + 5),
                     Move::NO_MOVE);
            return value;
        }

        if (b == Bound::LOWER)
        {
            best_value = value;
            alpha      = std::max(alpha, value);
        }
    }

    bool is_check = position.is_check();
    bool improving;

    if (is_check)
    {
        depth += 1;  // Extension for checks.
        ss->staticEval = Value::NO_VALUE;
        improving      = false;
    }
    else  // Speculative pruning available when not in check:
    {
        ss->staticEval = Evaluation::evaluate(position);

        // Razoring
        //
        // If we are close to the horizon and we seem to be doing very bad,
        // skip straight to quiescent search instead of doing so for every move.
        if (depth < 2 && alpha + 1 == beta
            && ss->staticEval + Value::RAZOR_MARGIN <= alpha)
            return quiescent(ss, alpha, beta, ply);

        // Improving is true if we have a better static eval than we did on our last
        // move (2 ply ago). Special case for when we have no defined static eval for
        // last move, where we let improving = true.
        improving = ss->staticEval >= (ss - 2)->staticEval
                    || (ss - 2)->staticEval == Value::NO_VALUE;

        // Futility Pruning
        //
        // If we are near the horizon and the static eval is sufficiently larger than
        // beta, then we can assume that we won't lose the advantage and exit
        // immediately.
        if (depth < 3 && ss->staticEval - futility_margin(depth, improving) >= beta
            && ss->staticEval < Value::KNOWN_WIN)  // Don't trust unproven wins.
            return ss->staticEval;

        // Null move pruning.
        //
        // Idea is that if we have a position that is so strong that even if we
        // don't move (i.e. pass), we still exceed beta. Only started if we have reason
        // to belive that NMP will be useful, which here is check by eval >= beta.
        //
        // Only used when the following is _NOT_ true:
        //
        //  1. We are in check (NM would be illegal) (already tested for)
        //  2. The last move made was a null move (double null move has no effect other
        //  than reduced depth)
        //  3. A beta-cutoff must be by finding a mate score (mates with NM is not
        //  proven)
        //  4. We are in zugzwang (not moving is better than any other move)
        //
        // Number 4 is hard to guarantee (but possible with verification search, see
        // SF). But by not using null move when we only have K and P we escape most
        // cases.
        if (beta < Value::CHECKMATE_THRESHOLD && !position.last_move_was_null_move()
            && (position.pieces[position.active_color][PieceType::QUEEN]
                || position.pieces[position.active_color][PieceType::ROOK]
                || position.pieces[position.active_color][PieceType::BISHOP]
                || position.pieces[position.active_color][PieceType::KNIGHT])
            && Evaluation::evaluate(position) >= beta)
        {
            position.make_null_move();
            assert(position.last_move_was_null_move());

            // We do recursive null move, with depth reduction factor 3.
            // Why 3? Because this is common, for instance in sunfish.
            constexpr Depth R = Depth(3);
            Value value       = -search(depth - R, ss + 1, -beta, -beta + 1, ply + 1);

            position.undo_null_move();
            assert(!position.last_move_was_null_move());

            // Beta cutoff?
            if (value >= beta)
            {
                // Do not return unproven mate scores
                if (value >= Value::CHECKMATE_THRESHOLD)
                    value = beta;

                TT.store(position.zobrist_key,
                         tt::value_to_tt(value, ply),
                         Bound::LOWER,
                         std::max(Depth::DEPTH_ZERO, depth - R + 1),
                         Move::NO_MOVE);
                return value;
            }
        }
    }

    // Internal Iterative deepening:
    // When we have no good guess for the best move, do a reduced search
    // first to find a likely candidate. Only do this if a search would
    // lead to a new entry in the TT.
    constexpr Depth iid_reduction = Depth(7);
    if (depth > iid_reduction
        and (entry == nullptr
             or (entry->move() == Move::NO_MOVE
                 and entry->depth() < depth - iid_reduction)))
    {
        search(depth - iid_reduction, ss + 1, alpha, beta, ply);
        entry = TT.probe(position.zobrist_key);
    }

    MoveList<MoveEntry>& moves
        = move_generators[ply].get_moves(position, depth, is_check);


    // Killer Move Heuristic:
    // Search moves that caused cuttoffs in sibling nodes early.
    // Killers are moved further to the front of the list.
    moves.add_killer(ss->killers[1]);
    moves.add_killer(ss->killers[0]);

    // TTable Move Heuristic:
    // If lookup didn't cause a cutoff, including if we don't have the required depth to
    // use the table entry, lets use the stored move as a best move, searching it
    // first in the hopes that it will lead to more cutoffs.
    if (entry != nullptr and entry->move() != Move::NO_MOVE)
        moves.sort_as_best(entry->move());

    for (int i = 0; i < moves.size; i++)
    {
        Move  move  = moves.entries[i].move;
        Value value = best_value;

        position.make_move(move);
        if (!position.is_check(~position.active_color))
        {
            value = pv_search(depth, ss, alpha, beta, ply, searched_moves);

            searched_moves++;
        }
        position.undo_move(move);

        if (abort)
        {
            return best_value;
        }

        // Pruning
        if (value > best_value)
        {
            best_value = value;
            best_move  = move;

            // Do we have a better value?
            if (value > alpha)
            {
                alpha = value;
                save_pv(move, pv[ply + 1], pv[ply]);

                // Is the value higher than beta?
                if (value >= beta)
                {
                    // Cut-off
                    break;
                }
            }
        }
    }

    // Determine bound type.
    Bound best_value_bound = best_value <= alpha_orig
                                 ? Bound::UPPER
                                 : best_value >= beta ? Bound::LOWER : Bound::EXACT;

    // If we cannot move, check for checkmate and stalemate.
    if (searched_moves == 0)
    {
        best_value       = is_check ? -Value::CHECKMATE + ply : contempt;
        best_value_bound = Bound::EXACT;
    }

    update_cutoff(ss, best_move);

    TT.store(position.zobrist_key,
             tt::value_to_tt(best_value, ply),
             best_value_bound,
             depth,
             best_move);
    return best_value;
}

Value Search::quiescent(Stack* ss, Value alpha, Value beta, int ply)
{
    // No need to check the TT, as we only descend to quiescence if there is
    // no entry in the table.

    update_search(ply);

    // Abort conditions
    if (abort || ply == Depth::MAX_PLY)
    {
        return Evaluation::evaluate(position);
    }

    // Check insufficient material, repetition and fifty move rule
    if (position.is_repetition() || position.has_insufficient_material()
        || position.halfmove_clock >= 100)
    {
        return contempt;
    }

    // Initialize
    Value best_value     = -Value::INFINITE;
    int   searched_moves = 0;
    bool  is_check       = position.is_check();

    //### BEGIN Stand pat
    if (!is_check)
    {
        best_value = Evaluation::evaluate(position);

        // Do we have a better value?
        if (best_value > alpha)
        {
            alpha = best_value;

            // Is the value higher than beta?
            if (best_value >= beta)
            {
                // Cut-off
                return best_value;
            }
        }
    }
    //### ENDOF Stand pat

    // Delta pruning:
    // Test if alpha can be improved by greatest
    // possible material swing. If not, then don't bother.
    //
    // Best possible single move is to capture a queen while promoting a pawn.
    // Make sure we're not in check, as then the stand pat is -INFINITE.
    Value delta = 2 * Value::QUEEN_VALUE - Value::PAWN_VALUE;
    if (!is_check and best_value + delta < alpha)
        return best_value;

    MoveList<MoveEntry>& moves
        = move_generators[ply].get_moves(position, Depth::DEPTH_ZERO, is_check);
    for (int i = 0; i < moves.size; i++)
    {
        Move  move  = moves.entries[i].move;
        Value value = best_value;

        position.make_move(move);
        if (!position.is_check(~position.active_color))
        {
            searched_moves++;
            // Note that we do not use PVS here, as we have no
            // reason to believe move ordering works very well here, and
            // we know we don't have a killer move from TT.
            value = -quiescent(ss + 1, -beta, -alpha, ply + 1);
        }
        position.undo_move(move);

        if (abort)
        {
            return best_value;
        }

        // Pruning
        if (value > best_value)
        {
            best_value = value;

            // Do we have a better value?
            if (value > alpha)
            {
                alpha = value;
                save_pv(move, pv[ply + 1], pv[ply]);

                // Is the value higher than beta?
                if (value >= beta)
                {
                    // Cut-off
                    update_cutoff(ss, move);
                    break;
                }
            }
        }
    }

    // If we cannot move, check for checkmate.
    if (searched_moves == 0 && is_check)
    {
        // We have a check mate. This is bad for us, so return a -CHECKMATE.
        return -Value::CHECKMATE + ply;
    }

    return best_value;
}

}  // namespace goldfish
