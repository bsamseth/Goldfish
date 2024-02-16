#![allow(dead_code)]
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
            entry.value = Value::new(0);

            if let Some(target) = board.piece_on(entry.mv.get_dest()) {
                entry.value = Value::new(
                    10 * piece_value(target)
                        - piece_value(
                            board
                                .piece_on(entry.mv.get_source())
                                .expect("there should always be a source piece for a move"),
                        ),
                );
            }
        }
        self
    }

    pub fn sort(&mut self) {
        self.0.sort_by_key(|m| -m.value);
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
