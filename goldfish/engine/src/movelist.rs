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

    pub fn new_with_rating(mv: ChessMove, board: &chess::Board) -> Self {
        let mut entry = Self::new(mv);
        entry.rate_move(board);
        entry
    }

    /// Rate move base on MVV/LVA.
    ///
    /// Most valuable victim / least valuable attacker (MVV/LVA) is a heuristic for ordering captures.
    /// The idea is to capture the most valuable victim with the least valuable attacker.
    /// Not a perfect heuristic, but much better than random/generation order.
    fn rate_move(&mut self, board: &chess::Board) {
        self.value = Value::new(0);

        if let Some(target) = board.piece_on(self.mv.get_dest()) {
            self.value = Value::new(
                10 * piece_value(target)
                    - piece_value(
                        board
                            .piece_on(self.mv.get_source())
                            .expect("there should always be a source piece for a move"),
                    ),
            );
        }
    }
}

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

impl MoveVec {
    pub fn new_from_moves(move_gen: MoveGen, board: &Board) -> Self {
        Self(
            move_gen
                .map(|mv| MoveEntry::new_with_rating(mv, board))
                .collect(),
        )
    }

    pub fn new_from_board(board: &Board) -> Self {
        Self::new_from_moves(MoveGen::new_legal(board), board)
    }

    pub fn rate_moves(&mut self, board: &chess::Board) {
        self.iter_mut().for_each(|entry| entry.rate_move(board));
    }

    pub fn sort_moves(&mut self) {
        self.0.sort_by_key(|m| -m.value);
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
