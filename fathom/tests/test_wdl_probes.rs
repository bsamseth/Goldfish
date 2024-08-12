use std::sync::OnceLock;

use chess::Board;

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

fn test(fen: &str, expected_wdl: Wdl) {
    let board: Board = fen.parse().unwrap();
    let wdl = tb().probe_wdl(&board, 0).unwrap();

    assert_eq!(wdl, expected_wdl);
}

macro_rules! test_wdl_probe {
    ( $name:ident, $fen:expr, $wdl:expr ) => {
        #[test]
        fn $name() {
            test($fen, $wdl)
        }
    };
}

test_wdl_probe!(
    single_winning_move,
    "8/8/8/8/5kp1/P7/8/1K1N4 w - - 0 1",
    Wdl::Win
);
test_wdl_probe!(
    single_winning_move_dtz_99,
    "8/8/8/5N2/8/p7/8/2NK3k w - - 0 1",
    Wdl::Win
);
test_wdl_probe!(
    draw_all_moves,
    "8/3k4/8/8/8/4B3/4KB2/2B5 w - - 0 1",
    Wdl::Draw
);
test_wdl_probe!(
    several_winning_moves,
    "K7/2kB4/8/8/8/8/8/5N2 w - - 0 1",
    Wdl::Win
);
test_wdl_probe!(
    blessed_loss_with_underpromotion,
    "8/6B1/8/8/B7/8/K1pk4/8 b - - 0 1",
    Wdl::BlessedLoss
);
test_wdl_probe!(
    en_passant_only_winning_move,
    "8/8/8/k1pPp3/8/K7/8/8 w - e6 0 1",
    Wdl::Win
);
test_wdl_probe!(
    en_passant_only_drawing_move,
    "8/8/8/k1pPp3/8/K7/8/8 w - c6 0 1",
    Wdl::Draw
);
