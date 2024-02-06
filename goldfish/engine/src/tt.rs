use chess::ChessMove;

use crate::value::{self, Depth, Value};

/// A bound on the value of a position.
///
/// When searching a position using alpha-beta pruning, the evaluations of a position may
/// not be exact. For the purposes of a transposition table, we need to keep track of
/// whether the value is an upper bound, a lower bound, or an exact value, so that we
/// can take the correct action when we encounter the position again.
#[derive(Debug, Clone, Copy)]
#[repr(u8)]
pub enum Bound {
    Lower = 1,
    Upper = 2,
    Exact = Self::Lower as u8 | Self::Upper as u8,
}

impl std::ops::BitAnd for Bound {
    type Output = bool;

    fn bitand(self, rhs: Self) -> Self::Output {
        let self_bits = self as u8;
        let rhs_bits = rhs as u8;
        self_bits & rhs_bits != 0
    }
}

/// A [`Value`] with special handling for mate scores.
///
/// In short, mates are scored as the distance to the mate from the root position, but that
/// distance might be different between two nodes that are transpositions of each other.
/// Therefore, we store instead the distance to the mate from the current position, which
/// would be the same for both nodes. More detailed explenation below.
/// [`https://github.com/maksimKorzh/chess_programming/blob/master/src/bbc/tt_search_mating_scores/TT_mate_scoring.txt`]
#[derive(Debug, Clone, Copy)]
struct TtValue(Value);

/// A transposition table entry.
///
/// Each entry stores the following information:
/// - Checkbits: The 32 most significant bits of the Zobrist key, used to detect collisions.
/// - move: The best move found for the position, if any.
/// - Bound: The type of the value stored in the entry.
/// - Depth: The depth at which the value was computed.
#[derive(Debug, Clone)]
struct Entry {
    checkbits: u32,
    pub mv: Option<ChessMove>,
    pub bound: Bound,
    pub value: TtValue,
    depth: Depth,
}

/// A transposition table.
///
/// The table is a fixed-size array of entries. The size is specified at creation time.
/// The entries in the table are indexed by the Zobrist key of the position they represent.
///
/// See [`Entry`] for more information on the contents of each entry.
#[derive(Debug)]
pub struct TranspositionTable {
    entries: Vec<Option<Entry>>,
}

impl Default for TranspositionTable {
    fn default() -> Self {
        Self::new(crate::DEFAULT_HASH_SIZE_MB * crate::MB)
    }
}

impl TranspositionTable {
    /// Creates a new transposition table with the given size in bytes.
    pub fn new(size: usize) -> Self {
        let count = size / std::mem::size_of::<Option<Entry>>();
        Self {
            entries: vec![None; count],
        }
    }

    /// Resizes the table to the given size in bytes.
    ///
    /// This clears the table, and all entries are lost.
    /// This is required, because the indices are only consistent for a given size.
    pub fn resize(&mut self, size: usize) {
        let count = size / std::mem::size_of::<Option<Entry>>();
        self.entries.clear();
        self.entries.resize(count, None);
    }

    /// Returns an index into the table for the given (Zobrist) key.
    ///
    /// # Safety
    /// It is always safe to use the returned index to access the table.
    fn index(&self, key: u64) -> usize {
        #[allow(clippy::cast_possible_truncation)]
        let key = key as usize;
        key % self.entries.len()
    }

    /// Returns the checkbits for the given key.
    fn checkbits(key: u64) -> u32 {
        (key >> 32) as u32
    }

    /// Returns an entry for the given key if it exists and is valid.
    ///
    /// Two conditions must be met to get a [`Some`] value:
    ///
    /// 1. The entry must exist, i.e. the position must have been stored in the table.
    /// 2. The entry must have a depth greater than or equal to the requested depth.
    pub fn get(
        &self,
        key: u64,
        depth: Depth,
        ply: Depth,
    ) -> Option<(Option<ChessMove>, Bound, Value)> {
        // Safety: `index` is guaranteed to return a valid index.
        let entry = unsafe { self.entries.get_unchecked(self.index(key)).as_ref() };

        entry
            .filter(|entry| Self::checkbits(key) == entry.checkbits && depth <= entry.depth)
            .map(|entry| (entry.mv, entry.bound, entry.value.into(ply)))
    }

    /// Stores an entry in the table if it is better than the current entry.
    ///
    /// The entry is stored if:
    ///
    /// 1. There is no entry at the index to which the key maps, or
    /// 2. The existing entry has a depth less than or equal to the new entry.
    ///
    /// In case of a collision, the entry with the greater depth is stored, and ties broken in
    /// favor of the new entry.
    pub fn store(
        &mut self,
        key: u64,
        mv: Option<ChessMove>,
        bound: Bound,
        value: Value,
        depth: Depth,
        ply: Depth,
    ) {
        let index = self.index(key);
        // Safety: `index` is guaranteed to return a valid index.
        let entry = unsafe { self.entries.get_unchecked_mut(index) };

        if entry.as_mut().map_or(true, |e| depth >= e.depth) {
            *entry = Some(Entry {
                checkbits: (key >> 32) as u32,
                mv,
                bound,
                value: TtValue::from(value, ply),
                depth,
            });
        }
    }
}

impl TtValue {
    fn from(value: Value, ply: Depth) -> Self {
        if value >= value::CHECKMATE_THRESHOLD {
            Self(value + Value::from(ply))
        } else if value <= -value::CHECKMATE_THRESHOLD {
            Self(value - Value::from(ply))
        } else {
            Self(value)
        }
    }

    fn into(self, ply: Depth) -> Value {
        if self.0 >= value::CHECKMATE_THRESHOLD {
            self.0 - Value::from(ply)
        } else if self.0 <= -value::CHECKMATE_THRESHOLD {
            self.0 + Value::from(ply)
        } else {
            self.0
        }
    }
}
