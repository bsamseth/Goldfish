use chess::ChessMove;
use uci::{Info, InfoPart};

use crate::{
    movelist::MoveEntry,
    value::{self, Depth},
};

#[derive(Debug)]
pub struct Logger {
    info_writer: uci::InfoWriter,
    pub search_start_time: std::time::Instant,
    last_log_time: std::time::Instant,
    pub total_nodes: usize,
    tb_hits: usize,
    current_depth: Depth,
    current_max_depth: Depth,
    current_move: ChessMove,
    current_move_number: usize,
}

impl Logger {
    pub fn new(info_writer: uci::InfoWriter) -> Self {
        Self {
            info_writer,
            search_start_time: std::time::Instant::now(),
            last_log_time: std::time::Instant::now(),
            total_nodes: 0,
            tb_hits: 0,
            current_depth: 0,
            current_max_depth: 0,
            current_move: ChessMove::default(),
            current_move_number: 0,
        }
    }

    pub fn set_current_depth(&mut self, current_depth: Depth) {
        self.current_depth = current_depth;
        self.current_max_depth = 0;
    }

    pub fn set_current_move(&mut self, current_move: ChessMove, current_move_number: usize) {
        self.current_move = current_move;
        self.current_move_number = current_move_number;
    }

    pub fn update_search(&mut self, ply: Depth) {
        self.total_nodes += 1;
        self.current_max_depth = self.current_max_depth.max(ply);

        self.send_optional_status()
    }

    pub fn send_optional_status(&mut self) {
        if self.last_log_time.elapsed().as_secs() >= 1 {
            self.send_status();
        }
    }

    pub fn send_status(&mut self) {
        let delta = self.search_start_time.elapsed();

        if delta.as_secs() < 1 {
            return;
        }

        let millis = delta.as_millis() as usize;
        let info = Info::new()
            .with(InfoPart::Depth(self.current_depth as usize))
            .with(InfoPart::SelDepth(self.current_max_depth as usize))
            .with(InfoPart::Nodes(self.total_nodes))
            .with(InfoPart::Time(millis))
            .with(InfoPart::Nps(self.total_nodes * 1000 / millis))
            .with(InfoPart::TbHits(self.tb_hits))
            .with(InfoPart::CurrMove(self.current_move))
            .with(InfoPart::CurrMoveNumber(self.current_move_number));

        self.info_writer.send_info(info);
        self.last_log_time = std::time::Instant::now();
    }

    pub fn send_move(&mut self, move_entry: &MoveEntry, pv: &[ChessMove]) {
        let millis = self.search_start_time.elapsed().as_millis() as usize;
        let mut info = Info::new()
            .with(InfoPart::Depth(self.current_depth as usize))
            .with(InfoPart::SelDepth(self.current_max_depth as usize))
            .with(InfoPart::Nodes(self.total_nodes))
            .with(InfoPart::Time(millis))
            .with(InfoPart::Nps(if millis > 0 {
                self.total_nodes * 1000 / millis
            } else {
                0
            }))
            .with(InfoPart::TbHits(self.tb_hits));

        if move_entry.value.abs() >= value::CHECKMATE_THRESHOLD {
            let mate_depth = (value::CHECKMATE - move_entry.value.abs() + 1) / 2;
            let mate_depth = if move_entry.value > 0 {
                mate_depth
            } else {
                -mate_depth
            };
            info = info.with(InfoPart::Mate(mate_depth));
        } else {
            info = info.with(InfoPart::Cp(move_entry.value));
        }

        info = info.with(InfoPart::Pv(pv));
        self.info_writer.send_info(info);
        self.last_log_time = std::time::Instant::now();
    }
}
