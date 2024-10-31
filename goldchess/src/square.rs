use std::str::FromStr;

use crate::{Bitboard, Error, File, Rank, Result};

/// A square on the chessboard.
///
/// Squares are indexed from 0 to 63, where `A1=0`, `B1=1`, ..., `H8=63`.
///
/// You can get [`File`] and [`Rank`] from a square, and combining squares with the `|` operator
/// yields a [`Bitboard`].
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Square(pub u8);

impl Square {
    /// Create a new square from a `u8` without checking if it is a valid square index.
    ///
    /// # Safety
    /// The input must be a valid square index, i.e. 0 <= sq < 64.
    #[must_use]
    pub unsafe fn new_unchecked(sq: u8) -> Self {
        Self(sq)
    }

    /// Create a new square from a `u8`.
    ///
    /// # Errors
    /// Returns an error if the input is not a valid square index.
    pub fn new(sq: u8) -> Result<Self, Error> {
        if sq < 64 {
            Ok(Self(sq))
        } else {
            Err(Error::InvalidSquare(sq))
        }
    }

    #[must_use]
    pub fn file(self) -> File {
        // SAFETY: The modulo operation ensures that the result is always in 0..8.
        unsafe { File::new_unchecked(self.0 % 8) }
    }

    #[must_use]
    pub fn rank(self) -> Rank {
        // SAFETY: self is a valid square index, so self.0 / 8 is a valid rank index.
        unsafe { Rank::new_unchecked(self.0 / 8) }
    }
}

#[allow(dead_code)]
impl Square {
    pub const A1: Square = Square(0);
    pub const B1: Square = Square(1);
    pub const C1: Square = Square(2);
    pub const D1: Square = Square(3);
    pub const E1: Square = Square(4);
    pub const F1: Square = Square(5);
    pub const G1: Square = Square(6);
    pub const H1: Square = Square(7);
    pub const A2: Square = Square(8);
    pub const B2: Square = Square(9);
    pub const C2: Square = Square(10);
    pub const D2: Square = Square(11);
    pub const E2: Square = Square(12);
    pub const F2: Square = Square(13);
    pub const G2: Square = Square(14);
    pub const H2: Square = Square(15);
    pub const A3: Square = Square(16);
    pub const B3: Square = Square(17);
    pub const C3: Square = Square(18);
    pub const D3: Square = Square(19);
    pub const E3: Square = Square(20);
    pub const F3: Square = Square(21);
    pub const G3: Square = Square(22);
    pub const H3: Square = Square(23);
    pub const A4: Square = Square(24);
    pub const B4: Square = Square(25);
    pub const C4: Square = Square(26);
    pub const D4: Square = Square(27);
    pub const E4: Square = Square(28);
    pub const F4: Square = Square(29);
    pub const G4: Square = Square(30);
    pub const H4: Square = Square(31);
    pub const A5: Square = Square(32);
    pub const B5: Square = Square(33);
    pub const C5: Square = Square(34);
    pub const D5: Square = Square(35);
    pub const E5: Square = Square(36);
    pub const F5: Square = Square(37);
    pub const G5: Square = Square(38);
    pub const H5: Square = Square(39);
    pub const A6: Square = Square(40);
    pub const B6: Square = Square(41);
    pub const C6: Square = Square(42);
    pub const D6: Square = Square(43);
    pub const E6: Square = Square(44);
    pub const F6: Square = Square(45);
    pub const G6: Square = Square(46);
    pub const H6: Square = Square(47);
    pub const A7: Square = Square(48);
    pub const B7: Square = Square(49);
    pub const C7: Square = Square(50);
    pub const D7: Square = Square(51);
    pub const E7: Square = Square(52);
    pub const F7: Square = Square(53);
    pub const G7: Square = Square(54);
    pub const H7: Square = Square(55);
    pub const A8: Square = Square(56);
    pub const B8: Square = Square(57);
    pub const C8: Square = Square(58);
    pub const D8: Square = Square(59);
    pub const E8: Square = Square(60);
    pub const F8: Square = Square(61);
    pub const G8: Square = Square(62);
    pub const H8: Square = Square(63);
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
        sq.0
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
        Bitboard(1 << sq.0)
    }
}

impl FromStr for Square {
    type Err = Error;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        let file = File::from_str(&s[0..1])?;
        let rank = Rank::from_str(&s[1..2])?;
        Ok(Self(rank.0 * 8 + file.0))
    }
}
