mod comm;
mod types;

use crate::engine::Engine;
use chess::Game;

#[allow(dead_code)]
#[derive(Debug)]
pub struct Uci {
    game: Game,
    engine: Engine,
}
