//! [`NonMaxI16`] is a newtype that represents a signed 16-bit integer that is not equal to `i16::MAX`.
//!
//! This is the equivalent of `NonZeroI16` but with `i16::MAX` as the invalid value.
//! Only the [`i16`] variant is implemented, as this is the only one needed for the engine.

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct NonMaxI16(std::num::NonZeroI16);

impl NonMaxI16 {
    pub const fn new(n: i16) -> Option<Self> {
        let inner = std::num::NonZeroI16::new(n ^ i16::MAX);
        if let Some(inner) = inner {
            Some(Self(inner))
        } else {
            None
        }
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
        n.0.get() ^ i16::MAX
    }
}

impl From<u8> for NonMaxI16 {
    fn from(n: u8) -> Self {
        // SAFETY: `n` cannot be `i16::MAX`.
        unsafe { Self::new_unchecked(n as i16) }
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

// The following impls are dodgey in general, as we just ignore the check for `i16::MAX`.
// For the purposes of this engine, release builds will just assume everything to be fine.
// We'll make sure this is the case logically, but we won't check it all the time for
// performance. Debug asserts should catch any unlikely surprises.

impl std::ops::Add for NonMaxI16 {
    type Output = Self;

    fn add(self, rhs: Self) -> Self {
        let sum = i16::from(self) + i16::from(rhs);
        debug_assert_ne!(sum, i16::MAX);
        unsafe { Self::new_unchecked(sum) }
    }
}

impl std::ops::Sub for NonMaxI16 {
    type Output = Self;

    fn sub(self, rhs: Self) -> Self {
        let diff = i16::from(self) - i16::from(rhs);
        debug_assert_ne!(diff, i16::MAX);
        unsafe { Self::new_unchecked(diff) }
    }
}

impl std::ops::Neg for NonMaxI16 {
    type Output = Self;

    fn neg(self) -> Self {
        let neg = -i16::from(self);
        debug_assert_ne!(neg, i16::MAX);
        unsafe { Self::new_unchecked(neg) }
    }
}

impl std::ops::AddAssign for NonMaxI16 {
    fn add_assign(&mut self, rhs: Self) {
        *self = *self + rhs;
    }
}

impl std::ops::SubAssign for NonMaxI16 {
    fn sub_assign(&mut self, rhs: Self) {
        *self = *self - rhs;
    }
}

impl std::str::FromStr for NonMaxI16 {
    type Err = anyhow::Error;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        let n = s.parse()?;
        NonMaxI16::new(n).ok_or_else(|| anyhow::Error::msg("value is i16::MAX, which is invalid"))
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

    #[test]
    fn test_nonmaxi16_add() {
        assert_eq!(nu(1) + nu(2), nu(3));
        assert_eq!(nu(i16::MAX - 2) + nu(1), nu(i16::MAX - 1));
    }

    #[test]
    fn test_nonmaxi16_sub() {
        assert_eq!(nu(3) - nu(2), nu(1));
        assert_eq!(nu(i16::MAX - 1) - nu(2), nu(i16::MAX - 3));
    }

    #[test]
    fn test_nonmaxi16_neg() {
        for case in [1, 0, i16::MAX - 1, i16::MIN + 2].iter().copied() {
            assert_eq!(-nu(case), nu(-case));
        }
    }

    #[test]
    fn test_nonmaxi16_add_assign() {
        let mut n = nu(1);
        n += nu(2);
        assert_eq!(n, nu(3));
    }

    #[test]
    fn test_nonmaxi16_sub_assign() {
        let mut n = nu(3);
        n -= nu(2);
        assert_eq!(n, nu(1));
    }

    #[test]
    fn test_nonmaxi16_from_str() {
        assert_eq!("1".parse::<NonMaxI16>().unwrap(), nu(1));
        assert_eq!("0".parse::<NonMaxI16>().unwrap(), nu(0));
        assert_eq!("-1".parse::<NonMaxI16>().unwrap(), nu(-1));
        assert_eq!(
            format!("{}", i16::MAX - 1).parse::<NonMaxI16>().unwrap(),
            nu(i16::MAX - 1)
        );
        assert!(format!("{}", i16::MAX).parse::<NonMaxI16>().is_err());
    }
}
