use rand::seq::IteratorRandom;
use std::sync::{mpsc, Arc, Mutex};
use std::{thread, time};

use chess::{Board, MoveGen};
use vampirc_uci::{UciSearchControl, UciTimeControl};

use crate::io::EngineResponse;

pub fn search(
    board: Board,
    _: Option<UciTimeControl>,
    _: Option<UciSearchControl>,
    engine_tx: mpsc::Sender<EngineResponse>,
    stop: Arc<Mutex<bool>>,
) {
    loop {
        {
            let stop = stop.lock().unwrap();

            if *stop {
                break;
            }
        }
        thread::sleep(time::Duration::from_millis(50));
    }

    // Placeholder:
    let mut rng = rand::thread_rng();
    let bestmove = MoveGen::new_legal(&board)
        .into_iter()
        .choose(&mut rng)
        .expect("No legal moves in position!");

    // Failing to send is a crash.
    engine_tx.send(EngineResponse::BestMove(bestmove)).unwrap();
}
