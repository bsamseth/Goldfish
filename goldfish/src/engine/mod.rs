mod evaluate;
mod movelist;
mod value;

use movelist::{MoveEntry, MoveVec};
pub use value::{Depth, Value};

use chess::{Game, MoveGen};

#[derive(Debug)]
pub struct Engine {
    game: Game,
    root_moves: MoveVec,
}

impl Engine {
    pub fn new() -> Self {
        Self::new_from_game(Game::new())
    }

    pub fn new_from_game(game: Game) -> Self {
        let moves = MoveGen::new_legal(&game.current_position());
        let mut root_moves = MoveVec::new();

        for mv in moves {
            root_moves.push(MoveEntry {
                mv,
                value: value::INFINITE,
                pv: vec![mv],
            });
        }

        Self { game, root_moves }
    }
}
