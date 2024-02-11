use crate::newtypes::{Depth, Ply};
use uci::GoOption;

#[derive(Debug)]
pub struct Limits {
    pub depth: Depth,
    pub nodes: usize,
    pub movetime: usize,
    pub mate: Option<Ply>,
}

impl From<&[GoOption]> for Limits {
    fn from(options: &[GoOption]) -> Self {
        let mut limits = Limits {
            depth: Depth::MAX,
            nodes: usize::MAX,
            movetime: usize::MAX,
            mate: None,
        };

        for option in options {
            match option {
                GoOption::Depth(depth) => limits.depth = Depth::from(*depth),
                GoOption::Nodes(nodes) => limits.nodes = limits.nodes.min(*nodes),
                GoOption::MoveTime(movetime) => limits.movetime = limits.movetime.min(*movetime),
                GoOption::Mate(full_moves) if *full_moves > 0 => {
                    limits.mate = Some((*full_moves * 2 - 1).into());
                }
                _ => {}
            }
        }
        limits
    }
}
