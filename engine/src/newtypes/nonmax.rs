//! [`NonMaxI16`] is a newtype that represents a signed 16-bit integer that is not equal to `i16::MAX`.
//!
//! This is the equivalent of `NonZeroI16` but with `i16::MAX` as the invalid value.
//! Only the [`i16`] variant is implemented, as this is the only one needed for the engine.

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct NonMaxI16(std::num::NonZeroI16);

impl NonMaxI16 {
    pub const fn new(n: i16) -> Option<Self> {
        if n == i16::MAX {
            return None;
        }
        // SAFETY: `n` is not `i16::MAX`.
        Some(unsafe { Self::new_unchecked(n) })
    }

    pub const unsafe fn new_unchecked(n: i16) -> Self {
        // SAFETY: Caller guarantees that `n` is not `i16::MAX`.
        Self(unsafe { std::num::NonZeroI16::new_unchecked(n ^ i16::MAX) })
    }

    pub const fn get(self) -> i16 {
        self.0.get() ^ i16::MAX
    }

    pub const MAX: Self = unsafe { Self::new_unchecked(i16::MAX - 1) };
    pub const MIN: Self = unsafe { Self::new_unchecked(i16::MIN) };
}

impl From<NonMaxI16> for i16 {
    fn from(n: NonMaxI16) -> i16 {
        n.get()
    }
}

impl From<u8> for NonMaxI16 {
    fn from(n: u8) -> Self {
        // SAFETY: `n` cannot be `i16::MAX`.
        unsafe { Self::new_unchecked(i16::from(n)) }
    }
}

impl std::fmt::Display for NonMaxI16 {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        i16::from(*self).fmt(f)
    }
}

impl Default for NonMaxI16 {
    fn default() -> Self {
        // SAFETY: `0` is not `i16::MAX`.
        unsafe { Self::new_unchecked(0) }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn nu(n: i16) -> NonMaxI16 {
        debug_assert_ne!(n, i16::MAX);
        // SAFETY: `n` is not `i16::MAX`.
        unsafe { NonMaxI16::new_unchecked(n) }
    }

    #[test]
    fn test_nonmaxi16_new() {
        assert_eq!(NonMaxI16::new(i16::MAX), None);
        assert_eq!(NonMaxI16::new(i16::MIN), Some(nu(i16::MIN)));
        assert_eq!(NonMaxI16::new(0), Some(nu(0)));
        assert_eq!(NonMaxI16::new(1), Some(nu(1)));
        assert_eq!(NonMaxI16::new(i16::MAX - 1), Some(nu(i16::MAX - 1)));
    }

    #[test]
    fn test_nonmaxi16_from() {
        assert_eq!(i16::from(nu(i16::MIN)), i16::MIN);
        assert_eq!(i16::from(nu(0)), 0);
        assert_eq!(i16::from(nu(1)), 1);
        assert_eq!(i16::from(nu(i16::MAX - 1)), i16::MAX - 1);
    }
}
