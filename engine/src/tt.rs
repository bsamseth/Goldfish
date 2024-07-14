//! Transposition table.
//!
//! This is a port of Stockfish's transposition table implementation, with modifications where
//! necessary and/or possible.
#![allow(clippy::cast_lossless, dead_code)]

use chess::{ChessMove, Piece, Square};

use crate::newtypes::{Depth, NonMaxI16, Ply, Value};

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
///
/// Note: This struct is trivially memcpy-able, but is _not_ marked as [`Copy`]. This is because
/// we want very deliberate move semantics and avoid accidentally writing to copies.
#[derive(Debug, Clone, Default)]
#[repr(C)]
struct Entry {
    key16: u16,
    depth8: u8,
    gen_and_bound8: u8,
    move16: Option<Move16>,
    value: Option<TtValue>,
    evaluation: Option<TtValue>,
}
static_assertions::assert_eq_size!(Entry, [u8; 10]);

/// A writer for an entry in the transposition table.
///
/// This effectively just a mutable pointer to the [`Entry`], but restricted to only allow
/// calling a well defined `save` method.
#[derive(Debug)]
pub struct EntryWriter {
    entry: *mut Entry,
    key: Key,
    generation: u8,
}

#[derive(Debug, Default)]
pub struct EntryWriterOpts {
    pub bound: Bound,
    pub depth: Depth,
    pub ply: Ply,
    pub mv: Option<ChessMove>,
    pub value: Option<Value>,
    pub eval: Option<Value>,
}

/// Data from the transposition table, absent bookkeeping information.
///
/// This is the data type that is read from during search.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Data {
    pub mv: Option<ChessMove>,
    pub value: Option<Value>,
    pub eval: Option<Value>,
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
#[derive(Debug, Clone, Copy, Default, PartialEq, Eq)]
#[repr(u8)]
pub enum Bound {
    #[default]
    Lower = 1,
    Upper = 2,
    Exact = Self::Lower as u8 | Self::Upper as u8,
}

/// A [`Value`] with special handling for mate scores and allowing efficient [`Option`] representation.
///
/// Mates are scored as the distance to the mate from the root position, but that
/// distance might be different between two nodes that are transpositions of each other.
/// Therefore, we store instead the distance to the mate from the current position, which
/// would be the same for both nodes. More detailed explenation below.
/// [`https://github.com/maksimKorzh/chess_programming/blob/master/src/bbc/tt_search_mating_scores/TT_mate_scoring.txt`]
///
/// Further, this uses [`NonMaxI16`] to allow for efficient [`Option`] representation.
#[derive(Debug, Clone, Copy, Default)]
struct TtValue(NonMaxI16);

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
    /// Create a new transposition table with the given size in bytes.
    pub fn new(size: usize) -> Self {
        let mut tt = Self::default();
        tt.resize(size);
        tt
    }

    /// Resizes the table to the given size in bytes.
    ///
    /// This clears the table, and all entries are lost.
    /// This is required, because the indices are only consistent for a given size.
    pub fn resize(&mut self, size: usize) -> &mut Self {
        let cluster_count = size / std::mem::size_of::<Cluster>();

        self.generation = 0;
        self.clusters.clear();
        self.clusters.resize(cluster_count, Cluster::default());
        self
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

    /// Prepare the table for a new search by incrementing the generation.
    ///
    /// This does nothing to the existing entries, but ensures new entries can take precedence
    /// over old ones during hash collisions.
    pub fn new_search(&mut self) {
        // Increment the generation, keeping the lower bits unchanged, with intentional wrapping.
        self.generation = self.generation.wrapping_add(Entry::GENERATION_DELTA);
    }

    /// Return the current generation of the table.
    pub fn generation(&self) -> u8 {
        self.generation
    }

    /// Retrieve data for the given [`Key`], or [`None`] if the key is not found.
    pub fn probe(&self, key: Key, ply: Ply, halfmove_count: usize) -> Option<Data> {
        #[allow(clippy::cast_possible_truncation)]
        let key16 = key as u16; // Use the low 16 bits as key within the cluster.
        let cluster = self.cluster_for(key);

        cluster
            .entries
            .iter()
            .find(|e| e.is_occupied() && e.key16 == key16)
            .map(|entry| Data::from_entry(entry, ply, halfmove_count))
    }

    /// Retrieve data for the given [`Key`], and an [`EntryWriter`] where the next entry may be written.
    ///
    /// Either the data is [`Some`] and the [`EntryWriter`] points to the returned entry, or the
    /// data is [`None`] and the [`EntryWriter`] points to the least valuable entry in the cluster.
    /// In either case, the user should use the writer to save updates/modifications to the
    /// table.
    ///
    /// This provides an efficient query-modify pattern against the table, as that all usage of the
    /// table will first probe for an entry, and then potentially modify it.
    pub fn probe_mut(
        &mut self,
        key: Key,
        ply: Ply,
        halfmove_count: usize,
    ) -> (Option<Data>, EntryWriter) {
        #[allow(clippy::cast_possible_truncation)]
        let key16 = key as u16; // Use the low 16 bits as key within the cluster.
        let generation = self.generation;
        let cluster = self.cluster_for_mut(key);

        if let Some(entry) = cluster
            .entries
            .iter_mut()
            .find(|e| e.is_occupied() && e.key16 == key16)
        {
            let data = Data::from_entry(entry, ply, halfmove_count);
            let writer = EntryWriter {
                entry: std::ptr::from_mut(entry),
                key,
                generation,
            };
            return (Some(data), writer);
        }

        // Find an entry to replace according to the replacement strategy.
        let entry = cluster
            .entries
            .iter_mut()
            .min_by_key(|e| e.depth8.saturating_add(e.relative_age(generation)));
        // Safety: Each cluster has a non-zero number of entries, so the minimum is always present.
        let entry = unsafe { entry.unwrap_unchecked() };

        (
            None,
            EntryWriter {
                entry: std::ptr::from_mut(entry),
                key,
                generation,
            },
        )
    }

    fn cluster_for<'a, 'b: 'a>(&'b self, key: Key) -> &'a Cluster {
        let len = self.clusters.len() as u64;
        let key = mul_hi64(key, len);
        static_assertions::assert_eq_size!(usize, u64);
        #[allow(clippy::cast_possible_truncation)]
        &self.clusters[key as usize]
    }

    fn cluster_for_mut<'a, 'b: 'a>(&'b mut self, key: Key) -> &'a mut Cluster {
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

impl Data {
    /// Convert an [`Entry`] to a [`Data`].
    ///
    /// The [`Entry`] is a compacted form of the data used internally by the table,
    /// while the [`Data`] is directly useable in search. It also serves as a separation between
    /// the inner implementation of the table and the "public" interface.
    fn from_entry(entry: &Entry, ply: Ply, halfmove_count: usize) -> Self {
        Self {
            mv: entry.move16.map(From::from),
            value: entry.value.map(|v| v.into(ply, halfmove_count)),
            eval: entry.evaluation.map(|v| v.into(ply, halfmove_count)),
            // Safety: The depth is guaranteed to be in bounds.
            #[allow(clippy::cast_sign_loss, clippy::cast_possible_truncation)]
            depth: Depth::new(i16::from(entry.depth8) + DEPTH_ENTRY_OFFSET),
            bound: Bound::from(entry.gen_and_bound8 & 0b11),
            is_pv: entry.gen_and_bound8 & 0b100 != 0,
        }
    }
}

impl Entry {
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
}

impl EntryWriter {
    /// Save an entry to the transposition table.
    ///
    /// # Safety
    /// Calling this function is safe if it is only used within the same search that the entry it
    /// refers to was created in.
    pub unsafe fn save<const PV: crate::search::PvNode>(&self, opts: &EntryWriterOpts) {
        // Safety: The contained pointer will be valid as the table is only invalidated between
        // searches. Within a search, the pointer is valid, and [`EntryWriter`]s are only created
        // by probing, which is done during search.
        let entry = unsafe { self.entry.as_mut().unwrap_unchecked() };

        #[allow(clippy::cast_possible_truncation)]
        let key16 = self.key as u16;

        // Preserve existing move if no move is provided.
        if opts.mv.is_some() || key16 != entry.key16 {
            entry.move16 = opts.mv.map(Into::into);
        }

        let pv_bonus: i16 = if PV { 2 } else { 0 };
        let pv_bit: u8 = if PV { 0b100 } else { 0 };

        // Overwrite less valuable entries.
        if opts.bound == Bound::Exact
            || key16 != entry.key16
            || opts.depth.as_inner() - DEPTH_ENTRY_OFFSET + pv_bonus > (entry.depth8 as i16) - 4
            || entry.relative_age(self.generation) > 0
        {
            let d = opts.depth.as_inner();
            debug_assert!(d > DEPTH_ENTRY_OFFSET);
            debug_assert!(d < 256 + DEPTH_ENTRY_OFFSET);
            #[allow(clippy::cast_sign_loss, clippy::cast_possible_truncation)]
            let d = (d - DEPTH_ENTRY_OFFSET) as u8;

            entry.key16 = key16;
            entry.depth8 = d;
            entry.gen_and_bound8 = self.generation | pv_bit | opts.bound as u8;
            entry.value = opts.value.map(|v| TtValue::from(v, opts.ply));
            entry.evaluation = opts.eval.map(|v| TtValue::from(v, opts.ply));
        }
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
        let mv = to | (from << 6) | (promotion << 12);
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
    /// Convert from a [`Value`] to a [`TtValue`].
    ///
    /// Adjusts a mate or TB score from "plies to mate from the root"
    /// to "plies to mate from the current position". Standard scores are unchanged.
    /// The function is called before storing a value in the transposition table.
    fn from(value: Value, ply: Ply) -> Self {
        if value.is_known_win() {
            Self(
                NonMaxI16::new(value.as_inner() + ply.as_inner() as i16)
                    .expect("different to i16::MAX"),
            )
        } else if value.is_known_loss() {
            Self(
                NonMaxI16::new(value.as_inner() - ply.as_inner() as i16)
                    .expect("different to i16::MAX"),
            )
        } else {
            // SAFETY: `value` is neither very large, nor very small, so the conversion is safe.
            Self(unsafe { NonMaxI16::new_unchecked(value.as_inner()) })
        }
    }

    /// Convert from a [`TtValue`] to a [`Value`].
    ///
    /// Inverse of [`TtValue::from`] it adjusts a mate or TB score
    /// from the transposition table (which refers to the plies to mate/be mated from
    /// current position) to "plies to mate/be mated (TB win/loss) from the root".
    fn into(self, ply: Ply, halfmove_count: usize) -> Value {
        let value = Value::new(self.0.get());
        if value.is_known_win() {
            let value = value - Value::new(ply.as_inner() as i16);
            // Downgrade potentially false mates/tb wins.
            if value
                .mate_distance()
                .or_else(|| value.known_win_distance())
                .is_some_and(|dist| halfmove_count <= 100 && dist > 100 - halfmove_count)
            {
                return Value::known_win_in(Ply::MAX) - Value::ONE;
            }
            return value;
        } else if value.is_known_loss() {
            let value = value + Value::new(ply.as_inner() as i16);
            // Downgrade potentially false mates/tb losses.
            if (-value)
                .mate_distance()
                .or_else(|| (-value).known_win_distance())
                .is_some_and(|dist| halfmove_count <= 100 && dist > 100 - halfmove_count)
            {
                return Value::known_loss_in(Ply::MAX) + Value::ONE;
            }

            return value;
        }

        value
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_bound() {
        assert!(!(Bound::Lower & Bound::Upper));
        assert!(Bound::Lower & Bound::Lower);
        assert!(Bound::Upper & Bound::Upper);
        assert!(Bound::Lower & Bound::Exact);
        assert!(Bound::Upper & Bound::Exact);
        assert!(Bound::Exact & Bound::Exact);
    }

    #[test]
    fn test_move16() {
        for mv in [
            ChessMove::new(Square::A1, Square::H8, Some(Piece::Queen)),
            ChessMove::new(Square::A6, Square::A1, Some(Piece::Knight)),
            ChessMove::new(Square::H6, Square::B4, Some(Piece::Bishop)),
            ChessMove::new(Square::H7, Square::B3, Some(Piece::Rook)),
            ChessMove::new(Square::C7, Square::G8, None),
        ] {
            assert_eq!(mv, ChessMove::from(Move16::from(mv)));
        }
    }

    #[test]
    fn test_tt_value() {
        assert_eq!(
            Value::DRAW,
            TtValue::from(Value::DRAW, Ply::ONE).into(Ply::ONE, 0)
        );
        assert_eq!(
            Value::ONE,
            TtValue::from(Value::ONE, Ply::ONE).into(Ply::ONE, 0)
        );
        assert_eq!(
            Value::mated_in(Ply::ONE),
            TtValue::from(Value::mated_in(Ply::ONE), Ply::ONE).into(Ply::ONE, 0)
        );
        assert_eq!(
            Value::known_win_in(Ply::ONE),
            TtValue::from(Value::known_win_in(Ply::ONE), Ply::ONE).into(Ply::ONE, 0)
        );

        // Non-downgrade edge cases:
        assert_eq!(
            Value::mate_in(Ply::new(42)),
            TtValue::from(Value::mate_in(Ply::new(42)), Ply::ONE).into(Ply::ONE, 100 - 42)
        );
        assert_eq!(
            Value::mated_in(Ply::new(42)),
            TtValue::from(Value::mated_in(Ply::new(42)), Ply::ONE).into(Ply::ONE, 100 - 42)
        );

        // Downgrade examples.
        assert_eq!(
            Value::known_win_in(Ply::MAX) - Value::ONE,
            TtValue::from(Value::mate_in(Ply::MAX), Ply::ONE).into(Ply::ONE, 0)
        );
        assert_eq!(
            Value::known_win_in(Ply::MAX) - Value::ONE,
            TtValue::from(Value::known_win_in(Ply::MAX), Ply::ONE).into(Ply::ONE, 0)
        );
        assert_eq!(
            Value::known_loss_in(Ply::MAX) + Value::ONE,
            TtValue::from(Value::mated_in(Ply::MAX), Ply::ONE).into(Ply::ONE, 0)
        );
        assert_eq!(
            Value::known_loss_in(Ply::MAX) + Value::ONE,
            TtValue::from(Value::known_loss_in(Ply::MAX), Ply::ONE).into(Ply::ONE, 0)
        );

        // Downgrade edge cases.
        assert_eq!(
            Value::known_win_in(Ply::MAX) - Value::ONE,
            TtValue::from(Value::mate_in(Ply::new(42)), Ply::ONE).into(Ply::ONE, 100 - 41)
        );
        assert_eq!(
            Value::known_loss_in(Ply::MAX) + Value::ONE,
            TtValue::from(Value::mated_in(Ply::new(42)), Ply::ONE).into(Ply::ONE, 100 - 41)
        );
    }

    #[test]
    fn test_tt_value_halfmove_downgrading() {
        // Wins in 255 plies are downgraded regardless of the halfmove count.
        let v = Value::known_win_in(Ply::MAX);
        let ttv = TtValue::from(v, Ply::ONE);
        assert_eq!(
            Value::known_win_in(Ply::MAX) - Value::ONE,
            ttv.into(Ply::ONE, 0)
        );

        // Same for losses.
        let v = Value::known_loss_in(Ply::MAX);
        let ttv = TtValue::from(v, Ply::ONE);
        assert_eq!(
            Value::known_loss_in(Ply::MAX) + Value::ONE,
            ttv.into(Ply::ONE, 0)
        );

        for ply in (0..=u8::MAX).map(Ply::new) {
            for halfmove_count in 0..=100 {
                let v = Value::mate_in(ply);
                let ttv = TtValue::from(v, Ply::ONE);
                let remaining_plies = 100 - halfmove_count;

                let expected = if ply.as_inner() as usize > remaining_plies {
                    Value::known_win_in(Ply::MAX) - Value::ONE
                } else {
                    v
                };

                assert_eq!(
                    expected,
                    ttv.into(Ply::ONE, halfmove_count),
                    "mate_in({ply:?}) at halfmove_count={halfmove_count}"
                );
            }
        }
    }

    #[test]
    fn test_tt_basic() {
        const PV: bool = true;
        let mut tt = TranspositionTable::new(64);

        let key: Key = 0x1234_5678_9abc_def0;
        let (None, writer) = tt.probe_mut(key, Ply::ZERO, 0) else {
            panic!("expected None");
        };

        //// SAFETY: The table hasn't been dropped or resized since the writer was created, so the
        //// pointer is still valid. Same goes for the next few unsafe blocks.
        unsafe {
            writer.save::<PV>(&EntryWriterOpts {
                bound: Bound::Exact,
                depth: Depth::new(6),
                ply: Ply::ONE,
                mv: Some(ChessMove::new(Square::A1, Square::H8, Some(Piece::Queen))),
                value: Some(Value::ONE),
                eval: Some(Value::DRAW),
            });
        }

        let data = tt.probe(key, Ply::ZERO, 0).expect("expected Some");
        assert_eq!(Some(Value::ONE), data.value);
        assert_eq!(Some(Value::DRAW), data.eval);
        assert_eq!(
            Some(ChessMove::new(Square::A1, Square::H8, Some(Piece::Queen))),
            data.mv
        );
        assert_eq!(Depth::new(6), data.depth);
        assert_eq!(Bound::Exact, data.bound);
        assert!(data.is_pv);

        // Write an entry to the same key, but with bound+depth so that it isn't accepted.
        unsafe {
            writer.save::<PV>(&EntryWriterOpts {
                value: Some(Value::new(-42)),
                eval: Some(Value::ONE),
                ..Default::default() // Defaults to lower bound, depth 0, ply 0, no move
            });
        }

        let unchanged = tt.probe(key, Ply::ZERO, 0).expect("expected Some");
        assert_eq!(unchanged, data);

        tt.new_search();
        assert_eq!(tt.generation(), Entry::GENERATION_DELTA);
        let (Some(_), writer) = tt.probe_mut(key, Ply::ZERO, 0) else {
            panic!("entries should be preserved across generations");
        };

        // Write the same low-value entry. Now it should be accepted, as the previous is of the
        // last generation. But because we don't provide a move, the old move should still be preserved.
        // Need to fetch a new writer though, as the old one is no longer valid.
        unsafe {
            writer.save::<PV>(&EntryWriterOpts {
                value: Some(Value::new(-42)),
                eval: Some(Value::ONE),
                ..Default::default()
            });
        }

        let new = tt.probe(key, Ply::ZERO, 0).expect("expected Some");
        assert_eq!(Some(Value::new(-42)), new.value);
        assert_eq!(Some(Value::ONE), new.eval);
        assert_eq!(
            Some(ChessMove::new(Square::A1, Square::H8, Some(Piece::Queen))),
            new.mv
        );
        assert_eq!(Depth::ZERO, new.depth);
        assert_eq!(Bound::Lower, new.bound);
        assert!(new.is_pv);
    }
}
