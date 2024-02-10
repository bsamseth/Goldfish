use crate::value::{Depth, MAX_PLY};
use uci::GoOption;

#[derive(Debug)]
pub struct Limits {
    pub depth: Depth,
    pub nodes: usize,
    pub movetime: usize,
    pub mate: Option<Depth>,
}

impl From<&[GoOption]> for Limits {
    fn from(options: &[GoOption]) -> Self {
        let mut limits = Limits {
            depth: MAX_PLY,
            nodes: usize::MAX,
            movetime: usize::MAX,
            mate: None,
        };

        for option in options {
            match option {
                GoOption::Depth(depth) => {
                    let depth = Depth::try_from(*depth).unwrap_or(MAX_PLY);
                    limits.depth = depth.min(limits.depth);
                }
                GoOption::Nodes(nodes) => limits.nodes = limits.nodes.min(*nodes),
                GoOption::MoveTime(movetime) => limits.movetime = limits.movetime.min(*movetime),
                GoOption::Mate(full_moves) if *full_moves > 0 => {
                    limits.mate = Some(Depth::try_from(*full_moves * 2 - 1).unwrap_or(MAX_PLY));
                }
                _ => {}
            }
        }
        limits
    }
}
