#![allow(dead_code)]
use chess::{ChessMove, MoveGen};

use crate::value;

#[derive(Debug)]
pub struct MoveVec(Vec<MoveEntry>);

#[derive(Debug, Clone)]
pub struct MoveEntry {
    pub mv: ChessMove,
    pub value: value::Value,
}

impl MoveEntry {
    pub fn new(mv: ChessMove) -> Self {
        Self {
            mv,
            value: -value::INFINITE,
        }
    }
}

impl MoveVec {
    pub fn new_from_moves(move_gen: MoveGen) -> Self {
        Self(move_gen.map(MoveEntry::new).collect())
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
