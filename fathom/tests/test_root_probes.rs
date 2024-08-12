use std::sync::OnceLock;

use chess::{Board, ChessMove, MoveGen};

use fathom::{Tablebase, Wdl};

fn tb() -> &'static Tablebase {
    static mut TB: OnceLock<&Tablebase> = OnceLock::new();
    unsafe {
        TB.get_or_init(|| {
            Tablebase::load(concat!(env!("CARGO_MANIFEST_DIR"), "/../syzygy"))
                .unwrap()
                .as_mut()
                .unwrap()
        })
    }
}

fn test(
    fen: &str,
    expected_wdl: Wdl,
    expected_moves: &[&str],
) -> Result<(), Box<dyn std::error::Error>> {
    let board: Board = fen.parse().unwrap();
    let halfmove_clock = fen.split(' ').nth(4).unwrap().parse::<usize>()?;
    let expected_moves = if expected_moves.len() == 1 && expected_moves[0] == "ALL" {
        MoveGen::new_legal(&board).collect::<Vec<_>>()
    } else {
        expected_moves
            .iter()
            .map(|&m| ChessMove::from_san(&board, m).unwrap())
            .collect()
    };

    let (wdl, filter) = tb().probe_dtz(&board, halfmove_clock).unwrap();
    assert_eq!(wdl, expected_wdl);

    let moves = MoveGen::new_legal(&board)
        .filter(|mv| filter(mv))
        .collect::<Vec<_>>();
    assert_eq!(moves.len(), expected_moves.len());
    for m in &expected_moves {
        if !moves.contains(m) {
            eprintln!("Expected move not found: {m}");
        }
        assert!(moves.contains(m));
    }
    Ok(())
}

macro_rules! test_root_probe {
    ( $name:ident, $fen:expr, $wdl:expr, $( $mv:expr ),* ) => {
        #[test]
        fn $name() -> Result<(), Box<dyn std::error::Error>> {
            test($fen, $wdl, &[$( $mv ),*])
        }
    };
}

test_root_probe!(
    single_winning_move,
    "8/8/8/8/5kp1/P7/8/1K1N4 w - - 0 1",
    Wdl::Win,
    "Kc2"
);
test_root_probe!(
    single_cursed_winning_move,
    "8/8/8/8/5kp1/P7/8/1K1N4 w - - 96 1",
    Wdl::CursedWin,
    "Kc2"
);
test_root_probe!(
    single_winning_move_dtz_99,
    "8/8/8/5N2/8/p7/8/2NK3k w - - 0 1",
    Wdl::Win,
    "Na2"
);
test_root_probe!(
    single_cursed_winning_move_dtz_99,
    "8/8/8/5N2/8/p7/8/2NK3k w - - 1 1",
    Wdl::CursedWin,
    "Na2"
);
test_root_probe!(
    draw_all_moves,
    "8/3k4/8/8/8/4B3/4KB2/2B5 w - - 0 1",
    Wdl::Draw,
    "ALL"
);
test_root_probe!(
    two_cursed_winning_moves,
    "k7/7p/8/8/8/8/1NN5/7K w - - 0 1",
    Wdl::CursedWin,
    "Nd3",
    "Nd1"
);
test_root_probe!(
    four_winning_three_cursed,
    "K7/2kB4/8/8/8/8/8/5N2 w - - 35 1",
    Wdl::Win,
    "Bh3",
    "Bb5",
    "Bf5",
    "Be6"
);
test_root_probe!(
    seven_cursed_winning_moves,
    // BUG: According to Lichess, this is cursed even with hmc set to 36.
    "K7/2kB4/8/8/8/8/8/5N2 w - - 37 1",
    Wdl::CursedWin,
    "Bh3",
    "Bb5",
    "Bf5",
    "Be6",
    "Ba4",
    "Bg4",
    "Be8"
);
test_root_probe!(
    blessed_loss_with_underpromotion,
    "8/6B1/8/8/B7/8/K1pk4/8 b - - 0 1",
    Wdl::BlessedLoss,
    "c1N+"
);
test_root_probe!(
    en_passant_only_winning_move,
    "8/8/8/k1pPp3/8/K7/8/8 w - e6 0 1",
    Wdl::Win,
    "dxe6 e.p."
);
test_root_probe!(
    en_passant_only_drawing_move,
    "8/8/8/k1pPp3/8/K7/8/8 w - c6 0 1",
    Wdl::Draw,
    "dxc6 e.p."
);
