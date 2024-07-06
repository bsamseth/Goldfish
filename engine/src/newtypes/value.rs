use derive_more::{Add, AddAssign, FromStr, Neg, Sub, SubAssign};

use super::Ply;

/// A [`Value`] represents a score in centipawns, or a "checkmate in x plies" value.
#[derive(
    Debug,
    Clone,
    Copy,
    Default,
    PartialEq,
    Eq,
    PartialOrd,
    Ord,
    Add,
    AddAssign,
    Sub,
    SubAssign,
    Neg,
    FromStr,
)]
pub struct Value(Inner);

/// The inner type of a [`Value`].
///
/// This is a signed 16-bit integer, which is the smallest integer type capable of representing the
/// maximum material differnece in a chess game in centipawns. It should be small so that
/// transposition table entries can be small.
type Inner = i16;

impl Value {
    /// The [`Value`] of a draw.
    pub const DRAW: Self = Self(0);

    /// The constant `1` as a [`Value`].
    pub const ONE: Self = Self(1);

    /// The [`Value`] w.r.t. the winning side of a board with a checkmate position.
    pub const CHECKMATE: Self = Self(30000);

    /// The smallest possible [`Value`] which is still a checkmate score.
    pub const CHECKMATE_THRESHOLD: Value = Self::mate_in(Ply::MAX);

    /// The [`Value`] of a known winning position, determined by endgame tablebases.
    pub const KNOWN_WIN: Self = Self(Self::CHECKMATE_THRESHOLD.0 - 1);

    /// The smallest possible [`Value`] which is still a known win score.
    pub const KNOWN_WIN_THRESHOLD: Value = Self::known_win_in(Ply::MAX);

    /// A [`Value`] greater than all other values.
    ///
    /// The negative of this value is a value smaller than all other values.
    pub const INFINITE: Value = Self(Inner::MAX);

    /// Create a new [`Value`] from an inner `i16`.
    pub const fn new(inner: Inner) -> Self {
        Self(inner)
    }

    /// Return the inner `i16` of this [`Value`].
    pub const fn as_inner(self) -> Inner {
        self.0
    }

    // Increment this [`Value`] by one.
    pub const fn increment(self) -> Self {
        Self(self.0 + 1)
    }

    // Decrement this [`Value`] by one.
    pub const fn decrement(self) -> Self {
        Self(self.0 - 1)
    }

    /// A win in `ply` halfmoves.
    pub const fn mate_in(ply: Ply) -> Self {
        Self(Self::CHECKMATE.0 - ply.as_inner() as Inner)
    }

    /// A loss in `ply` halfmoves.
    pub const fn mated_in(ply: Ply) -> Self {
        Self(-Self::CHECKMATE.0 + (ply.as_inner() as Inner))
    }

    /// A known win in `ply` halfmoves.
    ///
    /// That is, in `ply` halfmoves the position is known to be a win.
    pub const fn known_win_in(ply: Ply) -> Self {
        Self(Self::KNOWN_WIN.0 - (ply.as_inner() as Inner))
    }

    /// A known loss in `ply` halfmoves.
    ///
    /// That is, in `ply` halfmoves the position is known to be a loss.
    pub const fn known_loss_in(ply: Ply) -> Self {
        Self(-Self::KNOWN_WIN.0 + (ply.as_inner() as Inner))
    }

    /// Return `true` if this [`Value`] is a checkmate score.
    pub const fn is_checkmate(self) -> bool {
        self.is_winning_checkmate() || self.is_losing_checkmate()
    }

    /// Return `true` if this [`Value`] is a winning checkmate score.
    pub const fn is_winning_checkmate(self) -> bool {
        self.0 >= Self::CHECKMATE_THRESHOLD.0
    }

    /// Return `true` if this [`Value`] is a losing checkmate score.
    pub const fn is_losing_checkmate(self) -> bool {
        self.0 <= -Self::CHECKMATE_THRESHOLD.0
    }

    /// Return `true` if this [`Value`] is a known win score.
    pub const fn is_known_win(self) -> bool {
        self.0 >= Self::KNOWN_WIN_THRESHOLD.0
    }

    /// Return `true` if this [`Value`] is a known loss score.
    pub const fn is_known_loss(self) -> bool {
        self.0 <= -Self::KNOWN_WIN_THRESHOLD.0
    }

    /// Return the number of plies until checkmate.
    ///
    /// If the [`Value`] is not a checkmate score, this returns `None`.
    pub const fn mate_distance(self) -> Option<usize> {
        if self.is_winning_checkmate() {
            debug_assert!(self.0 <= Self::CHECKMATE.0);
            #[allow(clippy::cast_sign_loss)]
            Some((Self::CHECKMATE.0 - self.0) as usize)
        } else {
            None
        }
    }
    /// Return the number of plies until a known win.
    ///
    /// If the [`Value`] is not a known win, this returns `None`.
    pub const fn known_win_distance(self) -> Option<usize> {
        if self.is_known_win() {
            debug_assert!(self.0 <= Self::KNOWN_WIN.0);
            #[allow(clippy::cast_sign_loss)]
            Some((Self::KNOWN_WIN.0 - self.0) as usize)
        } else {
            None
        }
    }

    /// Return the signed number of full moves until checkmate.
    ///
    /// This is the number of full moves until checkmate, with the same sign as the [`Value`].
    ///
    /// # Safety
    /// This should only be called if `self.is_checkmate() == true`.
    /// Otherwise the behaviour is undefined.
    pub const fn mate_depth(self) -> i16 {
        debug_assert!(self.is_checkmate());
        let abs_depth = (Value::CHECKMATE.0 - self.0.abs() + 1) / 2;
        if self.0 > 0 {
            abs_depth
        } else {
            -abs_depth
        }
    }
}

impl From<fathom::Wdl> for Value {
    fn from(value: fathom::Wdl) -> Self {
        match value {
            fathom::Wdl::Win => Value::KNOWN_WIN,
            fathom::Wdl::Loss => -Value::KNOWN_WIN,
            _ => Value::DRAW,
        }
    }
}

/// A utility trait for operations involving [`Value`]s.
#[allow(clippy::module_name_repetitions)]
pub trait ValueExt {
    fn scaled_by(self, factor: Value) -> Self;
}

impl ValueExt for i32 {
    fn scaled_by(self, factor: Value) -> Self {
        self * i32::from(factor.0)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_mate_distance() {
        assert_eq!(Value::CHECKMATE.mate_distance(), Some(0));
        assert_eq!(Value::CHECKMATE_THRESHOLD.mate_distance(), Some(255));
        assert_eq!(Value::KNOWN_WIN.mate_distance(), None);
        assert_eq!(Value::KNOWN_WIN_THRESHOLD.mate_distance(), None);
        assert_eq!(Value::DRAW.mate_distance(), None);
        assert_eq!(Value::ONE.mate_distance(), None);
        assert_eq!(Value::mate_in(Ply::MAX).mate_distance(), Some(255));
        assert_eq!(Value::mate_in(Ply::new(42)).mate_distance(), Some(42));
        assert_eq!((-Value::mated_in(Ply::new(42))).mate_distance(), Some(42));
        assert_eq!(Value::mated_in(Ply::MAX).mate_distance(), None);
        assert_eq!(Value::mated_in(Ply::ONE).mate_distance(), None);
    }
}
