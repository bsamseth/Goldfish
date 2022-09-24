use std::io;
use std::io::BufRead;
use std::str::FromStr;
use std::sync::{mpsc, Arc, Mutex};
use std::thread;
use std::thread::JoinHandle;

use chess::Game;
use vampirc_uci::{parse_one, UciMessage};

use crate::search::search;

pub fn driver(engine_tx: mpsc::Sender<UciMessage>) {
    let mut board = None;
    let stop = Arc::new(Mutex::new(false));
    let mut search_thread = None;

    // We loop forever, and only break out if the stdin channel is terminated.
    for line in io::stdin().lock().lines() {
        let msg = parse_one(&line.unwrap());

        match msg {
            UciMessage::Uci => {
                let name = UciMessage::id_name(&format!("Goldfish {}", env!("CARGO_PKG_VERSION")));
                let author = UciMessage::id_author(env!("CARGO_PKG_AUTHORS"));
                println!("{}", name);
                println!("{}", author);

                // TODO: Output options
                println!("{}", UciMessage::UciOk);
            }

            UciMessage::IsReady => {
                // Make sure we are ready, then say so.
                println!("{}", UciMessage::ReadyOk);
            }

            UciMessage::Position {
                startpos,
                fen,
                moves,
            } => {
                let mut game = if let Some(fen) = fen {
                    Game::from_str(&fen.to_string()).expect("Erroneous FEN string")
                } else {
                    assert!(
                        startpos,
                        "position must be followed by either `startpos` or a valid FEN string."
                    );
                    Game::new()
                };

                for m in moves {
                    game.make_move(m);
                }

                board = Some(game.current_position());
            }

            UciMessage::Go {
                time_control,
                search_control,
            } => {
                // Ensure the stopping marker is not set.
                {
                    let mut stop = stop.lock().unwrap();
                    *stop = false;
                }

                // Dispatch a search in a new thread. We store the handle to join with it when we stop the search.
                let stop = stop.clone();
                let engine_tx = engine_tx.clone();
                search_thread = Some(thread::spawn(move || {
                    search(
                        board.expect(
                            "Can't start searching without a position being specified first!",
                        ),
                        time_control,
                        search_control,
                        engine_tx,
                        stop,
                    )
                }));
            }

            UciMessage::Stop => {
                // Set "stop" variable to true, then join with the search thread.
                // The search thread should ensure to output bestmove
                {
                    let mut stop = stop.lock().unwrap();
                    *stop = true;
                }

                // If a stop was sent without a running search, just ignore the command.
                search_thread.take().map(JoinHandle::join);
            }

            UciMessage::Quit => {
                break;
            }

            _ => {
                // We don't handle any other messages, so just ignore them.
            }
        }
    }
}
