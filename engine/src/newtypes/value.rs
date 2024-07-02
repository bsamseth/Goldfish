use derive_more::{Add, AddAssign, FromStr, Neg, Sub, SubAssign};

use super::{nonmax::NonMaxI16, Ply};

/// A [`Value`] represents a score in centipawns, or a "checkmate in x plies" value.
///
/// This type allows the niche optimization such that it can be wrapped in [`Option`] without
/// increasing its size. This is important for storing [`Value`]s in transposition tables.
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
type Inner = NonMaxI16;
type Innerst = i16;

impl Value {
    /// The [`Value`] of a draw.
    pub const DRAW: Self = Self(unsafe { Inner::new_unchecked(0) });

    /// The [`Value`] w.r.t. the winning side of a board with a checkmate position.
    pub const CHECKMATE: Self = Self(unsafe { Inner::new_unchecked(30000) });

    /// The smallest possible [`Value`] which is still a checkmate score.
    pub const CHECKMATE_THRESHOLD: Value = Self::mate_in(Ply::MAX);

    /// The [`Value`] of a known winning position, determined by endgame tablebases.
    pub const KNOWN_WIN: Self =
        Self(unsafe { Inner::new_unchecked(Self::CHECKMATE_THRESHOLD.0.get() - 1) });

    /// The smallest possible [`Value`] which is still a known win score.
    pub const KNOWN_WIN_THRESHOLD: Value = Self::known_win_in(Ply::MAX);

    /// A [`Value`] greater than all other values.
    ///
    /// The negative of this value is a value smaller than all other values.
    pub const INFINITE: Value = Self(Inner::MAX);

    /// Create a new [`Value`] from an inner `i16`.
    pub const fn new(innerst: Innerst) -> Option<Self> {
        if let Some(inner) = Inner::new(innerst) {
            Some(Self(inner))
        } else {
            None
        }
    }

    /// Return the inner `i16` of this [`Value`].
    pub const fn as_inner(self) -> Inner {
        self.0
    }

    /// A win in `ply` halfmoves.
    pub const fn mate_in(ply: Ply) -> Self {
        // SAFETY: `CHECKMATE` is a valid checkmate score, as so is `CHECKMATE - Nu8`.
        Self(unsafe { Inner::new_unchecked(Self::CHECKMATE.0.get() - (ply.as_inner() as Innerst)) })
    }

    /// A loss in `ply` halfmoves.
    pub const fn mated_in(ply: Ply) -> Self {
        // SAFETY: `-CHECKMATE` is a valid checkmate score, as so is `-CHECKMATE + Nu8`.
        Self(unsafe {
            Inner::new_unchecked(-Self::CHECKMATE.0.get() + (ply.as_inner() as Innerst))
        })
    }

    /// A known win in `ply` halfmoves.
    ///
    /// That is, in `ply` halfmoves the position is known to be a win.
    pub const fn known_win_in(ply: Ply) -> Self {
        // SAFETY: `KNOWN_WIN` is a valid checkmate score, as so is `KNOWN_WIN - Nu8`.
        Self(unsafe { Inner::new_unchecked(Self::KNOWN_WIN.0.get() - (ply.as_inner() as Innerst)) })
    }

    /// A known loss in `ply` halfmoves.
    ///
    /// That is, in `ply` halfmoves the position is known to be a loss.
    pub const fn known_loss_in(ply: Ply) -> Self {
        // SAFETY: `-KNOWN_WIN` is a valid checkmate score, as so is `-KNOWN_WIN + Nu8`.
        Self(unsafe {
            Inner::new_unchecked(-Self::KNOWN_WIN.0.get() + (ply.as_inner() as Innerst))
        })
    }

    /// Return `true` if this [`Value`] is a checkmate score.
    pub fn is_checkmate(self) -> bool {
        self.is_winning_checkmate() || self.is_losing_checkmate()
    }

    /// Return `true` if this [`Value`] is a winning checkmate score.
    pub fn is_winning_checkmate(self) -> bool {
        self.0 >= Self::CHECKMATE_THRESHOLD.0
    }

    /// Return `true` if this [`Value`] is a losing checkmate score.
    pub fn is_losing_checkmate(self) -> bool {
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
        let abs_depth = (Value::CHECKMATE.0.get() - self.0.get().abs() + 1) / 2;
        if self.0.get() > 0 {
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
        self * i32::from(factor.0.get())
    }
}
