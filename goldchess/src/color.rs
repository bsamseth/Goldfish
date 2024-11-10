/// Represents the color of a piece, or the side to move.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Color {
    White,
    Black,
}

impl Color {
    pub const ALL: [Color; 2] = [Color::White, Color::Black];
    pub const NUM_COLORS: usize = Color::ALL.len();

    /// Get the index of the color.
    #[must_use]
    pub const fn as_index(self) -> usize {
        match self {
            Color::White => 0,
            Color::Black => 1,
        }
    }
}

impl std::ops::Not for Color {
    type Output = Self;

    fn not(self) -> Self {
        match self {
            Color::White => Color::Black,
            Color::Black => Color::White,
        }
    }
}

impl std::fmt::Display for Color {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Color::White => write!(f, "w"),
            Color::Black => write!(f, "b"),
        }
    }
}
