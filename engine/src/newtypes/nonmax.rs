//! [`NonMaxI16`] is a newtype that represents a signed 16-bit integer that is not equal to `i16::MAX`.
//!
//! This is the equivalent of `NonZeroI16` but with `i16::MAX` as the invalid value.
//! Only the [`i16`] variant is implemented, as this is the only one needed for the engine.

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
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

    pub const fn new_unchecked(n: i16) -> Self {
        // SAFETY: Caller guarantees that `n` is not `i16::MAX`.
        Self(unsafe { std::num::NonZeroI16::new_unchecked(n ^ i16::MAX) })
    }
}

impl From<NonMaxI16> for i16 {
    fn from(n: NonMaxI16) -> i16 {
        n.0.get() ^ i16::MAX
    }
}

impl std::fmt::Display for NonMaxI16 {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        i16::from(*self).fmt(f)
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
        Self::new_unchecked(sum)
    }
}

impl std::ops::Sub for NonMaxI16 {
    type Output = Self;

    fn sub(self, rhs: Self) -> Self {
        let diff = i16::from(self) - i16::from(rhs);
        debug_assert_ne!(diff, i16::MAX);
        Self::new_unchecked(diff)
    }
}

impl std::ops::Neg for NonMaxI16 {
    type Output = Self;

    fn neg(self) -> Self {
        let neg = -i16::from(self);
        debug_assert_ne!(neg, i16::MAX);
        Self::new_unchecked(neg)
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

    #[test]
    fn test_nonmaxi16_new() {
        assert_eq!(NonMaxI16::new(i16::MAX), None);
        assert_eq!(
            NonMaxI16::new(i16::MIN),
            Some(NonMaxI16::new_unchecked(i16::MIN))
        );
        assert_eq!(NonMaxI16::new(0), Some(NonMaxI16::new_unchecked(0)));
        assert_eq!(NonMaxI16::new(1), Some(NonMaxI16::new_unchecked(1)));
        assert_eq!(
            NonMaxI16::new(i16::MAX - 1),
            Some(NonMaxI16::new_unchecked(i16::MAX - 1))
        );
    }

    #[test]
    fn test_nonmaxi16_from() {
        assert_eq!(i16::from(NonMaxI16::new_unchecked(i16::MIN)), i16::MIN);
        assert_eq!(i16::from(NonMaxI16::new_unchecked(0)), 0);
        assert_eq!(i16::from(NonMaxI16::new_unchecked(1)), 1);
        assert_eq!(
            i16::from(NonMaxI16::new_unchecked(i16::MAX - 1)),
            i16::MAX - 1
        );
    }

    #[test]
    fn test_nonmaxi16_add() {
        assert_eq!(
            NonMaxI16::new_unchecked(1) + NonMaxI16::new_unchecked(2),
            NonMaxI16::new_unchecked(3)
        );
        assert_eq!(
            NonMaxI16::new_unchecked(i16::MAX - 2) + NonMaxI16::new_unchecked(1),
            NonMaxI16::new_unchecked(i16::MAX - 1)
        );
    }

    #[test]
    fn test_nonmaxi16_sub() {
        assert_eq!(
            NonMaxI16::new_unchecked(3) - NonMaxI16::new_unchecked(2),
            NonMaxI16::new_unchecked(1)
        );
        assert_eq!(
            NonMaxI16::new_unchecked(i16::MAX - 1) - NonMaxI16::new_unchecked(2),
            NonMaxI16::new_unchecked(i16::MAX - 3)
        );
    }

    #[test]
    fn test_nonmaxi16_neg() {
        for case in [1, 0, i16::MAX - 1, i16::MIN + 2].iter().copied() {
            assert_eq!(
                -NonMaxI16::new_unchecked(case),
                NonMaxI16::new_unchecked(-case)
            );
        }
    }

    #[test]
    fn test_nonmaxi16_add_assign() {
        let mut n = NonMaxI16::new_unchecked(1);
        n += NonMaxI16::new_unchecked(2);
        assert_eq!(n, NonMaxI16::new_unchecked(3));
    }

    #[test]
    fn test_nonmaxi16_sub_assign() {
        let mut n = NonMaxI16::new_unchecked(3);
        n -= NonMaxI16::new_unchecked(2);
        assert_eq!(n, NonMaxI16::new_unchecked(1));
    }

    #[test]
    fn test_nonmaxi16_from_str() {
        assert_eq!(
            "1".parse::<NonMaxI16>().unwrap(),
            NonMaxI16::new_unchecked(1)
        );
        assert_eq!(
            "0".parse::<NonMaxI16>().unwrap(),
            NonMaxI16::new_unchecked(0)
        );
        assert_eq!(
            "-1".parse::<NonMaxI16>().unwrap(),
            NonMaxI16::new_unchecked(-1)
        );
        assert_eq!(
            format!("{}", i16::MAX - 1).parse::<NonMaxI16>().unwrap(),
            NonMaxI16::new_unchecked(i16::MAX - 1)
        );
        assert!(format!("{}", i16::MAX).parse::<NonMaxI16>().is_err());
    }
}
