#![allow(dead_code)]

pub type Depth = u8;
pub type Value = i32;

pub const MAX_PLY: Depth = 255;

pub const ZERO: Value = 0;
pub const DRAW: Value = 0;
pub const CHECKMATE: Value = 30000;
pub const CHECKMATE_THRESHOLD: Value = CHECKMATE - MAX_PLY as Value;
pub const INFINITE: Value = 2 * CHECKMATE;
pub const NO_VALUE: Value = 3 * CHECKMATE;
pub const KNOWN_WIN: Value = CHECKMATE_THRESHOLD - 1;
pub const KNOWN_LOSS: Value = -KNOWN_WIN;
