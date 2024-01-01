use chess::ChessMove;
use uci::{Info, InfoPart};

use crate::{
    movelist::MoveEntry,
    value::{self, Depth},
};

#[derive(Debug)]
struct Logger {
    search_start_time: std::time::Instant,
    last_log_time: std::time::Instant,
    info_writer: uci::InfoWriter,
}

impl Logger {
    fn new(info_writer: uci::InfoWriter) -> Self {
        Self {
            info_writer,
            search_start_time: std::time::Instant::now(),
            last_log_time: std::time::Instant::now(),
        }
    }

    fn send_optional_status(
        &mut self,
        current_depth: Depth,
        current_max_depth: Depth,
        total_nodes: usize,
        tb_hits: usize,
        current_move: ChessMove,
        current_move_number: usize,
    ) {
        if self.last_log_time.elapsed().as_secs() >= 1 {
            self.send_status(
                current_depth,
                current_max_depth,
                total_nodes,
                tb_hits,
                current_move,
                current_move_number,
            );
        }
    }

    fn send_status(
        &mut self,
        current_depth: Depth,
        current_max_depth: Depth,
        total_nodes: usize,
        tb_hits: usize,
        current_move: ChessMove,
        current_move_number: usize,
    ) {
        let delta = self.search_start_time.elapsed();

        if delta.as_secs() < 1 {
            return;
        }

        let millis = delta.as_millis() as usize;
        let info = Info::new()
            .with(InfoPart::Depth(current_depth as usize))
            .with(InfoPart::SelDepth(current_max_depth as usize))
            .with(InfoPart::Nodes(total_nodes))
            .with(InfoPart::Time(millis))
            .with(InfoPart::Nps(total_nodes * 1000 / millis))
            .with(InfoPart::TbHits(tb_hits))
            .with(InfoPart::CurrMove(current_move))
            .with(InfoPart::CurrMoveNumber(current_move_number));

        self.info_writer.send_info(info);
        self.last_log_time = std::time::Instant::now();
    }

    fn send_move(
        &mut self,
        move_entry: &MoveEntry,
        current_depth: Depth,
        current_max_depth: Depth,
        total_nodes: usize,
        tb_hits: usize,
    ) {
        let millis = self.search_start_time.elapsed().as_millis() as usize;
        let mut info = Info::new()
            .with(InfoPart::Depth(current_depth as usize))
            .with(InfoPart::SelDepth(current_max_depth as usize))
            .with(InfoPart::Nodes(total_nodes))
            .with(InfoPart::Time(millis))
            .with(InfoPart::Nps(total_nodes * 1000 / millis))
            .with(InfoPart::TbHits(tb_hits));

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

        info = info.with(InfoPart::Pv(&move_entry.pv));
        self.info_writer.send_info(info);
        self.last_log_time = std::time::Instant::now();
    }
}
