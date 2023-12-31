mod controller;
mod evaluate;
mod movelist;
mod search;
mod value;

pub use value::{Depth, Value};

use crate::uci::{GoOption, StopSignal};
use controller::Controller;
use movelist::{MoveEntry, MoveVec};

use chess::{Board, Game, MoveGen};

#[derive(Debug)]
pub struct Engine {
    board: Board,
    root_moves: MoveVec,
    pub controller: Controller,
}

impl Engine {
    pub fn new(game: Game, options: Vec<GoOption>, stop_signal: StopSignal) -> Self {
        todo!()
    }

    pub fn new_from_board(board: Board) -> Self {
        let controller = Controller::default();
        let moves = MoveGen::new_legal(&board);
        let mut root_moves = MoveVec::new();

        for mv in moves {
            root_moves.push(MoveEntry::new(mv));
        }

        Self {
            board,
            root_moves,
            controller,
        }
    }
}
