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

type Inner = u8;

impl Depth {
    /// The maximum search depth considered by the engine from any given position.
    ///
    /// This could likely be almost [`u8::max`], but we choose a smaller value to avoid having to
    /// consider the possibility of overflow in the search code. Any search stopping due to
    /// reaching the maximum depth will be served just as well by stopping here.
    pub const MAX: Self = Self(127);

    #[cfg(test)]
    pub const ZERO: Self = Self(0);

    /// Create a new [`Depth`] from an inner `u8`.
    pub const fn new(inner: Inner) -> Self {
        Self(inner)
    }

    /// Convert this [`Depth`] to a `usize`.
    ///
    /// This is useful for indexing arrays, or in other (const) context where a `usize` is needed.
    /// The inner type of [`Depth`] is smaller than `usize`, so this conversion is trivial and a
    /// noop at runtime.
    pub const fn as_usize(self) -> usize {
        self.0 as usize
    }

    /// Convert this [`Depth`] to its inner type.
    pub const fn as_inner(self) -> Inner {
        self.0
    }
}

impl From<usize> for Depth {
    /// Create a new [`Depth`] from a `usize`, saturating at the maximum depth.
    fn from(depth: usize) -> Self {
        let clamped = depth.min(Self::MAX.as_usize());

        // This is safe because we just clamped the value to the maximum depth.
        #[allow(clippy::cast_possible_truncation)]
        Self::new(clamped as Inner)
    }
}
