use crate::value::{Depth, MAX_PLY};
use uci::GoOption;

#[derive(Debug)]
pub struct Limits {
    pub depth: Depth,
    pub nodes: usize,
    pub movetime: usize,
}

impl From<&[GoOption]> for Limits {
    fn from(options: &[GoOption]) -> Self {
        let mut limits = Limits {
            depth: MAX_PLY,
            nodes: usize::MAX,
            movetime: usize::MAX,
        };

        for option in options {
            match option {
                GoOption::Depth(depth) => {
                    let depth = Depth::try_from(*depth).unwrap_or(MAX_PLY);
                    limits.depth = depth.min(limits.depth);
                }
                GoOption::Nodes(nodes) => limits.nodes = limits.nodes.min(*nodes),
                GoOption::MoveTime(movetime) => limits.movetime = limits.movetime.min(*movetime),
                _ => {}
            }
        }
        limits
    }
}
