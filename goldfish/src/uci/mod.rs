mod comm;
mod types;

use chess::Game;

#[allow(dead_code)]
#[derive(Debug)]
pub struct Uci {
    game: Game,
}
