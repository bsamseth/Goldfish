use chess::{Board, ChessMove, MoveGen, Piece};

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
