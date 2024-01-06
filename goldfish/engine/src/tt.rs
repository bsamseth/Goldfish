use chess::ChessMove;

use crate::value::{Depth, Value};

/// A bound on the value of a position.
///
/// When searching a position using alpha-beta pruning, the evaluations of a position may
/// not be exact. For the purposes of a transposition table, we need to keep track of
/// whether the value is an upper bound, a lower bound, or an exact value, so that we
/// can take the correct action when we encounter the position again.
#[derive(Debug, Clone, Copy)]
pub enum Bound {
    Lower(Value),
    Upper(Value),
    Exact(Value),
}

/// A transposition table entry.
///
/// Each entry stores the following information:
/// - Checkbits: The 32 most significant bits of the Zobrist key, used to detect collisions.
/// - move: The best move found for the position.
/// - Bound: The type of the value stored in the entry.
/// - Depth: The depth at which the value was computed.
#[derive(Debug, Clone)]
pub struct Entry {
    checkbits: u32,
    pub mv: ChessMove,
    pub bound: Bound,
    depth: Depth,
}

/// A transposition table.
///
/// The table is a fixed-size array of entries. The size is specified at creation time.
/// The entries in the table are indexed by the Zobrist key of the position they represent.
///
/// See `Entry` for more information on the contents of each entry.
#[derive(Debug)]
pub struct TranspositionTable {
    entries: Vec<Option<Entry>>,
}

impl TranspositionTable {
    /// Creates a new transposition table with the given size in bytes.
    pub fn new(size: usize) -> Self {
        Self {
            entries: vec![None; size],
        }
    }

    /// Returns an index into the table for the given (Zobrist) key.
    ///
    /// # Safety
    /// It is always safe to use the returned index to access the table.
    fn index(&self, key: u64) -> usize {
        key as usize % self.entries.len()
    }

    /// Returns the checkbits for the given key.
    fn checkbits(key: u64) -> u32 {
        (key >> 32) as u32
    }

    /// Returns an entry for the given key if it exists and is valid.
    ///
    /// Two conditions must be met to get a `Some` value:
    ///
    /// 1. The entry must exist, i.e. the position must have been stored in the table.
    /// 2. The entry must have a depth greater than or equal to the requested depth.
    pub fn get(&self, key: u64, depth: Depth) -> Option<&Entry> {
        // Safety: `index` is guaranteed to return a valid index.
        let entry = unsafe { self.entries.get_unchecked(self.index(key)).as_ref() };

        entry.filter(|entry| Self::checkbits(key) == entry.checkbits && depth <= entry.depth)
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
    pub fn store(&mut self, key: u64, mv: ChessMove, bound: Bound, depth: Depth) {
        let index = self.index(key);
        // Safety: `index` is guaranteed to return a valid index.
        let entry = unsafe { self.entries.get_unchecked_mut(index) };

        if entry.as_mut().map_or(true, |e| depth >= e.depth) {
            *entry = Some(Entry {
                checkbits: (key >> 32) as u32,
                mv,
                bound,
                depth,
            });
        }
    }
}
