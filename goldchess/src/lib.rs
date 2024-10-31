//! # Goldchess - A chess library
//!
//! <p align="center">
//!   <img src="https://github.com/bsamseth/goldfish/blob/main/logo.jpg?raw=true" width=60% alt="Logo, a goldfish playing chess."/>
//! </p>
//!
//! This is a library useful for writing chess engines in Rust. Its primary reason to exist is to
//! support the [Goldfish](https://github.com/bsamseth/goldfish) chess engine, but very well might be useful for other projects as well.
mod bitboard;
mod color;
mod error;
mod file;
mod generated_tables;
mod piece;
mod rank;
mod square;

pub use bitboard::Bitboard;
pub use color::Color;
pub use error::{Error, Result};
pub use file::File;
pub use piece::Piece;
pub use rank::Rank;
pub use square::Square;
