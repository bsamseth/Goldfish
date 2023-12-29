use chess::ChessMove;

use crate::engine::Value;

pub type MoveVec = Vec<MoveEntry>;

#[derive(Debug)]
pub struct MoveEntry {
    pub mv: ChessMove,
    pub value: Value,
    pub pv: Vec<ChessMove>,
}
