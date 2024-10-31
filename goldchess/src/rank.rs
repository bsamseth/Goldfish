use std::str::FromStr;

use crate::{Bitboard, Error, Result};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Rank(pub u8);

impl Rank {
    /// Create a new rank from a `u8`.
    ///
    /// # Safety
    /// The input must be a valid rank index, i.e. 0 <= rank < 8.
    #[must_use]
    pub unsafe fn new_unchecked(rank: u8) -> Self {
        Self(rank)
    }

    /// Create a new rank from a `u8`.
    ///
    /// # Errors
    /// Returns an error if the input is not a valid rank index.
    pub fn new(rank: u8) -> Result<Self, Error> {
        if rank < 8 {
            Ok(Self(rank))
        } else {
            Err(Error::InvalidRank(rank))
        }
    }

    /// Get the rank above this one, if it exists.
    #[must_use]
    pub fn up(self) -> Option<Self> {
        if self.0 == 7 {
            None
        } else {
            Some(Self(self.0 + 1))
        }
    }

    /// Get the rank below this one, if it exists.
    #[must_use]
    pub fn down(self) -> Option<Self> {
        if self.0 == 0 {
            None
        } else {
            Some(Self(self.0 - 1))
        }
    }
}

impl Rank {
    pub const R1: Rank = Rank(0);
    pub const R2: Rank = Rank(1);
    pub const R3: Rank = Rank(2);
    pub const R4: Rank = Rank(3);
    pub const R5: Rank = Rank(4);
    pub const R6: Rank = Rank(5);
    pub const R7: Rank = Rank(6);
    pub const R8: Rank = Rank(7);
}

impl From<Rank> for u8 {
    fn from(rank: Rank) -> u8 {
        rank.0
    }
}

impl TryFrom<u8> for Rank {
    type Error = Error;

    fn try_from(rank: u8) -> Result<Self> {
        Rank::new(rank)
    }
}

impl From<Rank> for Bitboard {
    fn from(rank: Rank) -> Bitboard {
        Bitboard(0xffu64 << (8 * rank.0))
    }
}

impl FromStr for Rank {
    type Err = Error;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s
            .chars()
            .next()
            .ok_or(Error::InvalidRankChar(std::char::REPLACEMENT_CHARACTER))?
        {
            '1' | 'A' => Ok(Rank::R1),
            '2' | 'B' => Ok(Rank::R2),
            '3' | 'C' => Ok(Rank::R3),
            '4' | 'D' => Ok(Rank::R4),
            '5' | 'E' => Ok(Rank::R5),
            '6' | 'F' => Ok(Rank::R6),
            '7' | 'G' => Ok(Rank::R7),
            '8' | 'H' => Ok(Rank::R8),
            other => Err(Error::InvalidRankChar(other)),
        }
    }
}
