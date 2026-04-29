use std::str::FromStr;

use crate::{Bitboard, CastleRights, Color, Error, File, Piece, Rank, Square};
use regex::Regex;

/// Representation of a chess position (a.k.a. the struct you care about).
#[derive(Debug, Clone)]
pub struct Position {
    pieces: [Bitboard; Piece::NUM_PIECES],
    color_combined: [Bitboard; Color::NUM_COLORS],
    side_to_move: Color,
    castle_rights: [CastleRights; Color::NUM_COLORS],
    en_passant: Option<Square>,
    halfmove_clock: u8,
}

impl Position {
    #[must_use]
    pub fn piece_on(&self, sq: Square) -> Option<Piece> {
        Piece::ALL
            .into_iter()
            .find(|piece| self.pieces[piece.as_index()] & sq != Bitboard::EMPTY)
    }

    #[must_use]
    pub fn color_on(&self, sq: Square) -> Option<Color> {
        Color::ALL
            .into_iter()
            .find(|color| self.color_combined[color.as_index()] & sq != Bitboard::EMPTY)
    }

    pub const STARTING_POSITION_FEN: &str =
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
}

impl Default for Position {
    fn default() -> Self {
        Position::from_str(Position::STARTING_POSITION_FEN).unwrap()
    }
}

impl FromStr for Position {
    type Err = Error;

    #[expect(clippy::too_many_lines)]
    fn from_str(value: &str) -> Result<Self, Self::Err> {
        let mut board = [None; Square::NUM_SQUARES];
        let invalid_fen = || Error::InvalidFen(value.to_string());
        if !Regex::new(
            r"(?x)  # Verbose regex mode.
            ^
            ([pnbrqkPNBRQK1-8]{1,8}/){7}[pnbrqkPNBRQK1-8]{1,8}\  # Piece positions.
            [wb]\                # Side to move.
            (K?Q?k?q?|-)\        # Castle rights.
            ([a-h][36]|-)\       # En-passant square.
            \d+\                 # Half-move clock.
            \d+                  # Full-move clock.
            $",
        )
        .unwrap()
        .is_match(value)
        {
            return Err(invalid_fen());
        }

        let tokens: Vec<&str> = value.split(' ').collect();
        if tokens.len() < 4 {
            return Err(invalid_fen());
        }

        let pieces = tokens[0];
        let side = tokens[1];
        let castles = tokens[2];
        let ep = tokens[3];
        let halfmove = tokens[4];
        let mut cur_rank = Rank::R8;
        let mut cur_file = File::A;

        for x in pieces.chars() {
            match x {
                '/' => {
                    cur_rank = cur_rank
                        .down()
                        .ok_or_else(|| Error::InvalidFen(value.to_string()))?;
                    cur_file = File::A;
                }
                '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' => {
                    cur_file = File::new(
                        u8::try_from(cur_file.0 as usize + (x as usize) - ('0' as usize))
                            .map_err(|_| invalid_fen())?
                            % 8,
                    )
                    .map_err(|_| invalid_fen())?;
                }
                x => {
                    let (piece, color) = match x {
                        'r' => (Piece::Rook, Color::Black),
                        'R' => (Piece::Rook, Color::White),
                        'n' => (Piece::Knight, Color::Black),
                        'N' => (Piece::Knight, Color::White),
                        'b' => (Piece::Bishop, Color::Black),
                        'B' => (Piece::Bishop, Color::White),
                        'q' => (Piece::Queen, Color::Black),
                        'Q' => (Piece::Queen, Color::White),
                        'k' => (Piece::King, Color::Black),
                        'K' => (Piece::King, Color::White),
                        'p' => (Piece::Pawn, Color::Black),
                        'P' => (Piece::Pawn, Color::White),
                        _ => {
                            return Err(invalid_fen());
                        }
                    };
                    board[Square::make_square(cur_rank, cur_file).as_index()] =
                        Some((piece, color));
                    cur_file = cur_file.right().unwrap_or(File::A);
                }
            }
        }
        let side_to_move = match side {
            "w" | "W" => Color::White,
            "b" | "B" => Color::Black,
            _ => {
                return Err(invalid_fen());
            }
        };

        let mut castle_rights = [CastleRights::None; Color::NUM_COLORS];
        if castles.contains('K') && castles.contains('Q') {
            castle_rights[Color::White.as_index()] = CastleRights::Both;
        } else if castles.contains('K') {
            castle_rights[Color::White.as_index()] = CastleRights::KingSide;
        } else if castles.contains('Q') {
            castle_rights[Color::White.as_index()] = CastleRights::QueenSide;
        }

        if castles.contains('k') && castles.contains('q') {
            castle_rights[Color::Black.as_index()] = CastleRights::Both;
        } else if castles.contains('k') {
            castle_rights[Color::Black.as_index()] = CastleRights::KingSide;
        } else if castles.contains('q') {
            castle_rights[Color::Black.as_index()] = CastleRights::QueenSide;
        }

        let ep_square = if ep == "-" {
            None
        } else {
            Some(Square::from_str(ep).map_err(|_| invalid_fen())?)
        };

        let halfmove_clock = halfmove.parse::<u8>().map_err(|_| invalid_fen())?;

        let mut pieces = [Bitboard(0); Piece::NUM_PIECES];
        let mut color_combined = [Bitboard(0); Color::NUM_COLORS];
        for (&piece, sq) in board.iter().zip(Square::ALL_SQUARES) {
            if let Some((piece, color)) = piece {
                pieces[piece.as_index()] |= sq;
                color_combined[color.as_index()] |= sq;
            }
        }

        Ok(Position {
            pieces,
            color_combined,
            side_to_move,
            castle_rights,
            en_passant: ep_square,
            halfmove_clock,
        })
    }
}

impl std::fmt::Display for Position {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        for rank in Rank::ALL.into_iter().rev() {
            let mut empty = 0;
            for file in File::ALL {
                let sq = Square::make_square(rank, file);
                match self.piece_on(sq) {
                    None => empty += 1,
                    Some(piece) => {
                        if empty > 0 {
                            write!(f, "{empty}")?;
                            empty = 0;
                        }
                        let color = self.color_on(sq).unwrap();
                        let c = piece.as_char();
                        write!(
                            f,
                            "{}",
                            if color == Color::Black {
                                c.to_ascii_lowercase()
                            } else {
                                c
                            }
                        )?;
                    }
                }
            }
            if empty > 0 {
                write!(f, "{empty}")?;
            }
            if rank != Rank::R1 {
                write!(f, "/")?;
            }
        }

        write!(f, " {} ", self.side_to_move)?;

        match self.castle_rights {
            [CastleRights::None, CastleRights::None] => write!(f, "-")?,
            [white, black] => {
                match white {
                    CastleRights::KingSide => write!(f, "K")?,
                    CastleRights::QueenSide => write!(f, "Q")?,
                    CastleRights::Both => write!(f, "KQ")?,
                    CastleRights::None => {}
                }

                match black {
                    CastleRights::KingSide => write!(f, "k")?,
                    CastleRights::QueenSide => write!(f, "q")?,
                    CastleRights::Both => write!(f, "kq")?,
                    CastleRights::None => {}
                }
            }
        }

        if let Some(ep) = self.en_passant {
            write!(f, " {ep}")?;
        } else {
            write!(f, " -")?;
        }

        write!(f, " {}", self.halfmove_clock)?;
        write!(f, " 1") // Full move clock, not stored here so just write 1 to be FEN-compliant.
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn from_valid_fens_roundtrip() {
        for fen in [
            "2n1k3/8/8/8/8/8/8/2N1K3 w - - 0 1",
            "5B2/6P1/1p6/8/1N6/kP6/2K5/8 w - - 0 1",
            "6R1/P2k4/r7/5N1P/r7/p7/7K/8 w - - 0 1",
            "7K/8/k1P5/7p/8/8/8/8 w - - 0 1",
            "7k/8/8/8/8/8/8/7K w - - 0 1",
            "8/5k2/3p4/1pPPp2p/pP2Pp1P/P4P1K/8/8 b - b6 99 1",
            "8/8/7p/3KNN1k/2p4p/8/3P2p1/8 w - - 0 1",
            "r4rk1/1pp2ppp/3b4/3P4/3p4/3B4/1PP2PPP/R4RK1 b - - 0 1",
            "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b - - 1 1",
            "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 1",
            "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b Kkq - 1 1",
            "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R w kq - 10 1",
            "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 1",
            "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        ] {
            let pos = Position::from_str(fen).unwrap();
            assert_eq!(pos.to_string(), fen);
            println!("{pos}");
        }
    }
    #[test]
    fn reject_bad_fens() {
        for fen in [
            "2n1k3/8/8/8/8/8/8/2N1K3 w - z2 0 1",
            "5B2/6P1/1p6/8/1N6/kP6/2K5/8 w KQpq - 0 1",
            "6R1/P2k4/r7/5N1P/r7/p7/7K/8 z - - 0 1",
            "7K/8/k1P5/7p/8/8/9/8 w - - 0 1",
            "7l/8/8/8/8/8/8/7K w - - 0 1",
            "8/5k2/3p4/1pPPp2p/pP2Pp1P/P4P1K/8/8/8 b - b6 99 1",
        ] {
            if let Ok(p) = Position::from_str(fen) {
                panic!("Bad FEN string was accepted: {fen}\nParsed into this position: {p}");
            }
        }
    }
}
