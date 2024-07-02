//! Transposition table.
//!
//! This is a port of Stockfish's transposition table implementation, with modifications where
//! necessary and/or possible.
#![allow(clippy::cast_lossless, dead_code)]

use chess::{ChessMove, Piece, Square};

use crate::newtypes::{Depth, Ply, Value};

const DEPTH_ENTRY_OFFSET: i16 = -3;

pub type Key = u64;
pub type Found = bool;

/// An entry in the transposition table.
///
/// An [`Entry`] struct is a 10 bytes transposition table entry, defined as below:
///
/// key        16 bit
/// depth       8 bit
/// generation  5 bit
/// pv node     1 bit
/// bound type  2 bit
/// move       16 bit
/// value      16 bit
/// evaluation 16 bit
///
/// These fields are in the same order as accessed by `probe()`, since memory is fastest sequentially.
/// Equally, the store order in `save()` matches this order.
#[derive(Debug, Clone, Copy, Default)]
#[repr(C)]
struct Entry {
    key16: u16,
    depth8: u8,
    gen_and_bound8: u8,
    move16: Option<Move16>,
    value: TtValue,
    evaluation: TtValue,
}
static_assertions::assert_eq_size!(Entry, [u8; 10]);

/// A writer for an entry in the transposition table.
///
/// This effectively just a mutable pointer to the [`Entry`], but restricted to only allow
/// calling the [`Entry::save`] method.
#[derive(Debug)]
pub struct EntryWriter(*mut Entry);

/// Data from the transposition table, absent bookkeeping information.
///
/// This is the data type that is read from during search.
#[derive(Debug)]
pub struct Data {
    pub mv: Option<ChessMove>,
    pub value: TtValue,
    pub eval: TtValue,
    pub depth: Depth,
    pub bound: Bound,
    pub is_pv: bool,
}

/// A condensed representation of a move, used only inside the transposition table.
///
/// A move needs 16 bits to be stored
///
/// bit  0- 5: destination square (from 0 to 63)
/// bit  6-11: origin square (from 0 to 63)
/// bit 12-14: promotion piece type (none = 000, queen = 001, rook = 010, bishop = 011, knight = 100)
/// bit 15-16: spare bits (because [`chess::ChessMove`] is context dependent and doesn't itself store
///            special move types
///
/// This can be stored in an [`Option`] without additional size overhead.
#[derive(Debug, Clone, Copy)]
struct Move16(std::num::NonZeroU16);

/// A bound on the value of a position.
///
/// When searching a position using alpha-beta pruning, the evaluations of a position may
/// not be exact. For the purposes of a transposition table, we need to keep track of
/// whether the value is an upper bound, a lower bound, or an exact value, so that we
/// can take the correct action when we encounter the position again.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum Bound {
    Lower = 1,
    Upper = 2,
    Exact = Self::Lower as u8 | Self::Upper as u8,
}

/// A [`Value`] with special handling for mate scores.
///
/// In short, mates are scored as the distance to the mate from the root position, but that
/// distance might be different between two nodes that are transpositions of each other.
/// Therefore, we store instead the distance to the mate from the current position, which
/// would be the same for both nodes. More detailed explenation below.
/// [`https://github.com/maksimKorzh/chess_programming/blob/master/src/bbc/tt_search_mating_scores/TT_mate_scoring.txt`]
#[derive(Debug, Clone, Copy, Default)]
struct TtValue(Value);

/// A cluster of entries in the transposition table.
///
/// This is a performance optimization aimed at improving CPU cache usage.
/// It is aligned to 32 bytes for this reason.
#[derive(Debug, Clone, Default)]
struct Cluster {
    entries: [Entry; 3],
    _padding: [u8; 2],
}

static_assertions::assert_eq_size!(Cluster, [u8; 32]);

/// A transposition table.
///
/// The table consists of [`Entry`], grouped into [`Cluster`]s, indexed by a Zobrist key.
/// See [`Entry`] for more information on the contents of each entry.
#[derive(Debug, Default)]
pub struct TranspositionTable {
    clusters: Vec<Cluster>,
    generation: u8,
}

impl TranspositionTable {
    /// Resizes the table to the given size in bytes.
    ///
    /// This clears the table, and all entries are lost.
    /// This is required, because the indices are only consistent for a given size.
    pub fn resize(&mut self, size: usize) {
        let cluster_count = size / std::mem::size_of::<Cluster>();

        self.clusters.clear();
        self.clusters.resize(cluster_count, Cluster::default());
    }

    /// Return an approximation of the number of used entries in the table, as a permill of the total size.
    ///
    /// This only counts entries in the current generation.
    pub fn hashfull(&self) -> usize {
        self.clusters
            .iter()
            .take(1000)
            .map(|c| {
                c.entries
                    .iter()
                    .filter(|e| e.is_occupied() && e.relative_age(self.generation) == 0)
                    .count()
            })
            .sum()
    }

    pub fn new_search(&mut self) {
        // Increment the generation, keeping the lower bits unchanged.
        self.generation += Entry::GENERATION_DELTA;
    }

    pub fn probe(&mut self, key: Key) -> (Found, Data, EntryWriter) {
        #[allow(clippy::cast_possible_truncation)]
        let key16 = key as u16; // Use the low 16 bits as key within the cluster.
        let generation = self.generation;
        let cluster = self.cluster_for(key);

        if let Some(entry) = cluster.entries.iter_mut().find(|e| e.key16 == key16) {
            let data = entry.read();
            let writer = EntryWriter(entry);
            return (true, data, writer);
        }

        // Find an entry to replace according to the replacement strategy.
        let entry = cluster
            .entries
            .iter_mut()
            .min_by_key(|e| e.depth8 + e.relative_age(generation));
        // Safety: Each cluster has a non-zero number of entries, so the minimum is always present.
        let entry = unsafe { entry.unwrap_unchecked() };

        (false, entry.read(), EntryWriter(entry))
    }

    fn cluster_for<'a, 'b: 'a>(&'b mut self, key: Key) -> &'a mut Cluster {
        let len = self.clusters.len() as u64;
        let key = mul_hi64(key, len);
        static_assertions::assert_eq_size!(usize, u64);
        #[allow(clippy::cast_possible_truncation)]
        &mut self.clusters[key as usize]
    }
}

/// Return the higher 64 bits of the product of two 64-bit integers.
fn mul_hi64(a: u64, b: u64) -> u64 {
    let a = a as u128;
    let b = b as u128;
    let r = a * b;
    (r >> 64) as u64
}

impl From<&Entry> for Data {
    fn from(entry: &Entry) -> Self {
        Self {
            mv: entry.move16.map(From::from),
            value: entry.value,
            eval: entry.evaluation,
            // Safety: The depth is guaranteed to be in bounds.
            #[allow(clippy::cast_sign_loss, clippy::cast_possible_truncation)]
            depth: Depth::new((i16::from(entry.depth8) + DEPTH_ENTRY_OFFSET) as u8),
            bound: Bound::from(entry.gen_and_bound8 & 0b11),
            is_pv: entry.gen_and_bound8 & 0b100 != 0,
        }
    }
}

impl Entry {
    fn save<const PV: crate::search::PvNode>(
        &mut self,
        key: Key,
        value: Value,
        bound: Bound,
        depth: Depth,
        mv: Option<ChessMove>,
        eval: Value,
        generation: u8,
        ply: Ply,
    ) {
        #[allow(clippy::cast_possible_truncation)]
        let key16 = key as u16;

        // Preserve existing move if no move is provided.
        if mv.is_some() || key16 != self.key16 {
            self.move16 = mv.map(Into::into);
        }

        let pv_bonus: i16 = if PV { 2 } else { 0 };
        let pv_bit: u8 = if PV { 0b100 } else { 0 };

        // Overwrite less valuable entries.
        if bound == Bound::Exact
            || key16 != self.key16
            || (depth.as_inner() as i16) - DEPTH_ENTRY_OFFSET + pv_bonus > (self.depth8 as i16) - 4
            || self.relative_age(generation) > 0
        {
            let d = depth.as_inner() as i16;
            debug_assert!(d > DEPTH_ENTRY_OFFSET);
            debug_assert!(d < 256 + DEPTH_ENTRY_OFFSET);
            #[allow(clippy::cast_sign_loss, clippy::cast_possible_truncation)]
            let d = (d - DEPTH_ENTRY_OFFSET) as u8;

            self.key16 = key16;
            self.depth8 = d;
            self.gen_and_bound8 = generation | pv_bit | bound as u8;
            self.value = TtValue::from(value, ply);
            self.evaluation = TtValue::from(eval, ply);
        }
    }

    fn is_occupied(&self) -> bool {
        self.depth8 != 0
    }

    const GENERATION_DELTA: u8 = 0b1000;
    const GENERATION_CYCLE: u16 = 0xff + (Entry::GENERATION_DELTA as u16);
    const GENERATION_MASK: u16 = 0b1111_1000;

    fn relative_age(&self, generation: u8) -> u8 {
        static_assertions::const_assert_eq!(Entry::GENERATION_CYCLE & Entry::GENERATION_MASK, 0);

        let age = (Entry::GENERATION_CYCLE + (generation as u16) - (self.gen_and_bound8 as u16))
            & Entry::GENERATION_MASK;

        // Safety: `age` is guaranteed to be in bounds due to the bitmask.
        age as u8
    }

    fn read(&self) -> Data {
        self.into()
    }
}

impl EntryWriter {
    /// Save an entry to the transposition table.
    ///
    /// # Safety
    /// Calling this function is safe if the [`EntryWriter`] is only used within the same search,
    /// that it was created in.
    unsafe fn save<const PV: crate::search::PvNode>(
        &mut self,
        key: Key,
        value: Value,
        bound: Bound,
        depth: Depth,
        mv: Option<ChessMove>,
        eval: Value,
        generation: u8,
        ply: Ply,
    ) {
        // Safety: The contained pointer will be valid as the table is only invalidated between
        // searches. Within a search, the pointer is valid, and [`EntryWriter`]s are only created
        // by probing, which is done during search.
        unsafe { *self.0 }.save::<PV>(key, value, bound, depth, mv, eval, generation, ply);
    }
}

impl From<Move16> for ChessMove {
    fn from(mv: Move16) -> Self {
        let mv = mv.0.get();
        // Safety: Due to the bitmask, the values are guaranteed to be in bounds.
        let to = unsafe { Square::new((mv & 0b11_1111) as u8) };
        let from = unsafe { Square::new(((mv >> 6) & 0b11_1111) as u8) };
        let promotion = match (mv >> 12) & 0b111 {
            1 => Some(Piece::Queen),
            3 => Some(Piece::Rook),
            2 => Some(Piece::Bishop),
            4 => Some(Piece::Knight),
            _ => None,
        };
        ChessMove::new(from, to, promotion)
    }
}

impl From<ChessMove> for Move16 {
    fn from(mv: ChessMove) -> Self {
        let from = mv.get_source().to_int() as u16;
        let to = mv.get_dest().to_int() as u16;
        let promotion = mv.get_promotion().map_or(0u16, |p| match p {
            Piece::Queen => 1,
            Piece::Rook => 3,
            Piece::Bishop => 2,
            Piece::Knight => 4,
            _ => unreachable!(),
        });
        let mv = from | (to << 6) | (promotion << 12);
        // Safety: All moves are to and from distinct squares, so `mv` cannot be zero.
        debug_assert_ne!(mv, 0);
        Self(unsafe { std::num::NonZeroU16::new_unchecked(mv) })
    }
}

impl From<u8> for Bound {
    fn from(bits: u8) -> Self {
        match bits {
            1 => Self::Lower,
            2 => Self::Upper,
            3 => Self::Exact,
            _ => unreachable!(),
        }
    }
}

impl std::ops::BitAnd for Bound {
    type Output = bool;

    fn bitand(self, rhs: Self) -> Self::Output {
        let self_bits = self as u8;
        let rhs_bits = rhs as u8;
        self_bits & rhs_bits != 0
    }
}

impl TtValue {
    fn from(value: Value, ply: Ply) -> Self {
        if value.is_winning_checkmate() {
            Self(value + Value::new(i16::from(ply.as_inner())))
        } else if value.is_losing_checkmate() {
            Self(value - Value::new(i16::from(ply.as_inner())))
        } else {
            Self(value)
        }
    }

    fn into(self, ply: Ply) -> Value {
        if self.0.is_winning_checkmate() {
            self.0 - Value::new(i16::from(ply.as_inner()))
        } else if self.0.is_losing_checkmate() {
            self.0 + Value::new(i16::from(ply.as_inner()))
        } else {
            self.0
        }
    }
}
