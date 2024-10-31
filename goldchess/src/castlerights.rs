/// Represent what castling rights are available to a player.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum CastleRights {
    None,
    KingSide,
    QueenSide,
    Both,
}
