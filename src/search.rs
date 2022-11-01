use std::sync::atomic::AtomicBool;
use std::sync::{mpsc, Arc};
use std::time::Instant;

use chess::{Board, ChessMove, MoveGen};
use vampirc_uci::{UciInfoAttribute, UciMessage, UciSearchControl, UciTimeControl};

use crate::eval;

const MAX_DEPTH: u8 = u8::MAX - 1;

#[derive(Debug, Clone, Default, PartialEq)]
struct Pv(Vec<ChessMove>);

impl Pv {
    fn update(&mut self, m: ChessMove, pv: &Pv) {
        self.0.clear();
        self.0.push(m);
        self.0.extend(&pv.0);
    }
}

pub struct Search {
    stopping_condition: Arc<AtomicBool>,
    engine_tx: mpsc::Sender<UciMessage>,

    last_update: Instant,
    pvs: Vec<Pv>,
}

impl Search {
    fn new(stop: Arc<AtomicBool>, engine_tx: mpsc::Sender<UciMessage>) -> Self {
        let mut pvs = Vec::with_capacity(MAX_DEPTH as usize);
        for _ in 0..MAX_DEPTH {
            pvs.push(Pv::default());
        }

        Search {
            stopping_condition: stop,
            engine_tx,

            last_update: Instant::now(),
            pvs,
        }
    }
}

impl Search {
    /// Return true if the stop condition is set.
    fn check_stop(&self) -> bool {
        self.stopping_condition
            .load(std::sync::atomic::Ordering::Relaxed)
    }

    fn save_pv(&mut self, m: ChessMove, depth_limit: u8, depth: u8, score: i32) {
        let depth = depth as usize;

        // Save pv at this depth as m followed by the pv at the next depth.
        // Need to split the pvs vector to be allowed to mutate on pv based on another.
        let (left, right) = self.pvs.split_at_mut(depth + 1);
        left[depth].update(m, &right[0]);

        // Conditionally send info if enough time has elapsed since the last update.
        if self.last_update.elapsed().as_millis() < 1000 {
            return;
        }
        self.last_update = std::time::Instant::now();

        // Send info to output thread.
        self.engine_tx
            .send(UciMessage::Info(vec![
                UciInfoAttribute::Depth(depth_limit as u8),
                UciInfoAttribute::SelDepth(depth as u8),
                eval::score_as_uci_info(score),
                UciInfoAttribute::Pv(self.pvs[depth].0.clone()),
            ]))
            .unwrap();
    }

    /// Run a depth-bound search.
    fn search_to_depth(
        &mut self,
        board: &Board,
        depth: u8,
        current_depth: u8,
        alpha: i32,
        beta: i32,
    ) -> i32 {
        debug_assert!(current_depth <= depth);
        debug_assert!(alpha < beta);

        let moves = MoveGen::new_legal(board);

        // Check for checkmate or stalemate.
        if moves.len() == 0 {
            if board.checkers().to_size(0) == 0 {
                return 0;
            } else {
                return -eval::mate_in_ply(current_depth);
            }
        }

        // Check for depth limit.
        if current_depth == depth {
            return eval::eval(board);
        }

        // Mate distance pruning:
        // Even if we mate at the next move our score would be at best CHECKMATE - ply, but if
        // alpha is already bigger because a shorter mate was found upward in the tree then there is no
        // need to search because we will never beat the current alpha. Same logic but with reversed signs
        // applies also in the opposite condition of being mated instead of giving mate. In this case
        // return a fail-high score.
        let mut alpha = std::cmp::max(-eval::mate_in_ply(current_depth), alpha);
        let beta = std::cmp::min(eval::mate_in_ply(current_depth + 1), beta);
        if alpha >= beta {
            return alpha;
        }

        // Search each move recursively.
        let next_board = &mut Board::default(); // Reuse the same board to avoid allocations inside the loop.
        let mut best_score = -eval::mate_in_ply(0);
        for m in moves {
            // Check for stop condition.
            if self.check_stop() {
                break;
            }

            board.make_move(m, next_board);
            let value = -self.search_to_depth(next_board, depth, current_depth + 1, -beta, -alpha);

            if value > best_score {
                best_score = value;

                // Beta cutoff implies we don't need to consider this node at all, and we
                // should not save this as part of the PV either.
                if value >= beta {
                    break;
                }

                if value > alpha {
                    alpha = value;

                    self.save_pv(
                        m,
                        depth,
                        current_depth,
                        eval::normalize_score(value, board.side_to_move()),
                    );
                }
            }
        }

        best_score
    }
}

/// Search the given board for the best move.
///
/// The search is stopped when the stop condition is set, or when stopping is suitable based on the
/// time control or search control.
///
/// Search information, including the bestmove output, is sent to the engine_tx channel.
pub fn search(
    board: Board,
    _: Option<UciTimeControl>,
    search_control: Option<UciSearchControl>,
    engine_tx: mpsc::Sender<UciMessage>,
    stop: Arc<AtomicBool>,
) {
    let mut search_controller = Search::new(stop, engine_tx.clone());

    let max_depth = search_control
        .map(|c| c.depth.unwrap_or(MAX_DEPTH))
        .unwrap_or(MAX_DEPTH);

    for depth in 1..=max_depth {
        let score = eval::normalize_score(
            search_controller.search_to_depth(&board, depth, 0, i32::MIN + 1, i32::MAX - 1),
            board.side_to_move(),
        );

        engine_tx
            .send(UciMessage::Info(vec![
                UciInfoAttribute::Depth(depth as u8),
                eval::score_as_uci_info(score),
                UciInfoAttribute::Pv(search_controller.pvs[0].0.clone()),
            ]))
            .unwrap();

        if search_controller.check_stop() {
            break;
        }
    }
    engine_tx
        .send(UciMessage::best_move(search_controller.pvs[0].0[0]))
        .unwrap();
}
