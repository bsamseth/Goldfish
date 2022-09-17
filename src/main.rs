use rand::seq::IteratorRandom;
use std::{io, thread, error};
use std::io::BufRead;
use std::str::FromStr;
use std::sync::mpsc;

use chess::{Board, Game, MoveGen, ChessMove};
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
    return Ok(())
}

fn handle_outputs(
    engine_rx: mpsc::Receiver<EngineResponse>,
) -> Result<(), mpsc::RecvError> {
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

fn engine(gui_rx: mpsc::Receiver<GuiActionRequest>, engine_tx: mpsc::Sender<EngineResponse>) -> std::result::Result<(), Box<dyn error::Error>> {
    let mut rng = rand::thread_rng();
    let mut board = None;
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
            GuiActionRequest::StartSearching(_, _) => {
               // Search! 
               // Dispatch a search thread that has a shared reference to engine_tx and a mutex protected "stop" variable.
            }
            GuiActionRequest::StopSearching => {
                // Set "stop" variable to true, then join with the search thread.
                // The search thread should ensure to output bestmove


                // Placeholder: 
                let bestmove = MoveGen::new_legal(&board.expect("Stop without running search!"))
                    .into_iter()
                    .choose(&mut rng)
                    .expect("No legal moves in position!");

                // Failing to send is a crash.
                engine_tx.send(EngineResponse::BestMove(bestmove)).unwrap();
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

