use std::{num::NonZeroU8, str::FromStr};

use crate::{Bitboard, Error, File, Rank, Result};

/// A square on the chessboard.
///
/// Squares are indexed from 1 to 64, where `A1=1`, `B1=2`, ..., `H8=64`. Yes, that's right,
/// one-based indexing. This enables the niche optimization, where [`Option<Square>`] is still
/// only the size of a single `u8`.
///
/// You can get [`File`] and [`Rank`] from a square, and combining squares with the `|` operator
/// yields a [`Bitboard`].
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Square(NonZeroU8);

impl Square {
    /// Create a new square from a `u8` without checking if it is a valid square index.
    ///
    /// # Safety
    /// The input must be a valid square index, i.e. 1 <= sq <= 64.
    #[must_use]
    pub const unsafe fn new_unchecked(sq: u8) -> Self {
        Self(NonZeroU8::new_unchecked(sq))
    }

    /// Create a new square from a `u8`.
    ///
    /// # Errors
    /// Returns an error if the input is not a valid square index.
    pub const fn new(sq: u8) -> Result<Self, Error> {
        if sq > 0 && sq <= 64 {
            // SAFETY: sq is in 1..=64, so it is safe to create.
            Ok(unsafe { Self::new_unchecked(sq) })
        } else {
            Err(Error::InvalidSquare(sq))
        }
    }

    #[must_use]
    pub const fn file(self) -> File {
        // SAFETY: The modulo operation ensures that the result is always in 0..8.
        unsafe { File::new_unchecked((self.0.get() - 1) % 8) }
    }

    #[must_use]
    pub const fn rank(self) -> Rank {
        // SAFETY: self is a valid square index, so (self.0 - 1) / 8 is a valid rank index.
        unsafe { Rank::new_unchecked((self.0.get() - 1) / 8) }
    }
}

#[allow(dead_code)]
impl Square {
    pub const A1: Square = unsafe { Square::new_unchecked(1) };
    pub const B1: Square = unsafe { Square::new_unchecked(2) };
    pub const C1: Square = unsafe { Square::new_unchecked(3) };
    pub const D1: Square = unsafe { Square::new_unchecked(4) };
    pub const E1: Square = unsafe { Square::new_unchecked(5) };
    pub const F1: Square = unsafe { Square::new_unchecked(6) };
    pub const G1: Square = unsafe { Square::new_unchecked(7) };
    pub const H1: Square = unsafe { Square::new_unchecked(8) };
    pub const A2: Square = unsafe { Square::new_unchecked(9) };
    pub const B2: Square = unsafe { Square::new_unchecked(10) };
    pub const C2: Square = unsafe { Square::new_unchecked(11) };
    pub const D2: Square = unsafe { Square::new_unchecked(12) };
    pub const E2: Square = unsafe { Square::new_unchecked(13) };
    pub const F2: Square = unsafe { Square::new_unchecked(14) };
    pub const G2: Square = unsafe { Square::new_unchecked(15) };
    pub const H2: Square = unsafe { Square::new_unchecked(16) };
    pub const A3: Square = unsafe { Square::new_unchecked(17) };
    pub const B3: Square = unsafe { Square::new_unchecked(18) };
    pub const C3: Square = unsafe { Square::new_unchecked(19) };
    pub const D3: Square = unsafe { Square::new_unchecked(20) };
    pub const E3: Square = unsafe { Square::new_unchecked(21) };
    pub const F3: Square = unsafe { Square::new_unchecked(22) };
    pub const G3: Square = unsafe { Square::new_unchecked(23) };
    pub const H3: Square = unsafe { Square::new_unchecked(24) };
    pub const A4: Square = unsafe { Square::new_unchecked(25) };
    pub const B4: Square = unsafe { Square::new_unchecked(26) };
    pub const C4: Square = unsafe { Square::new_unchecked(27) };
    pub const D4: Square = unsafe { Square::new_unchecked(28) };
    pub const E4: Square = unsafe { Square::new_unchecked(29) };
    pub const F4: Square = unsafe { Square::new_unchecked(30) };
    pub const G4: Square = unsafe { Square::new_unchecked(31) };
    pub const H4: Square = unsafe { Square::new_unchecked(32) };
    pub const A5: Square = unsafe { Square::new_unchecked(33) };
    pub const B5: Square = unsafe { Square::new_unchecked(34) };
    pub const C5: Square = unsafe { Square::new_unchecked(35) };
    pub const D5: Square = unsafe { Square::new_unchecked(36) };
    pub const E5: Square = unsafe { Square::new_unchecked(37) };
    pub const F5: Square = unsafe { Square::new_unchecked(38) };
    pub const G5: Square = unsafe { Square::new_unchecked(39) };
    pub const H5: Square = unsafe { Square::new_unchecked(40) };
    pub const A6: Square = unsafe { Square::new_unchecked(41) };
    pub const B6: Square = unsafe { Square::new_unchecked(42) };
    pub const C6: Square = unsafe { Square::new_unchecked(43) };
    pub const D6: Square = unsafe { Square::new_unchecked(44) };
    pub const E6: Square = unsafe { Square::new_unchecked(45) };
    pub const F6: Square = unsafe { Square::new_unchecked(46) };
    pub const G6: Square = unsafe { Square::new_unchecked(47) };
    pub const H6: Square = unsafe { Square::new_unchecked(48) };
    pub const A7: Square = unsafe { Square::new_unchecked(49) };
    pub const B7: Square = unsafe { Square::new_unchecked(50) };
    pub const C7: Square = unsafe { Square::new_unchecked(51) };
    pub const D7: Square = unsafe { Square::new_unchecked(52) };
    pub const E7: Square = unsafe { Square::new_unchecked(53) };
    pub const F7: Square = unsafe { Square::new_unchecked(54) };
    pub const G7: Square = unsafe { Square::new_unchecked(55) };
    pub const H7: Square = unsafe { Square::new_unchecked(56) };
    pub const A8: Square = unsafe { Square::new_unchecked(57) };
    pub const B8: Square = unsafe { Square::new_unchecked(58) };
    pub const C8: Square = unsafe { Square::new_unchecked(59) };
    pub const D8: Square = unsafe { Square::new_unchecked(60) };
    pub const E8: Square = unsafe { Square::new_unchecked(61) };
    pub const F8: Square = unsafe { Square::new_unchecked(62) };
    pub const G8: Square = unsafe { Square::new_unchecked(63) };
    pub const H8: Square = unsafe { Square::new_unchecked(64) };
    pub const ALL_SQUARES: [Square; 64] = [
        Self::A1,
        Self::B1,
        Self::C1,
        Self::D1,
        Self::E1,
        Self::F1,
        Self::G1,
        Self::H1,
        Self::A2,
        Self::B2,
        Self::C2,
        Self::D2,
        Self::E2,
        Self::F2,
        Self::G2,
        Self::H2,
        Self::A3,
        Self::B3,
        Self::C3,
        Self::D3,
        Self::E3,
        Self::F3,
        Self::G3,
        Self::H3,
        Self::A4,
        Self::B4,
        Self::C4,
        Self::D4,
        Self::E4,
        Self::F4,
        Self::G4,
        Self::H4,
        Self::A5,
        Self::B5,
        Self::C5,
        Self::D5,
        Self::E5,
        Self::F5,
        Self::G5,
        Self::H5,
        Self::A6,
        Self::B6,
        Self::C6,
        Self::D6,
        Self::E6,
        Self::F6,
        Self::G6,
        Self::H6,
        Self::A7,
        Self::B7,
        Self::C7,
        Self::D7,
        Self::E7,
        Self::F7,
        Self::G7,
        Self::H7,
        Self::A8,
        Self::B8,
        Self::C8,
        Self::D8,
        Self::E8,
        Self::F8,
        Self::G8,
        Self::H8,
    ];
}

impl std::fmt::Display for Square {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "{}{}", (b'A' + self.file().0) as char, self.rank().0 + 1)
    }
}

impl From<Square> for u8 {
    fn from(sq: Square) -> u8 {
        sq.0.get()
    }
}

impl TryFrom<u8> for Square {
    type Error = Error;

    fn try_from(sq: u8) -> Result<Self> {
        Square::new(sq)
    }
}

impl From<Square> for Rank {
    fn from(sq: Square) -> Rank {
        sq.rank()
    }
}

impl From<Square> for File {
    fn from(sq: Square) -> File {
        sq.file()
    }
}

impl std::ops::BitOr for Square {
    type Output = Bitboard;

    fn bitor(self, rhs: Square) -> Self::Output {
        Bitboard::from(self) | Bitboard::from(rhs)
    }
}

impl From<Square> for Bitboard {
    fn from(sq: Square) -> Bitboard {
        Bitboard(1 << (sq.0.get() - 1))
    }
}

impl FromStr for Square {
    type Err = Error;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        let file = File::from_str(&s[0..1])?;
        let rank = Rank::from_str(&s[1..2])?;
        // SAFETY: file and rank are valid, so the square is valid.
        Ok(unsafe { Self::new_unchecked(rank.0 * 8 + file.0 + 1) })
    }
}
