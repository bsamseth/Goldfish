use derive_more::{Add, AddAssign, FromStr, Sub, SubAssign};

/// A [`Depth`] represents a search depth in the engine.
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
pub struct Depth(Inner);

type Inner = i16;

impl Depth {
    /// The maximum search depth considered by the engine from any given position.
    pub const MAX: Self = Self(127);

    pub const ZERO: Self = Self(0);
    pub const ONE: Self = Self(1);

    /// Depth used to indicate that no searching was done at all. As such, it evaluates to less
    /// than any other search depth.
    pub const UNSEARCHED: Self = Self(-1);

    /// Create a new [`Depth`] .
    pub const fn new(inner: Inner) -> Self {
        Self(inner)
    }

    /// Decrement this [`Depth`] by one.
    pub const fn decrement(self) -> Self {
        Self(self.0 - 1)
    }

    /// Convert this [`Depth`] to a `usize`.
    ///
    /// This is useful for indexing arrays, or in other (const) context where a `usize` is needed.
    /// The inner type of [`Depth`] is smaller than `usize`, so this conversion is trivial and a
    /// noop at runtime.
    pub const fn as_usize(self) -> usize {
        debug_assert!(self.0 >= 0, "Depth cannot be negative");
        #[allow(clippy::cast_sign_loss)]
        let depth = self.0 as usize;
        depth
    }

    /// Convert this [`Depth`] to its inner type.
    pub const fn as_inner(self) -> Inner {
        self.0
    }
}

impl From<usize> for Depth {
    /// Create a new [`Depth`] from a `usize`, saturating at the maximum depth.
    fn from(depth: usize) -> Self {
        let clamped = depth.min(Self::MAX.as_usize()).max(0);

        // This is safe because we just clamped the value to the maximum depth.
        #[allow(clippy::cast_possible_truncation, clippy::cast_possible_wrap)]
        Self::new(clamped as Inner)
    }
}
