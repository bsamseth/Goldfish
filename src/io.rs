use std::io;
use std::io::BufRead;
use std::str::FromStr;
use std::sync::mpsc;

use chess::{Board, ChessMove, Game};
use vampirc_uci::{parse_one, UciMessage, UciSearchControl, UciTimeControl};

pub enum GuiActionRequest {
    AreYouReady,
    SetPosition(Board),
    StartSearching(Option<UciTimeControl>, Option<UciSearchControl>),
    StopSearching,
}

pub enum EngineResponse {
    IamReady,
    BestMove(ChessMove),
}

pub fn handle_inputs(
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

pub fn handle_outputs(engine_rx: mpsc::Receiver<EngineResponse>) -> Result<(), mpsc::RecvError> {
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
