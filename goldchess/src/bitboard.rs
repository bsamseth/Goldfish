use crate::Square;

/// A 64-bit bitboard.
///
/// The underlying `u64` *is* available, but all useful operations/methods should be implemented.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Bitboard(pub u64);

impl Bitboard {
    /// Create a new bitboard from a `u64`.
    #[must_use]
    pub const fn new(bits: u64) -> Self {
        Self(bits)
    }

    #[must_use]
    pub fn count_ones(self) -> u32 {
        self.0.count_ones()
    }

    #[must_use]
    pub fn is_empty(self) -> bool {
        self.0 == 0
    }

    #[must_use]
    pub fn trailing_zeros(self) -> u32 {
        self.0.trailing_zeros()
    }

    #[must_use]
    pub fn pop_least_significant_square(&mut self) -> Option<Square> {
        if self.0 == 0 {
            return None;
        }
        let lsb = self.trailing_zeros();
        *self &= self.0 - 1;
        #[allow(
            clippy::cast_possible_truncation,
            reason = "lsb < 64 because of trailing_zeros and non-zero self"
        )]
        Some(unsafe { Square::new_unchecked(lsb as u8) })
    }
}

impl Bitboard {
    pub const FULL: Bitboard = Bitboard(u64::MAX);
    pub const EMPTY: Bitboard = Bitboard(0);
    pub const EDGES: Bitboard =
        Bitboard(Bitboard::RANK_1.0 | Bitboard::RANK_8.0 | Bitboard::FILE_A.0 | Bitboard::FILE_H.0);
    pub const WHITE_SQUARES: Bitboard = Bitboard(0x55AA_55AA_55AA_55AA);
    pub const BLACK_SQUARES: Bitboard = Bitboard(0xAA55_AA55_AA55_AA55);

    pub const RANK_1: Bitboard = Bitboard(0xFF);
    pub const RANK_2: Bitboard = Bitboard(0xFF00);
    pub const RANK_3: Bitboard = Bitboard(0x00FF_0000);
    pub const RANK_4: Bitboard = Bitboard(0xFF00_0000);
    pub const RANK_5: Bitboard = Bitboard(0x00FF_0000_0000);
    pub const RANK_6: Bitboard = Bitboard(0xFF00_0000_0000);
    pub const RANK_7: Bitboard = Bitboard(0x00FF_0000_0000_0000);
    pub const RANK_8: Bitboard = Bitboard(0xFF00_0000_0000_0000);

    pub const FILE_A: Bitboard = Bitboard(0x0101_0101_0101_0101);
    pub const FILE_B: Bitboard = Bitboard(0x0202_0202_0202_0202);
    pub const FILE_C: Bitboard = Bitboard(0x0404_0404_0404_0404);
    pub const FILE_D: Bitboard = Bitboard(0x0808_0808_0808_0808);
    pub const FILE_E: Bitboard = Bitboard(0x1010_1010_1010_1010);
    pub const FILE_F: Bitboard = Bitboard(0x2020_2020_2020_2020);
    pub const FILE_G: Bitboard = Bitboard(0x4040_4040_4040_4040);
    pub const FILE_H: Bitboard = Bitboard(0x8080_8080_8080_8080);
}

impl From<u64> for Bitboard {
    fn from(bits: u64) -> Bitboard {
        Bitboard(bits)
    }
}

impl<T: Into<Bitboard>> std::ops::BitOr<T> for Bitboard {
    type Output = Bitboard;

    fn bitor(self, rhs: T) -> Bitboard {
        let rhs = rhs.into();
        Bitboard(self.0 | rhs.0)
    }
}

impl<T: Into<Bitboard>> std::ops::BitOrAssign<T> for Bitboard {
    fn bitor_assign(&mut self, rhs: T) {
        *self = *self | rhs;
    }
}

impl<T: Into<Bitboard>> std::ops::BitAnd<T> for Bitboard {
    type Output = Bitboard;

    fn bitand(self, rhs: T) -> Bitboard {
        let rhs = rhs.into();
        Bitboard(self.0 & rhs.0)
    }
}

impl<T: Into<Bitboard>> std::ops::BitAndAssign<T> for Bitboard {
    fn bitand_assign(&mut self, rhs: T) {
        *self = *self & rhs;
    }
}

impl<T: Into<Bitboard>> std::ops::BitXor<T> for Bitboard {
    type Output = Bitboard;

    fn bitxor(self, rhs: T) -> Bitboard {
        let rhs = rhs.into();
        Bitboard(self.0 ^ rhs.0)
    }
}

impl<T: Into<Bitboard>> std::ops::BitXorAssign<T> for Bitboard {
    fn bitxor_assign(&mut self, rhs: T) {
        *self = *self ^ rhs;
    }
}

impl std::ops::Not for Bitboard {
    type Output = Bitboard;

    fn not(self) -> Bitboard {
        Bitboard(!self.0)
    }
}

impl std::fmt::Display for Bitboard {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        for rank in (0..8).rev() {
            for file in 0..8 {
                let sq = rank * 8 + file;
                if self.0 & (1 << sq) != 0 {
                    write!(f, "1")?;
                } else {
                    write!(f, "0")?;
                }
            }
            writeln!(f)?;
        }
        Ok(())
    }
}

impl IntoIterator for Bitboard {
    type Item = Square;
    type IntoIter = _Iterator;

    fn into_iter(self) -> Self::IntoIter {
        _Iterator(self)
    }
}

pub struct _Iterator(Bitboard);

impl Iterator for _Iterator {
    type Item = Square;

    fn next(&mut self) -> Option<Self::Item> {
        self.0.pop_least_significant_square()
    }
}
