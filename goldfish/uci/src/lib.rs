mod types;

use chess::Game;
use std::io::BufRead;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;

use types::UciCommand;

pub use types::{EngineOptionSpesification, EngineOptionType, GoOption};
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

    /// The options supported by the engine.
    ///
    /// This will be sent to the GUI in response to a `uci` command, and
    /// might be used to configure the engine. See `EngineOptionSpesification`.
    ///
    /// By default, this function returns an empty vector, i.e. no supported options.
    fn options(&self) -> Vec<EngineOptionSpesification> {
        vec![]
    }

    /// Set an option to the given value.
    ///
    /// This will be called when the GUI sends a `setoption` command.
    /// The engine should update its internal state to reflect the new value.
    /// If the value is invalid, the engine might log an error, but should otherwise ignore
    /// the command.
    fn set_option(&mut self, name: &str, value: &str);

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
    /// # Important
    /// This function should _not_ block, but should return immediately.
    ///
    /// # Guarantees
    /// The engine will never be sent a `go` command with a game in an invalid state or a game that
    /// already has a result. I.e. the board will be a valid chess position with at least one legal
    /// move.
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
    let searching = Arc::new(AtomicBool::new(false));

    let (info_sender_tx, info_sender_rx) = std::sync::mpsc::channel::<Info>();
    std::thread::spawn(move || {
        while let Ok(info) = info_sender_rx.recv() {
            println!("{info}");
        }
    });

    let (best_move_tx, best_move_rx) = std::sync::mpsc::channel::<chess::ChessMove>();
    let _searching = searching.clone();
    std::thread::spawn(move || {
        while let Ok(best_move) = best_move_rx.recv() {
            _searching.store(false, Ordering::Relaxed);
            println!("bestmove {}", best_move);
        }
    });

    let stdin = std::io::stdin();
    loop {
        let line =
            stdin.lock().lines().next().ok_or_else(|| {
                UciError::Eof("Reached EOF while reading from stdin".to_string())
            })??;

        let command = UciCommand::from(line.as_str());

        if searching.load(Ordering::Relaxed) {
            match command {
                UciCommand::Stop => {
                    engine.stop();
                    searching.store(false, Ordering::Relaxed);
                }
                UciCommand::Quit => {
                    engine.stop();
                    break;
                }
                _ => {
                    tracing::info!("Search in progress, ignoring command: {command:?}");
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

                    println!();
                    for option in engine.options() {
                        println!("{}", option);
                    }
                    println!("uciok");
                }
                UciCommand::Debug => unimplemented!(),
                UciCommand::IsReady => {
                    engine.ready();
                    println!("readyok");
                }
                UciCommand::SetOption(option) => {
                    engine.set_option(&option.name, &option.value);
                }
                UciCommand::UciNewGame => {
                    game = None;
                }
                UciCommand::Position(g) => {
                    game = Some(g);
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
                        searching.store(true, Ordering::Relaxed);
                    } else {
                        tracing::error!("No position set, ignoring go command.");
                    }
                }
                UciCommand::Stop => {
                    tracing::info!("No search in progress, ignoring stop command.");
                }
                UciCommand::PonderHit => unimplemented!(),
                UciCommand::Quit => break,
            }
        }
    }

    Ok(())
}
