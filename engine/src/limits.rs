use crate::{
    newtypes::{Depth, Ply},
    opts::OPTS,
};
use chess::Board;
use uci::GoOption;

#[derive(Debug)]
pub struct Limits {
    pub depth: Depth,
    pub nodes: usize,
    pub movetime: usize,
    pub mate: Option<Ply>,
    pub moves_to_go: Option<usize>,
    pub w_time_left: Option<usize>,
    pub b_time_left: Option<usize>,
    pub w_time_inc: Option<usize>,
    pub b_time_inc: Option<usize>,
}

impl Default for Limits {
    fn default() -> Self {
        Self {
            depth: Depth::MAX,
            nodes: usize::MAX,
            movetime: usize::MAX,
            mate: None,
            moves_to_go: None,
            w_time_left: None,
            b_time_left: None,
            w_time_inc: None,
            b_time_inc: None,
        }
    }
}

impl From<&[GoOption]> for Limits {
    fn from(options: &[GoOption]) -> Self {
        let mut limits = Limits::default();
        for option in options {
            match option {
                GoOption::Infinite => return Limits::default(),
                GoOption::Depth(depth) => limits.depth = Depth::from(*depth),
                GoOption::Nodes(nodes) => limits.nodes = limits.nodes.min(*nodes),
                GoOption::MoveTime(movetime) => limits.movetime = limits.movetime.min(*movetime),
                GoOption::Mate(full_moves) if *full_moves > 0 => {
                    limits.mate = Some((*full_moves * 2 - 1).into());
                }
                GoOption::MovesToGo(x) => limits.moves_to_go = Some(*x),
                GoOption::WTime(x) => limits.w_time_left = Some(*x),
                GoOption::BTime(x) => limits.b_time_left = Some(*x),
                GoOption::WInc(x) => limits.w_time_inc = Some(*x),
                GoOption::BInc(x) => limits.b_time_inc = Some(*x),
                _ => {}
            }
        }

        limits
    }
}

impl Limits {
    /// Apply time control to the limits.
    ///
    /// This function will adjust the `movetime` field of the limits based on the time control.
    /// If no time control is in place, then the limits are returned unchanged.
    pub fn with_time_control(mut self, board: &Board) -> Self {
        let (time_left, time_inc) = match board.side_to_move() {
            chess::Color::White => (
                if let Some(x) = self.w_time_left {
                    x
                } else {
                    // If no time left is given, then no time control is in place.
                    return self;
                },
                self.w_time_inc.unwrap_or(0),
            ),
            chess::Color::Black => (
                if let Some(x) = self.b_time_left {
                    x
                } else {
                    // If no time left is given, then no time control is in place.
                    return self;
                },
                self.b_time_inc.unwrap_or(0),
            ),
        };

        // How much time do we have to spend on the remaining moves?
        let max_search_time = time_left * OPTS.tc_time_buffer_percentage / 100;

        if max_search_time < OPTS.tc_min_search_time_ms {
            self.movetime = 1; // Return a move as soon as possible.
            return self;
        }

        // Assume this many moves left in the game/until time control.
        let moves_to_go = self.moves_to_go.unwrap_or(OPTS.tc_default_moves_to_go);
        assert!(
            moves_to_go > 0,
            "moves_to_go must be positive, UCI protocol violation"
        );

        let time_per_move = (max_search_time + (moves_to_go - 1) * time_inc) / moves_to_go;

        // In case of large increments, don't spend more time than we have left on the clock right now.
        let time_per_move = time_per_move.min(max_search_time);

        self.movetime = time_per_move;
        self
    }
}
