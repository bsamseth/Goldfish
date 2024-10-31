use std::str::FromStr;

use crate::{Bitboard, Error, Result};

/// Represent a file (column) on a chess board.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct File(pub u8);

impl File {
    /// Create a new file from a `u8`.
    ///
    /// # Safety
    /// The input must be a valid file index, i.e. 0 <= file < 8.
    #[must_use]
    pub unsafe fn new_unchecked(file: u8) -> Self {
        Self(file)
    }

    /// Create a new file from a `u8`.
    ///
    /// # Errors
    /// Returns an error if the input is not a valid file index.
    pub fn new(file: u8) -> Result<Self, Error> {
        if file < 8 {
            Ok(Self(file))
        } else {
            Err(Error::InvalidFile(file))
        }
    }

    /// Get the file to the left of this one, if it exists.
    #[must_use]
    pub fn left(self) -> Option<Self> {
        if self.0 == 0 {
            None
        } else {
            Some(Self(self.0 - 1))
        }
    }

    /// Get the file to the right of this one, if it exists.
    #[must_use]
    pub fn right(self) -> Option<Self> {
        if self.0 == 7 {
            None
        } else {
            Some(Self(self.0 + 1))
        }
    }
}

impl File {
    pub const A: File = File(0);
    pub const B: File = File(1);
    pub const C: File = File(2);
    pub const D: File = File(3);
    pub const E: File = File(4);
    pub const F: File = File(5);
    pub const G: File = File(6);
    pub const H: File = File(7);
}

impl From<File> for u8 {
    fn from(file: File) -> u8 {
        file.0
    }
}
impl TryFrom<u8> for File {
    type Error = Error;

    fn try_from(file: u8) -> Result<Self> {
        File::new(file)
    }
}

impl From<File> for Bitboard {
    fn from(file: File) -> Bitboard {
        match file {
            File::A => Bitboard::FILE_A,
            File::B => Bitboard::FILE_B,
            File::C => Bitboard::FILE_C,
            File::D => Bitboard::FILE_D,
            File::E => Bitboard::FILE_E,
            File::F => Bitboard::FILE_F,
            File::G => Bitboard::FILE_G,
            File::H => Bitboard::FILE_H,
            _ => unreachable!(),
        }
    }
}

impl FromStr for File {
    type Err = Error;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s
            .chars()
            .next()
            .ok_or(Error::InvalidFileChar(std::char::REPLACEMENT_CHARACTER))?
        {
            'a' | 'A' => Ok(File::A),
            'b' | 'B' => Ok(File::B),
            'c' | 'C' => Ok(File::C),
            'd' | 'D' => Ok(File::D),
            'e' | 'E' => Ok(File::E),
            'f' | 'F' => Ok(File::F),
            'g' | 'G' => Ok(File::G),
            'h' | 'H' => Ok(File::H),
            other => Err(Error::InvalidFileChar(other)),
        }
    }
}
