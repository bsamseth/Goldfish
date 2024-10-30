//! Function and utilities to generate move tables.
//!
//! The structures and definitions here are only what makes implementing the generation easier. This
//! Deliberately does not depend on the rest of [`goldchess`], which defined the same things (and more)
//! in a more complete manner. This is because [`goldchess`] will not depend on this code, only on a
//! static file with the generated tables. This way magics can be generated once as opposed to on
//! every build.
#![allow(clippy::missing_panics_doc)]

use std::fmt::Display;

use rand::Rng;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Bitboard(pub u64);
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Square(pub u8);
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum Color {
    White = 0,
    Black = 1,
}

impl Square {
    /// Create a new square from a `u8`.
    ///
    /// # Safety
    /// The input must be a valid square index, i.e. 0 <= sq < 64.
    #[must_use]
    pub unsafe fn new(sq: u8) -> Self {
        Self(sq)
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

impl Square {
    fn get_file(self) -> u8 {
        self.0 % 8
    }
    fn get_rank(self) -> u8 {
        self.0 / 8
    }
}

impl Display for Square {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        let file = self.0 % 8;
        let rank = self.0 / 8;
        write!(f, "{}{}", (b'A' + file) as char, rank + 1)
    }
}

impl Bitboard {
    pub const EMPTY: Bitboard = Bitboard(0);
    pub const FULL: Bitboard = Bitboard(u64::MAX);
    pub const EDGES: Bitboard = Bitboard(
        0b1111_1111_1000_0001_1000_0001_1000_0001_1000_0001_1000_0001_1000_0001_1111_1111_u64,
    );
}

impl std::ops::BitOr for Square {
    type Output = Bitboard;

    fn bitor(self, rhs: Square) -> Bitboard {
        Bitboard::from(self) | Bitboard::from(rhs)
    }
}

impl From<Square> for Bitboard {
    fn from(sq: Square) -> Bitboard {
        Bitboard(1 << sq.0)
    }
}

impl std::ops::BitOrAssign<u64> for Bitboard {
    fn bitor_assign(&mut self, rhs: u64) {
        self.0 |= rhs;
    }
}
impl std::ops::BitOrAssign for Bitboard {
    fn bitor_assign(&mut self, rhs: Bitboard) {
        self.0 |= rhs.0;
    }
}
impl std::ops::BitAnd for Bitboard {
    type Output = Bitboard;

    fn bitand(self, rhs: Bitboard) -> Bitboard {
        Bitboard(self.0 & rhs.0)
    }
}
impl std::ops::Not for Bitboard {
    type Output = Bitboard;

    fn not(self) -> Bitboard {
        Bitboard(!self.0)
    }
}
impl<T: Into<Bitboard>> std::ops::BitOr<T> for Bitboard {
    type Output = Bitboard;

    fn bitor(self, rhs: T) -> Bitboard {
        let rhs = rhs.into();
        Bitboard(self.0 | rhs.0)
    }
}
impl std::ops::Not for Color {
    type Output = Color;

    fn not(self) -> Color {
        match self {
            Color::White => Color::Black,
            Color::Black => Color::White,
        }
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

fn rank_diff(sq1: Square, sq2: Square) -> i8 {
    let Square(sq1) = sq1;
    let Square(sq2) = sq2;
    #[allow(clippy::cast_possible_wrap)] // 0..64 / 8 fits in i8.
    let diff = (sq1 / 8) as i8 - (sq2 / 8) as i8;
    diff
}

fn file_diff(sq1: Square, sq2: Square) -> i8 {
    let Square(sq1) = sq1;
    let Square(sq2) = sq2;
    #[allow(clippy::cast_possible_wrap)] // 0..64 % 8 fits in i8.
    let diff = (sq1 % 8) as i8 - (sq2 % 8) as i8;
    diff
}

impl Bitboard {
    fn count_ones(self) -> u32 {
        self.0.count_ones()
    }
    fn pop_least_significant_square(&mut self) -> Option<Square> {
        if self.0 == 0 {
            return None;
        }
        let lsb = self.0.trailing_zeros();
        self.0 &= self.0 - 1;
        #[allow(
            clippy::cast_possible_truncation,
            reason = "lsb < 64 because of trailing_zeros and non-zero self"
        )]
        Some(unsafe { Square::new(lsb as u8) })
    }
}

#[must_use]
pub fn king_moves(at: Square) -> Bitboard {
    let mut bb = Bitboard(0);
    for sq in 0..64 {
        let sq = unsafe { Square::new(sq) };
        let rdiff = rank_diff(at, sq);
        let fdiff = file_diff(at, sq);
        if at != sq && rdiff.abs() <= 1 && fdiff.abs() <= 1 {
            bb |= Bitboard::from(sq);
        }
    }
    bb
}

#[must_use]
pub fn knight_moves(at: Square) -> Bitboard {
    let mut bb = Bitboard(0);
    for sq in 0..64 {
        let sq = unsafe { Square::new(sq) };
        let rdiff = rank_diff(at, sq);
        let fdiff = file_diff(at, sq);
        if at != sq && rdiff.abs() <= 2 && fdiff.abs() <= 2 && rdiff.abs() + fdiff.abs() == 3 {
            bb |= Bitboard::from(sq);
        }
    }
    bb
}

#[must_use]
pub fn pawn_attacks(at: Square, color: Color) -> Bitboard {
    let mut bb = Bitboard(0);
    let forward = match color {
        Color::White => 8,
        Color::Black => -8,
    };

    #[allow(clippy::cast_possible_wrap)] // 0..64 fits in i8.
    let forward = at.0 as i8 + forward;
    if !(0..=63).contains(&forward) {
        return Bitboard(0);
    }
    #[allow(clippy::cast_sign_loss)] // 0..64 fits in u8.
    let forward = forward as u8;

    if (at.0 % 8) != 0 {
        bb |= Bitboard::from(unsafe { Square::new(forward - 1) });
    }
    if (at.0 % 8) != 7 {
        bb |= Bitboard::from(unsafe { Square::new(forward + 1) });
    }

    bb
}

#[must_use]
pub fn pawn_quiets(at: Square, color: Color) -> Bitboard {
    let mut bb = Bitboard(0);
    let forward = match color {
        Color::White => 8,
        Color::Black => -8,
    };

    #[allow(clippy::cast_possible_wrap)] // 0..64 fits in i8.
    let forward = at.0 as i8 + forward;
    if !(0..=63).contains(&forward) {
        return Bitboard(0);
    }
    #[allow(clippy::cast_sign_loss)] // 0..64 fits in u8.
    let forward = forward as u8;

    bb |= Bitboard::from(unsafe { Square::new(forward) });

    if color == Color::White && at.0 / 8 == 1 {
        bb |= Bitboard::from(unsafe { Square::new(forward + 8) });
    } else if color == Color::Black && at.0 / 8 == 6 {
        bb |= Bitboard::from(unsafe { Square::new(forward - 8) });
    }

    bb
}

pub const BISHOP: bool = true;
pub const ROOK: bool = false;
pub type SliderType = bool;

#[must_use]
pub fn attack_mask<const S: SliderType>(at: Square) -> Bitboard {
    let mut bb = Bitboard(0);
    if S == BISHOP {
        for sq in Square::ALL_SQUARES {
            let rdiff = rank_diff(at, sq);
            let fdiff = file_diff(at, sq);
            if at != sq && rdiff.abs() == fdiff.abs() {
                bb |= Bitboard::from(sq);
            }
        }
    } else {
        for sq in Square::ALL_SQUARES {
            let rdiff = rank_diff(at, sq);
            let fdiff = file_diff(at, sq);
            if at != sq && (rdiff == 0 || fdiff == 0) {
                bb |= Bitboard::from(sq);
            }
        }
    }
    bb
}

#[must_use]
pub fn occupancy_mask<const S: SliderType>(at: Square) -> Bitboard {
    let bb = attack_mask::<S>(at);
    if S == BISHOP {
        return bb & !Bitboard::EDGES;
    }
    // Rook
    let ignore = Square::ALL_SQUARES
        .iter()
        .filter(|edge| {
            (at.get_rank() == edge.get_rank() && (edge.get_file() == 0 || edge.get_file() == 7))
                || (at.get_file() == edge.get_file()
                    && (edge.get_rank() == 0 || edge.get_rank() == 7))
        })
        .fold(Bitboard::EMPTY, |b, s| b | *s);
    bb & !ignore
}

#[must_use]
pub fn attacks_from<const S: SliderType>(at: Square, occupancy: Bitboard) -> Bitboard {
    let mut bb = Bitboard(0);
    if S == BISHOP {
        for rd in [-1, 1] {
            for fd in [-1, 1] {
                let mut sq = at;
                #[allow(clippy::cast_possible_wrap)]
                #[allow(clippy::cast_sign_loss)]
                loop {
                    let sq_unchecked = (sq.0 as i8) + rd * 8 + fd;

                    if !(0..64).contains(&sq_unchecked) {
                        break;
                    }

                    sq = unsafe { Square::new(sq_unchecked as u8) };

                    if file_diff(at, sq).abs() != rank_diff(at, sq).abs() {
                        break;
                    }

                    bb |= Bitboard::from(sq);

                    if (occupancy & Bitboard::from(sq)) != Bitboard::EMPTY {
                        break;
                    }
                }
            }
        }
    } else {
        for d in [1, 8] {
            for sign in [-1, 1] {
                let mut sq = at;
                #[allow(clippy::cast_possible_wrap)]
                #[allow(clippy::cast_sign_loss)]
                loop {
                    let sq_unchecked = (sq.0 as i8) + d * sign;

                    if !(0..64).contains(&sq_unchecked) {
                        break;
                    }

                    sq = unsafe { Square::new(sq_unchecked as u8) };

                    if (file_diff(at, sq) == 0) == (rank_diff(at, sq) == 0) {
                        break;
                    }

                    bb |= Bitboard::from(sq);

                    if (occupancy & Bitboard::from(sq)) != Bitboard::EMPTY {
                        break;
                    }
                }
            }
        }
    }
    bb
}

#[must_use]
pub fn number_of_relevant_occupancy_bits<const S: SliderType>(at: Square) -> u32 {
    occupancy_mask::<S>(at).count_ones()
}

#[must_use]
pub fn occupancy_for_mask_and_index(mut mask: Bitboard, index: usize) -> Bitboard {
    let mut bb = Bitboard(0);
    for bit_nr in 0..mask.count_ones() {
        let sq = mask
            .pop_least_significant_square()
            .expect("Mask is not empty");

        if index & (1 << bit_nr) != 0 {
            bb |= Bitboard::from(sq);
        }
    }
    bb
}

fn random_candidate() -> u64 {
    let mut rng = rand::thread_rng();
    rng.gen::<u64>() & rng.gen::<u64>() & rng.gen::<u64>()
}

#[must_use]
pub fn find_magic<const S: SliderType>(at: Square) -> u64 {
    let relevant_bits = number_of_relevant_occupancy_bits::<S>(at);
    let occ_mask = occupancy_mask::<S>(at);
    let occupancy_count = 1 << relevant_bits;
    let occupancies = (0..occupancy_count)
        .map(|index| occupancy_for_mask_and_index(occ_mask, index))
        .collect::<Vec<_>>();
    let attacks = occupancies
        .iter()
        .map(|occ| attacks_from::<S>(at, *occ))
        .collect::<Vec<_>>();

    let mut used = vec![Bitboard::EMPTY; occupancy_count];
    'outer: loop {
        let magic_number = random_candidate();
        if (occ_mask.0.wrapping_mul(magic_number) & 0xFF00_0000_0000_0000).count_ones() < 6 {
            continue;
        }
        used.fill(Bitboard::EMPTY);

        for (index, occupancy) in occupancies.iter().enumerate() {
            #[allow(clippy::cast_possible_truncation)]
            let magic_index =
                (occupancy.0.wrapping_mul(magic_number) >> (64 - relevant_bits)) as usize;

            if used[magic_index] == Bitboard::EMPTY {
                used[magic_index] = attacks[index];
            } else if used[magic_index] != attacks[index] {
                continue 'outer;
            }
        }

        return magic_number;
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_king_moves() {
        let cases = [Square::A1, Square::A8, Square::H1, Square::H8, Square::E4];
        let expected = [
            Square::B1 | Square::A2 | Square::B2,
            Square::A7 | Square::B8 | Square::B7,
            Square::G1 | Square::H2 | Square::G2,
            Square::H7 | Square::G8 | Square::G7,
            Square::D3
                | Square::E3
                | Square::F3
                | Square::D4
                | Square::F4
                | Square::D5
                | Square::E5
                | Square::F5,
        ];

        for (case, expected) in cases.iter().zip(expected.iter()) {
            let bb = king_moves(*case);
            assert_eq!(
                bb, *expected,
                "King moves from {case:?}: got:\n{bb} expected:\n{expected}"
            );
        }
    }

    #[test]
    fn test_knight_moves() {
        let cases = [
            Square::A1,
            Square::A8,
            Square::H1,
            Square::H8,
            Square::G7,
            Square::B1,
            Square::B8,
            Square::G1,
            Square::G8,
        ];
        let expected = [
            Square::C2 | Square::B3,
            Square::C7 | Square::B6,
            Square::F2 | Square::G3,
            Square::F7 | Square::G6,
            Square::F5 | Square::H5 | Square::F5 | Square::E6 | Square::E8,
            Square::A3 | Square::C3 | Square::D2,
            Square::A6 | Square::C6 | Square::D7,
            Square::F3 | Square::H3 | Square::E2,
            Square::F6 | Square::H6 | Square::E7,
        ];

        for (case, expected) in cases.iter().zip(expected.iter()) {
            let bb = knight_moves(*case);
            assert_eq!(
                bb, *expected,
                "Knight moves from {case:?}: got:\n{bb} expected:\n{expected}"
            );
        }
    }

    #[test]
    fn test_pawn_moves() {
        let cases = [
            (Square::A2, Color::White),
            (Square::A7, Color::Black),
            (Square::H2, Color::White),
            (Square::H7, Color::Black),
            (Square::E2, Color::White),
            (Square::E3, Color::White),
            (Square::D4, Color::Black),
        ];
        let expected = [
            Square::A3 | Square::A4 | Square::B3,
            Square::A6 | Square::A5 | Square::B6,
            Square::H3 | Square::H4 | Square::G3,
            Square::H6 | Square::H5 | Square::G6,
            Square::D3 | Square::F3 | Square::E3 | Square::E4,
            Square::D4 | Square::F4 | Square::E4,
            Square::D3 | Square::C3 | Square::E3,
        ];

        for ((case, color), expected) in cases.iter().zip(expected.iter()) {
            let bb = pawn_attacks(*case, *color) | pawn_quiets(*case, *color);
            assert_eq!(
                bb, *expected,
                "Pawn moves from {case:?} as {color:?}: got:\n{bb} expected:\n{expected}"
            );
        }
    }

    #[test]
    fn test_bishop_attack_mask() {
        let cases = [Square::A1, Square::A8, Square::H1, Square::H8, Square::E4];
        let expected = [
            Square::B2
                | Square::C3
                | Square::D4
                | Square::E5
                | Square::F6
                | Square::G7
                | Square::H8,
            Square::B7
                | Square::C6
                | Square::D5
                | Square::E4
                | Square::F3
                | Square::G2
                | Square::H1,
            Square::A8
                | Square::B7
                | Square::C6
                | Square::D5
                | Square::E4
                | Square::F3
                | Square::G2,
            Square::A1
                | Square::B2
                | Square::C3
                | Square::D4
                | Square::E5
                | Square::F6
                | Square::G7,
            Square::D3
                | Square::C2
                | Square::B1
                | Square::F3
                | Square::G2
                | Square::D5
                | Square::A8
                | Square::H1
                | Square::C6
                | Square::B7
                | Square::F5
                | Square::G6
                | Square::H7,
        ];
        for (case, expected) in cases.iter().zip(expected.iter()) {
            let bb = attack_mask::<BISHOP>(*case);
            assert_eq!(
                bb, *expected,
                "Bishop attack mask from {case:?}: got:\n{bb} expected:\n{expected}"
            );
        }
    }

    #[test]
    fn test_rook_attack_mask() {
        let cases = [Square::A1, Square::A8, Square::H1, Square::H8, Square::E4];
        let expected = [
            Square::A2
                | Square::A3
                | Square::A4
                | Square::A5
                | Square::A6
                | Square::A7
                | Square::A8
                | Square::B1
                | Square::C1
                | Square::D1
                | Square::E1
                | Square::F1
                | Square::G1
                | Square::H1,
            Square::A1
                | Square::A2
                | Square::A3
                | Square::A4
                | Square::A5
                | Square::A6
                | Square::A7
                | Square::B8
                | Square::C8
                | Square::D8
                | Square::E8
                | Square::F8
                | Square::G8
                | Square::H8,
            Square::H2
                | Square::H3
                | Square::H4
                | Square::H5
                | Square::H6
                | Square::H7
                | Square::H8
                | Square::A1
                | Square::B1
                | Square::C1
                | Square::D1
                | Square::E1
                | Square::F1
                | Square::G1,
            Square::A8
                | Square::B8
                | Square::C8
                | Square::D8
                | Square::E8
                | Square::F8
                | Square::G8
                | Square::H2
                | Square::H3
                | Square::H4
                | Square::H5
                | Square::H6
                | Square::H1
                | Square::H7,
            Square::E1
                | Square::E2
                | Square::E3
                | Square::E5
                | Square::E6
                | Square::E7
                | Square::E8
                | Square::A4
                | Square::B4
                | Square::C4
                | Square::D4
                | Square::F4
                | Square::H4
                | Square::G4,
        ];

        for (case, expected) in cases.iter().zip(expected.iter()) {
            let bb = attack_mask::<ROOK>(*case);
            assert_eq!(
                bb, *expected,
                "Rook attack mask from {case:?}: got:\n{bb} expected:\n{expected}"
            );
        }
    }

    #[test]
    fn test_occupancy_for_index() {
        let mask = attack_mask::<BISHOP>(Square::E4);
        let bb = occupancy_for_mask_and_index(mask, 511);
        let expected = Bitboard(4_569_847_841_922);
        assert_eq!(bb, expected,);
    }

    #[test]
    fn test_relevant_bishop_bits() {
        assert_eq!(6, number_of_relevant_occupancy_bits::<BISHOP>(Square::A1));
        assert_eq!(5, number_of_relevant_occupancy_bits::<BISHOP>(Square::A2));
        assert_eq!(9, number_of_relevant_occupancy_bits::<BISHOP>(Square::D5));
    }

    #[test]
    fn test_relevant_rook_bits() {
        assert_eq!(12, number_of_relevant_occupancy_bits::<ROOK>(Square::A1));
        assert_eq!(11, number_of_relevant_occupancy_bits::<ROOK>(Square::A2));
        assert_eq!(10, number_of_relevant_occupancy_bits::<ROOK>(Square::D5));
    }

    #[test]
    fn test_bishop_attacks_from() {
        let bb = attacks_from::<BISHOP>(Square::E4, Bitboard::EMPTY);
        assert_eq!(bb, Bitboard(108_724_279_602_332_802));
        let bb = attacks_from::<BISHOP>(Square::E4, Square::C6 | Square::F3);
        assert_eq!(bb, Bitboard(36_103_735_610_967_042));
    }
    #[test]
    fn test_rook_attacks_from() {
        let bb = attacks_from::<ROOK>(Square::A1, Bitboard::EMPTY);
        assert_eq!(bb, Bitboard(72_340_172_838_076_926));
        let bb = attacks_from::<ROOK>(Square::A1, Square::A6 | Square::G1);
        assert_eq!(bb, Bitboard(1_103_823_438_206));
    }
}
