#![allow(dead_code)]
use chess::ChessMove;

use crate::value;

pub type MoveVec = Vec<MoveEntry>;

#[derive(Debug)]
pub struct MoveEntry {
    pub mv: ChessMove,
    pub value: value::Value,
    pub pv: Vec<ChessMove>,
}

impl MoveEntry {
    pub fn new(mv: ChessMove) -> Self {
        Self {
            mv,
            value: -value::INFINITE,
            pv: vec![mv],
        }
    }
}
