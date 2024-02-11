use derive_more::{Add, AddAssign, Neg, Sub, SubAssign};

use super::Ply;

/// A [`Value`] represents a score in centipawns, or a "checkmate in x plies" value.
#[derive(
    Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Add, AddAssign, Sub, SubAssign, Neg,
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

    /// The [`Value`] w.r.t. the winning side of a board with a checkmate position.
    pub const CHECKMATE: Self = Self(30000);

    /// The smallest possible [`Value`] which is still a checkmate score.
    pub const CHECKMATE_THRESHOLD: Value = Self::mate_in(Ply::MAX);

    /// A [`Value`] greater than all other values.
    ///
    /// The negative of this value is a value smaller than all other values.
    pub const INFINITE: Value = Self(Inner::MAX - 1);

    /// Create a new [`Value`] from an inner `i16`.
    pub const fn new(inner: Inner) -> Self {
        Self(inner)
    }

    /// Return the inner `i16` of this [`Value`].
    pub const fn as_inner(self) -> Inner {
        self.0
    }

    /// A win in `ply` halfmoves.
    pub const fn mate_in(ply: Ply) -> Self {
        Self(Self::CHECKMATE.0 - (ply.as_inner() as Inner))
    }

    /// A loss in `ply` halfmoves.
    pub const fn mated_in(ply: Ply) -> Self {
        Self(-Self::CHECKMATE.0 + (ply.as_inner() as Inner))
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

    /// Return the signed number of full moves until checkmate.
    ///
    /// This is the number of full moves until checkmate, with the same sign as the [`Value`].
    ///
    /// # Safety
    /// This should only be called if `self.is_checkmate() == true`.
    /// Otherwise the behaviour is undefined.
    pub const fn mate_depth(self) -> i16 {
        let abs_depth = (Value::CHECKMATE.0 - self.0.abs() + 1) / 2;
        if self.0 > 0 {
            abs_depth
        } else {
            -abs_depth
        }
    }
}
