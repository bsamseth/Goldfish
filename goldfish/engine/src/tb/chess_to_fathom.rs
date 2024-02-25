/// Convert a [`chess::Board`] and halfmove clock to a [`fathom_syzygy::Position`].
pub fn fathom_position(board: &chess::Board, halfmove_clock: usize) -> fathom_syzygy::Position {
    fathom_syzygy::Position {
        white: board.color_combined(chess::Color::White).0,
        black: board.color_combined(chess::Color::Black).0,
        kings: board.pieces(chess::Piece::King).0,
        queens: board.pieces(chess::Piece::Queen).0,
        rooks: board.pieces(chess::Piece::Rook).0,
        bishops: board.pieces(chess::Piece::Bishop).0,
        knights: board.pieces(chess::Piece::Knight).0,
        pawns: board.pieces(chess::Piece::Pawn).0,
        rule50: u32::try_from(halfmove_clock).expect("halfmove clock should fit in u32"),
        castling: fathom_combined_castle_rights(
            board.castle_rights(chess::Color::White),
            board.castle_rights(chess::Color::Black),
        ),
        ep: board.en_passant().map_or(0, |sq| u32::from(sq.to_int())),
        turn: u8::from(board.side_to_move() == chess::Color::White),
    }
}

/// Convert a pair of [`chess::CastleRights`] to a single `u32`, as expected by fathom.
fn fathom_combined_castle_rights(white: chess::CastleRights, black: chess::CastleRights) -> u32 {
    const WHITE_KING: u32 = 0x1;
    const BLACK_KING: u32 = 0x4;
    const WHITE_QUEEN: u32 = 0x2;
    const BLACK_QUEEN: u32 = 0x8;

    let mut rights = 0;
    match white {
        chess::CastleRights::Both => rights |= WHITE_KING | WHITE_QUEEN,
        chess::CastleRights::KingSide => rights |= WHITE_KING,
        chess::CastleRights::QueenSide => rights |= WHITE_QUEEN,
        chess::CastleRights::NoRights => {}
    }

    match black {
        chess::CastleRights::Both => rights |= BLACK_KING | BLACK_QUEEN,
        chess::CastleRights::KingSide => rights |= BLACK_KING,
        chess::CastleRights::QueenSide => rights |= BLACK_QUEEN,
        chess::CastleRights::NoRights => {}
    }

    rights
}
