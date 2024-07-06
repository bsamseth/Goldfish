use chess::ChessMove;
use uci::{Info, InfoPart};

use crate::{
    movelist::MoveEntry,
    newtypes::{Depth, Ply},
};

#[derive(Debug)]
pub struct Logger {
    pub search_start_time: std::time::Instant,
    last_log_time: std::time::Instant,
    pub total_nodes: usize,
    tb_hits: usize,
    current_depth: Depth,
    current_max_ply: Ply,
    current_move: ChessMove,
    current_move_number: usize,
    silent: bool,
}

impl Logger {
    pub fn new() -> Self {
        Self {
            search_start_time: std::time::Instant::now(),
            last_log_time: std::time::Instant::now(),
            total_nodes: 0,
            tb_hits: 0,
            current_depth: Depth::new(0),
            current_max_ply: Ply::new(0),
            current_move: ChessMove::default(),
            current_move_number: 0,
            silent: false,
        }
    }

    pub fn exceeds_node_limit(&self, node_limit: usize) -> bool {
        self.total_nodes >= node_limit
    }

    pub fn exceeds_time_limit(&self, time_limit: usize) -> bool {
        self.search_start_time.elapsed().as_millis() as usize >= time_limit
    }

    pub fn tb_hit(&mut self) {
        self.tb_hits += 1;
    }

    pub fn set_current_depth(&mut self, current_depth: Depth) {
        self.current_depth = current_depth;
        self.current_max_ply = Ply::new(0);
    }

    pub fn set_current_move(&mut self, current_move: ChessMove, current_move_number: usize) {
        self.current_move = current_move;
        self.current_move_number = current_move_number;
    }

    pub fn update_search(&mut self, current_ply: Ply) {
        self.total_nodes += 1;
        self.current_max_ply = self.current_max_ply.max(current_ply);

        self.send_status();
    }

    pub fn silent(mut self, silent: bool) -> Self {
        self.silent = silent;
        self
    }

    pub fn send_status(&mut self) {
        if !self.silent && self.last_log_time.elapsed().as_secs() >= 1 {
            self.force_send_status();
        }
    }

    pub fn force_send_status(&mut self) {
        let delta = self.search_start_time.elapsed();
        let millis = delta.as_millis() as usize;
        let info = Info::new()
            .with(InfoPart::Depth(self.current_depth.as_usize()))
            .with(InfoPart::SelDepth(self.current_max_ply.as_usize()))
            .with(InfoPart::Nodes(self.total_nodes))
            .with(InfoPart::Time(millis))
            .with(InfoPart::Nps(if millis > 0 {
                self.total_nodes * 1000 / millis
            } else {
                0
            }))
            .with(InfoPart::TbHits(self.tb_hits))
            .with(InfoPart::CurrMove(self.current_move))
            .with(InfoPart::CurrMoveNumber(self.current_move_number));

        println!("{info}");
        self.last_log_time = std::time::Instant::now();
    }

    pub fn send_move(&mut self, move_entry: &MoveEntry, pv: &[ChessMove], hashfull: usize) {
        if self.silent {
            return;
        }

        let millis = self.search_start_time.elapsed().as_millis() as usize;
        let mut info = Info::new()
            .with(InfoPart::Depth(self.current_depth.as_usize()))
            .with(InfoPart::SelDepth(self.current_max_ply.as_usize()))
            .with(InfoPart::Nodes(self.total_nodes))
            .with(InfoPart::HashFull(hashfull))
            .with(InfoPart::Time(millis))
            .with(InfoPart::Nps(if millis > 0 {
                self.total_nodes * 1000 / millis
            } else {
                0
            }))
            .with(InfoPart::TbHits(self.tb_hits));

        if move_entry.value.is_checkmate() {
            info = info.with(InfoPart::Mate(move_entry.value.mate_depth()));
        } else {
            info = info.with(InfoPart::Cp(move_entry.value.as_inner()));
        }

        info = info.with(InfoPart::Pv(pv));
        println!("{info}");
        self.last_log_time = std::time::Instant::now();
    }
}
