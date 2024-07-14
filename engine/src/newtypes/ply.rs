use derive_more::{Add, AddAssign, FromStr, Sub, SubAssign};

/// A [`Ply`] represents a halfmove in a chess game.
#[derive(
    Debug,
    Clone,
    Copy,
    Default,
    PartialEq,
    Eq,
    Add,
    AddAssign,
    Sub,
    SubAssign,
    PartialOrd,
    Ord,
    FromStr,
)]
pub struct Ply(Inner);

type Inner = u8;

impl Ply {
    /// The maximum number of plies considered by the engine from any given position.
    ///
    /// There are games longer than 255 halfmoves, but from any given position the engine will
    /// never consider more than 255 halfmoves. In practice it will never consider nearly that many
    /// moves, but for optimization purposes we limit this at a constant with sufficient headroom.
    pub const MAX: Self = Self(255);

    /// Create a new [`Ply`] from an inner `u8`.
    pub const fn new(inner: Inner) -> Self {
        Self(inner)
    }

    /// Increment this [`Ply`] by one.
    pub const fn increment(self) -> Self {
        Self(self.0 + 1)
    }

    /// Decrease this [`Ply`] by one.
    pub const fn decrement(self) -> Self {
        Self(self.0 - 1)
    }

    /// Convert this [`Ply`] to a `usize`.
    ///
    /// This is useful for indexing arrays, or in other (const) context where a `usize` is needed.
    /// The inner type of [`Ply`] is smaller than `usize`, so this conversion is trivial and a
    /// noop at runtime.
    pub const fn as_usize(self) -> usize {
        self.0 as usize
    }

    /// Convert this [`Ply`] to its inner type.
    pub const fn as_inner(self) -> Inner {
        self.0
    }

    pub const ZERO: Self = Self(0);
    pub const ONE: Self = Self(1);
}

impl From<usize> for Ply {
    /// Create a new [`Ply`] from a `usize`, saturating at the maximum depth.
    fn from(depth: usize) -> Self {
        let clamped = depth.min(Self::MAX.as_usize());

        // This is safe because we just clamped the value to the maximum depth.
        #[allow(clippy::cast_possible_truncation)]
        Self::new(clamped as Inner)
    }
}
