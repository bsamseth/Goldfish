use chess::{BitBoard, Board, ChessMove, Color, MoveGen, Piece};

use crate::chessmove::ChessMoveExt;
use crate::{newtypes::Value, opts::OPTS};

#[derive(Debug)]
pub struct MoveVec(Vec<MoveEntry>);

#[derive(Debug, Clone)]
pub struct MoveEntry {
    pub mv: ChessMove,
    pub value: Value,
}

impl MoveEntry {
    pub fn new(mv: ChessMove) -> Self {
        Self {
            mv,
            value: -Value::INFINITE,
        }
    }
}

impl MoveVec {
    /// Use the Most Valuable Victim / Least Valuable Attacker heuristic to score moves.
    ///
    /// Critically, this implementaiton gives _any_ capture a positive score, at least 100
    /// centipawns in the case of `QxP`. This is done by ensuring that any captured piece is worth 10
    /// times more than the attacking piece. In the `QxP` example, the queen is worth 900 centipawns
    /// and the pawn is worth 100 centipawns, so the move is scored +100 centipawns.
    ///
    /// TODO: See if this can be swapped out for a Static Exchange Evaluation, provided an
    /// efficient implementation can be found using what's available from [`Board`].
    pub fn mvv_lva_rated(mut self, board: &Board) -> Self {
        for entry in self.iter_mut() {
            let mut value = 0i16;

            // In case of promotions, add the value of the promoted piece to the move value.
            if let Some(piece) = entry.mv.get_promotion() {
                value += 10 * (piece_value(piece) - piece_value(Piece::Pawn));
            }

            // Capture? Prefer captures with higher value victims, by lower value attackers.
            if let Some(target) = entry.mv.captures(board) {
                value += 10 * piece_value(target)
                    - piece_value(
                        board
                            .piece_on(entry.mv.get_source())
                            .expect("there should always be a source piece for a move"),
                    );
            }

            entry.value = Value::new(value);
        }
        self
    }

    /// Use move history statistics to adjust the move ordering.
    ///
    /// Each time a move is found to be best at a node, a counter is incremented for that from-to square pair.
    /// Order moves that tend to be best furhter up the list.
    ///
    /// Source: <http://www.frayn.net/beowulf/theory.html#history>
    pub fn with_history_stats(mut self, stats: &[[usize; 64]; 64]) -> Self {
        let max = self
            .iter()
            .map(|m| stats[m.mv.get_source().to_index()][m.mv.get_dest().to_index()])
            .max()
            .unwrap_or(1)
            .max(1);

        for entry in self.iter_mut() {
            let value = i16::try_from(
                stats[entry.mv.get_source().to_index()][entry.mv.get_dest().to_index()]
                    * OPTS.max_history_stats_impact
                    / max,
            )
            .expect("value is in range [0, MAX_HISTORY_STATS_IMPACT]");
            entry.value += Value::new(value);
        }
        self
    }

    pub fn sort(&mut self) {
        self.sort_by_key(|m| -m.value);
    }

    pub fn sorted(mut self) -> Self {
        self.sort();
        self
    }

    /// Add a bonus for killer moves.
    ///
    /// These are quiet moves that caused a beta cutoff in sibling nodes. Consider these right
    /// after captures. This is done by treating them as if they capture a pawn worth 99 centipawns.
    pub fn add_killers(mut self, killers: impl IntoIterator<Item = ChessMove>) -> Self {
        for killer in killers {
            for entry in self.iter_mut() {
                if entry.mv == killer {
                    entry.value += Value::new(piece_value(Piece::Pawn) - 1);
                }
            }
        }
        self
    }

    /// Sorts the moves with the given preference first, then by value.
    ///
    /// The preference, if a valid move, is placed first.
    /// Then the rest of the moves are sorted by value.
    pub fn sort_with_preference(mut self, best_move: Option<ChessMove>) -> Self {
        let mut sort_from = 0;
        if let Some(mv) = best_move {
            if let Some(index) = self.iter().position(|m| m.mv == mv) {
                self.swap(0, index);
                sort_from = 1;
            }
        }

        let (_, rest) = self.split_at_mut(sort_from);
        rest.sort_by_key(|m| -m.value);

        self
    }

    pub fn see(board: &Board, capture: ChessMove) -> Value {
        debug_assert!(capture.captures(board).is_some());

        let initial_capture = capture
            .captures(board)
            .expect("should only call see on captures");
        let target_square = capture.get_dest();
        let initial_colour = board.side_to_move();
        let mut blockers = board.combined() ^ BitBoard::from_square(capture.get_source());
        let white_pieces = board.color_combined(Color::White);
        let black_pieces = board.color_combined(Color::Black);

        let mut attackers =
            chess::get_king_moves(target_square) & blockers & board.pieces(Piece::King)
                | chess::get_knight_moves(target_square) & blockers & board.pieces(Piece::Knight)
                | chess::get_rook_moves(target_square, blockers)
                    & blockers
                    & (board.pieces(Piece::Rook) | board.pieces(Piece::Queen))
                | chess::get_bishop_moves(target_square, blockers)
                    & blockers
                    & (board.pieces(Piece::Bishop) | board.pieces(Piece::Queen))
                | chess::get_pawn_attacks(target_square, Color::Black, *white_pieces)
                | chess::get_pawn_attacks(target_square, Color::White, *black_pieces);

        let mut target_piece = board.piece_on(capture.get_source()).unwrap();
        let mut colour = !initial_colour;
        let mut gains = vec![piece_value(initial_capture)];

        'exchange: loop {
            for attacker_piece in [
                Piece::Pawn,
                Piece::Knight,
                Piece::Bishop,
                Piece::Rook,
                Piece::Queen,
                Piece::King,
            ] {
                let our_attacker =
                    attackers & board.color_combined(colour) & board.pieces(attacker_piece);

                if our_attacker == chess::EMPTY {
                    continue;
                }

                let attacker_square = our_attacker.to_square();
                let victim_value = piece_value(target_piece);
                gains.push(victim_value);

                if target_piece == Piece::King {
                    break;
                }

                blockers ^= BitBoard::from_square(attacker_square);
                attackers ^= BitBoard::from_square(attacker_square);

                target_piece = attacker_piece;

                if matches!(attacker_piece, Piece::Rook | Piece::Queen) {
                    attackers |= chess::get_rook_moves(target_square, blockers)
                        & blockers
                        & (board.pieces(Piece::Rook) | board.pieces(Piece::Queen));
                }

                if matches!(attacker_piece, Piece::Pawn | Piece::Bishop | Piece::Queen) {
                    attackers |= chess::get_bishop_moves(target_square, blockers)
                        & blockers
                        & (board.pieces(Piece::Bishop) | board.pieces(Piece::Queen));
                }

                colour = !colour;

                continue 'exchange;
            }

            while gains.len() > 1 {
                let forced = gains.len() == 2;

                let their_gain = gains.pop().unwrap();
                let our_gain = gains.last_mut().unwrap();

                *our_gain -= their_gain;

                if !forced && *our_gain < 0 {
                    *our_gain = 0;
                }
            }

            return Value::new(gains.pop().unwrap());
        }
    }
}

/// Piece values for use with MVV/LVA only.
const fn piece_value(piece: Piece) -> i16 {
    match piece {
        Piece::Pawn => 100,
        Piece::Knight => 320,
        Piece::Bishop => 330,
        Piece::Rook => 500,
        Piece::Queen => 900,
        Piece::King => 950,
    }
}

impl From<MoveGen> for MoveVec {
    fn from(move_gen: MoveGen) -> Self {
        Self(move_gen.map(MoveEntry::new).collect())
    }
}

impl std::ops::Deref for MoveVec {
    type Target = Vec<MoveEntry>;

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl std::ops::DerefMut for MoveVec {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    macro_rules! test_see {
        ($name:ident, $fen:expr, $capture:expr, $expected:expr) => {
            #[test]
            fn $name() {
                let board: Board = $fen.parse().unwrap();
                let capture: ChessMove = $capture.parse().unwrap();
                let expected = Value::new($expected);

                assert_eq!(expected, MoveVec::see(&board, capture));
            }
        };
    }

    test_see!(
        test_see_pawn_takes_protected_pawn,
        "4R3/2r3p1/5bk1/1p1r3p/p2PR1P1/P1BK4/1P6/8 b - - 0 1",
        "h5g4",
        0
    );
    test_see!(
        test_see_two_pawn_takes_doubly_protocted_pawn,
        "4R3/2r3p1/5bk1/1p1r1p1p/p2PR1P1/P1BK1P2/1P6/8 b - - 0 1",
        "h5g4",
        0
    );

    //#[test]
    //fn test_see_enough_attackers_but_wrong_order() {
    //    let board: Board = "k2r2q1/2n2b2/2p5/3n4/2K1P3/3QNB2/3R4/8 w - - 0 1"
    //        .parse()
    //        .unwrap();
    //    let capture: ChessMove = "e4d5".parse().unwrap();
    //
    //    //assert_eq!(Value::INFINITE, MoveVec::see(&board, capture));
    //
    //    for capture in MoveGen::new_legal(&board) {
    //        if capture.captures(&board).is_none() {
    //            continue;
    //        }
    //
    //        println!(
    //            "{} SEE: {}",
    //            capture,
    //            MoveVec::see(&board, capture).as_inner()
    //        );
    //    }
    //    panic!();
    //}
}
