use rand::seq::IteratorRandom;
use std::io::BufRead;
use std::str::FromStr;
use std::sync::{mpsc, Arc, Mutex};
use std::thread::JoinHandle;
use std::{io, thread, time};

use chess::{Board, ChessMove, Game, MoveGen};
use vampirc_uci::{parse_one, UciMessage, UciSearchControl, UciTimeControl};

enum GuiActionRequest {
    AreYouReady,
    SetPosition(Board),
    StartSearching(Option<UciTimeControl>, Option<UciSearchControl>),
    StopSearching,
}

enum EngineResponse {
    IamReady,
    BestMove(ChessMove),
}

fn handle_inputs(
    gui_tx: mpsc::Sender<GuiActionRequest>,
) -> Result<(), mpsc::SendError<GuiActionRequest>> {
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
                gui_tx.send(GuiActionRequest::AreYouReady)?;
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

                gui_tx.send(GuiActionRequest::SetPosition(game.current_position()))?;
            }
            UciMessage::Go {
                time_control,
                search_control,
            } => {
                gui_tx.send(GuiActionRequest::StartSearching(
                    time_control,
                    search_control,
                ))?;
            }
            UciMessage::Stop => {
                gui_tx.send(GuiActionRequest::StopSearching)?;
            }
            UciMessage::Quit => {
                break;
            }
            _ => {}
        };
    }
    Ok(())
}

fn handle_outputs(engine_rx: mpsc::Receiver<EngineResponse>) -> Result<(), mpsc::RecvError> {
    loop {
        let action = engine_rx.recv()?;

        match action {
            EngineResponse::IamReady => {
                println!("readyok");
            }
            EngineResponse::BestMove(bestmove) => {
                println!("bestmove {}", bestmove);
            }
        }
    }
}

fn search(
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

fn engine(
    gui_rx: mpsc::Receiver<GuiActionRequest>,
    engine_tx: mpsc::Sender<EngineResponse>,
) -> Result<(), mpsc::RecvError> {
    let mut board = None;
    let stop = Arc::new(Mutex::new(false));
    let mut search_thread = None;

    // We loop forever, and only break out if the gui_rx channel is terminated.
    loop {
        match gui_rx.recv()? {
            GuiActionRequest::AreYouReady => {
                // Make sure we are ready, then say so.
                engine_tx.send(EngineResponse::IamReady).unwrap();
            }
            GuiActionRequest::SetPosition(b) => {
                board = Some(b);
                // Potentially do other required initialization.
            }
            GuiActionRequest::StartSearching(uci_time_control, uci_search_control) => {
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
                        uci_time_control,
                        uci_search_control,
                        engine_tx,
                        stop,
                    )
                }));
            }
            GuiActionRequest::StopSearching => {
                // Set "stop" variable to true, then join with the search thread.
                // The search thread should ensure to output bestmove
                {
                    let mut stop = stop.lock().unwrap();
                    *stop = true;
                }

                // If a stop was sent without a running search, just ignore the command.
                search_thread.take().map(JoinHandle::join);
            }
        }
    }
}

fn main() {
    let (gui_tx, gui_rx) = mpsc::channel();
    let (engine_tx, engine_rx) = mpsc::channel();

    thread::spawn(move || handle_inputs(gui_tx));
    thread::spawn(move || handle_outputs(engine_rx));

    engine(gui_rx, engine_tx).unwrap_or(());
}
