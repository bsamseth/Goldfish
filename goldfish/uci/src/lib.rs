mod types;

use chess::Game;
use std::io::BufRead;

use types::GoOption;
use types::Info;
use types::UciCommand;

/// Engines must implement this trait to be compatible with this crate.
pub trait Engine {
    /// The name of the engine, possibly including a version number.
    ///
    /// This will be sent to the GUI in response to a `uci` command, and
    /// might be used to identify the engine in the GUI. Any newlines will
    /// be replaced with spaces.
    fn name(&self) -> &str;

    /// The name of the author of the engine.
    ///
    /// This will be sent to the GUI in response to a `uci` command, and
    /// might be used to identify the engine in the GUI. Any newlines will
    /// be replaced with spaces.
    fn author(&self) -> &str;

    /// Ensure the engine is ready to recieve commands.
    ///
    /// Called when the GUI sends a `isready` command. The protocol will always respond
    /// with `readyok`, but can optionally block until it is ready to proceed.
    ///
    /// By default, this function does nothing, and signals readiness immediately.
    fn ready(&self) {}

    /// Stop the search.
    ///
    /// This will only be called when the previous command was a `go` command,
    /// but could be sent even after the engine has decided end the search on its own.
    /// The engine should stop searching as soon as possible.
    fn stop(&self);

    /// Start a search.
    ///
    /// The game indicates the current state of the board, including any moves leading up to it.
    /// The options indicate any search options that were specified by the GUI.
    /// The info writer can be used to send information about the search back to the GUI.
    ///
    /// This function should _not_ block, but should return immediately.
    fn go(&self, game: &Game, options: Vec<GoOption>, info_writer: InfoWriter);
}

pub struct InfoWriter {
    sender: std::sync::mpsc::Sender<Info>,
}

impl InfoWriter {
    /// Send an `InfoPart` to the GUI.
    ///
    /// Any parts that can be joined together to form a complete `Info` will be joined
    /// by the writer. The engine should simply send each part as it is generated.
    pub fn send_info(&self, info: Info) {
        self.sender.send(info).unwrap();
    }
}

#[derive(thiserror::Error, Debug)]
pub enum UciError {
    #[error("EOF while reading from stdin")]
    Eof(String),
    #[error("Failed to read line from stdin")]
    ReadLine(#[from] std::io::Error),
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
pub fn start(engine: impl Engine) -> Result<(), UciError> {
    let mut game = Game::new();
    let mut searching = false;

    let (tx, rx) = std::sync::mpsc::channel::<Info>();
    std::thread::spawn(move || {
        while let Ok(info) = rx.recv() {
            println!("{}", info);
        }
    });

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
                    let name = engine.name().replace('\n', " ");
                    let author = engine.author().replace('\n', " ");
                    println!("id name {name}");
                    println!("id author {author}");
                    println!("uciok");
                }
                UciCommand::Debug => unimplemented!(),
                UciCommand::IsReady => {
                    engine.ready();
                    println!("readyok");
                }
                UciCommand::SetOption(_option) => unimplemented!(),
                UciCommand::UciNewGame => {
                    // Nothing to do.
                }
                UciCommand::Position(board) => {
                    game = Game::new_with_board(board);
                }
                UciCommand::Go(options) => {
                    let info_writer = InfoWriter { sender: tx.clone() };
                    engine.go(&game, options, info_writer);
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
