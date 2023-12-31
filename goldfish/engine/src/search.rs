#![allow(dead_code)]
use chess::{ChessMove, Game, Square};

use crate::stop_signal::StopSignal;
use uci::{Info, InfoPart};

#[derive(Debug)]
pub struct Searcher {
    game: Game,
    options: Vec<uci::GoOption>,
    info_writer: uci::InfoWriter,
    stop_signal: StopSignal,
}

impl Searcher {
    pub fn new(
        game: Game,
        options: Vec<uci::GoOption>,
        info_writer: uci::InfoWriter,
        stop_signal: StopSignal,
    ) -> Self {
        Self {
            game,
            options,
            info_writer,
            stop_signal,
        }
    }

    pub fn start(&mut self) {
        // TODO: Implement search
        for i in 0..10 {
            if self.stop_signal.check() {
                break;
            }
            self.info_writer
                .send_info(Info::new().with(InfoPart::Depth(i)));
            std::thread::sleep(std::time::Duration::from_millis(100));
        }

        self.info_writer
            .send_info(Info::new().with(InfoPart::Pv(&[ChessMove::new(
                Square::E2,
                Square::E4,
                None,
            )])));
    }
}
