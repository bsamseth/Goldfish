mod comm;
mod stop_signal;
mod types;

use chess::Game;

pub use stop_signal::StopSignal;
pub use types::GoOption;

#[allow(dead_code)]
#[derive(Debug)]
pub struct Uci {
    game: Game,
}
