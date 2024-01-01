mod types;

use chess::Game;
use std::io::BufRead;

pub use types::GoOption;
use types::UciCommand;
pub use types::{Info, InfoPart};

/// Engines must implement this trait to be compatible with this crate.
pub trait UciEngine {
    /// The name of the engine, possibly including a version number.
    ///
    /// This will be sent to the GUI in response to a `uci` command, and
    /// might be used to identify the engine in the GUI. Any newlines will
    /// be replaced with spaces.
    fn name(&self) -> String;

    /// The name of the author of the engine.
    ///
    /// This will be sent to the GUI in response to a `uci` command, and
    /// might be used to identify the engine in the GUI. Any newlines will
    /// be replaced with spaces.
    fn author(&self) -> String;

    /// Ensure the engine is ready to recieve commands.
    ///
    /// Called when the GUI sends a `isready` command. The protocol will always respond
    /// with `readyok`, but can optionally block until it is ready to proceed.
    ///
    /// By default, this function does nothing, and signals readiness immediately.
    fn ready(&mut self) {}

    /// Start a search.
    ///
    /// The game contains the current state of the board, including any moves leading up to it.
    /// The options indicate any search options that were specified by the GUI.
    /// The info writer can be used to send information about the search back to the GUI.
    /// When the search stops, a move _must_ be sent to the GUI using the `best_move` sender.
    ///
    /// This function should _not_ block, but should return immediately.
    fn go(
        &mut self,
        game: Game,
        options: Vec<GoOption>,
        info_writer: InfoWriter,
        best_move: std::sync::mpsc::Sender<chess::ChessMove>,
    );

    /// Stop the search.
    ///
    /// This will only be called when the previous command was a `go` command,
    /// but could be sent even after the engine has decided end the search on its own.
    /// The engine should stop searching as soon as possible.
    ///
    /// When the search has been stopped, either by itself or after a stop signal,
    /// the engine should send the best move to the GUI, using the `best_move` sender.
    fn stop(&mut self);
}

#[derive(Debug)]
pub struct InfoWriter {
    sender: std::sync::mpsc::Sender<Info>,
}

impl InfoWriter {
    /// Send an `Info` to the GUI.
    ///
    /// # Panics
    /// This function will panic if the UCI communication loop has exited. But if this
    /// happens, all threads will be dropped anyway, meaning no one would be able to trigger
    /// this function. So it's safe to assume that this function will never panic.
    pub fn send_info(&self, info: Info) {
        self.sender.send(info).expect("should be able to send info");
    }
}

#[derive(thiserror::Error, Debug)]
pub enum UciError {
    #[error("EOF while reading from stdin")]
    Eof(String),
    #[error("Failed to read line from stdin")]
    ReadLine(#[from] std::io::Error),
    #[error("Engine did not send best move")]
    NoBestMove(#[from] std::sync::mpsc::RecvError),
}

/// Start the UCI communication loop.
///
/// This function will block until the `quit` command is received, or an error occurs.
///
/// # Important
/// This crate uses `tracing` for logging. If you enable logging, e.g. using `tracing_subscriber`,
/// you **must** configure it to log to stderr. Otherwise, the UCI protocol will be violated, as
/// communication is done over stdin/stdout.
///
/// # Errors
/// This function will return an error if it fails to read from stdin.
///
/// It will _not_ return an error if it encounters an invalid UCI command. In this case the
/// error message will be logged to stderr, and otherwise ignored.
pub fn start(mut engine: impl UciEngine) -> Result<(), UciError> {
    let mut game = None;
    let mut searching = false;

    let (info_sender_tx, info_sender_rx) = std::sync::mpsc::channel::<Info>();
    std::thread::spawn(move || {
        while let Ok(info) = info_sender_rx.recv() {
            println!("{info}");
        }
    });

    let (best_move_tx, best_move_rx) = std::sync::mpsc::channel::<chess::ChessMove>();

    let stdin = std::io::stdin();
    loop {
        let line =
            stdin.lock().lines().next().ok_or_else(|| {
                UciError::Eof("Reached EOF while reading from stdin".to_string())
            })??;

        let command = UciCommand::from(line.as_str());

        if searching {
            match command {
                UciCommand::Stop => {
                    engine.stop();
                    let best_move = best_move_rx.recv()?;
                    println!("bestmove {}", best_move);
                    searching = false;
                }
                UciCommand::Quit => {
                    engine.stop();
                    break;
                }
                _ => {
                    tracing::error!("Search in progress, ignoring command: {command:?}");
                }
            }
        } else {
            match command {
                UciCommand::Unknown(e) => {
                    tracing::error!("{}", e);
                }
                UciCommand::Uci => {
                    println!("id name {}", engine.name().replace('\n', " "));
                    println!("id author {}", engine.author().replace('\n', " "));
                    println!("uciok");
                }
                UciCommand::Debug => unimplemented!(),
                UciCommand::IsReady => {
                    engine.ready();
                    println!("readyok");
                }
                UciCommand::SetOption(_option) => unimplemented!(),
                UciCommand::UciNewGame => {
                    game = None;
                }
                UciCommand::Position(board) => {
                    game = Some(Game::new_with_board(board));
                }
                UciCommand::Go(options) => {
                    if let Some(game) = &game {
                        let options = if options.is_empty() {
                            vec![GoOption::Infinite]
                        } else {
                            options
                        };
                        let info_writer = InfoWriter {
                            sender: info_sender_tx.clone(),
                        };
                        engine.go(game.clone(), options, info_writer, best_move_tx.clone());
                        searching = true;
                    } else {
                        tracing::error!("No position set, ignoring go command.");
                    }
                }
                UciCommand::Stop => {
                    tracing::error!("No search in progress, ignoring stop command.");
                }
                UciCommand::PonderHit => unimplemented!(),
                UciCommand::Quit => break,
            }
        }
    }

    Ok(())
}
