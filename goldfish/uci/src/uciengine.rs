use chess::Game;

use crate::commands::GoOption;
use crate::responses::EngineOptionSpesification;

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
    /// might be used to configure the engine. See [`EngineOptionSpesification`].
    ///
    /// By default, this function returns an empty vector, i.e. no supported options.
    fn options(&self) -> Vec<EngineOptionSpesification> {
        vec![]
    }

    /// Recieve a `ucinewgame` command.
    ///
    /// This is sent to the engine when the next search (started with "position" and "go")
    /// will be from a different game. Engines _could_ clear transposition tables and such,
    /// but the engine is free to do anything here. This function should not block. The GUI
    /// will send a `isready` command after this, so the engine could do background processing
    /// if needed.
    ///
    /// By default, this function does nothing.
    fn ucinewgame(&mut self) {}

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
