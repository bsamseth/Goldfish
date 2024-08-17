use chess::{Board, ChessMove, MoveGen, Piece};

use crate::chessmove::ChessMoveExt;
use crate::newtypes::Value;

#[derive(Debug)]
pub struct MoveVec<V = Value>(Vec<MoveEntry<V>>);

#[derive(Debug, Clone)]
pub struct MoveEntry<V = Value> {
    pub mv: ChessMove,
    pub value: V,
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

    pub fn sort(&mut self) {
        self.sort_by_key(|m| -m.value);
    }

    pub fn sorted(mut self) -> Self {
        self.sort();
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

impl<V> From<Vec<MoveEntry<V>>> for MoveVec<V> {
    fn from(vec: Vec<MoveEntry<V>>) -> Self {
        Self(vec)
    }
}

impl From<MoveGen> for MoveVec {
    fn from(move_gen: MoveGen) -> Self {
        Self(move_gen.map(MoveEntry::new).collect())
    }
}

impl<V> std::ops::Deref for MoveVec<V> {
    type Target = Vec<MoveEntry<V>>;

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl<V> std::ops::DerefMut for MoveVec<V> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}
