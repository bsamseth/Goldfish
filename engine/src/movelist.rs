use chess::{ChessMove, MoveGen};

use crate::newtypes::Value;

#[derive(Debug)]
pub struct MoveVec<V = Value>(Vec<MoveEntry<V>>);

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Ord)]
pub struct MoveEntry<V = Value> {
    pub value: V,
    pub mv: ChessMove,
}

impl MoveEntry {
    pub fn new(mv: ChessMove) -> Self {
        Self {
            mv,
            value: -Value::INFINITE,
        }
    }
}

impl<V> From<Vec<MoveEntry<V>>> for MoveVec<V> {
    fn from(vec: Vec<MoveEntry<V>>) -> Self {
        Self(vec)
    }
}

impl From<MoveGen> for MoveVec {
    fn from(move_gen: MoveGen) -> Self {
        Self(move_gen.map(MoveEntry::new).collect())
    }
}

impl<V> IntoIterator for MoveVec<V> {
    type Item = MoveEntry<V>;
    type IntoIter = std::vec::IntoIter<MoveEntry<V>>;

    fn into_iter(self) -> Self::IntoIter {
        self.0.into_iter()
    }
}

impl<V> std::ops::Deref for MoveVec<V> {
    type Target = Vec<MoveEntry<V>>;

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl<V> std::ops::DerefMut for MoveVec<V> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}
