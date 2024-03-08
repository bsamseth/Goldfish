use chess::{Board, ChessMove, MoveGen, Piece};

use crate::newtypes::Value;

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
    pub fn mvv_lva_rated(mut self, board: &Board) -> Self {
        for entry in self.iter_mut() {
            let mut value = 0i16;

            // In case of promotions, add the value of the promoted piece to the move value.
            if let Some(piece) = entry.mv.get_promotion() {
                value += 10 * (piece_value(piece) - piece_value(Piece::Pawn));
            }

            // Capture? Prefer captures with higher value victims, by lower value attackers.
            if let Some(target) = board.piece_on(entry.mv.get_dest()) {
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

    pub fn sort(&mut self) {
        self.sort_by_key(|m| -m.value);
    }

    pub fn sorted(mut self) -> Self {
        self.sort();
        self
    }

    /// Sorts the moves with the given preferences first, then by value.
    ///
    /// The preferences, if valid moves, are placed first in the given order. Then the
    /// rest of the moves are sorted by value.
    pub fn sort_with_preference(
        mut self,
        preferences: impl IntoIterator<Item = ChessMove>,
    ) -> Self {
        // Place preferences first in the given order, then sort the rest by value.
        let mut swaps = 0;
        for (i, mv) in preferences.into_iter().enumerate() {
            if i < self.len() {
                if let Some(index) = self.iter().position(|m| m.mv == mv) {
                    swaps += 1;
                    self.swap(i, index);
                }
            }
        }

        let swaps = swaps.min(self.len());
        let (_, rest) = self.split_at_mut(swaps);
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
