use crate::commands::GoOption;

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

    /// Print the options supported by the engine.
    ///
    /// This is requested when the GUI expects the engine to say what options it supports.
    ///
    /// By default, this function does nothing, indicating no support for options.
    fn print_options(&self) {}

    /// The options supported by the engine, as a mutable reference.
    ///
    /// This is called when the GUI tries to set an option.
    ///
    /// By default this errors, as no options are supported by default.
    ///
    /// # Errors
    /// If the option name is not recognized, or the value is invalid.
    fn set_option(&mut self, name: &str, value: &str) -> anyhow::Result<()> {
        let _ = (name, value);
        anyhow::bail!("No options are supported by this engine.");
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

    /// Ensure the engine is ready to recieve commands.
    ///
    /// Called when the GUI sends a `isready` command. The protocol will always respond
    /// with `readyok`, but can optionally block until it is ready to proceed.
    ///
    /// By default, this function does nothing, and signals readiness immediately.
    fn ready(&mut self) {}

    /// Start a search.
    ///
    ///
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
        position: UciPosition,
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

#[derive(Debug, Clone, Default)]
pub struct UciPosition {
    pub start_pos: chess::Board,
    pub moves: Vec<chess::ChessMove>,
    pub starting_halfmove_clock: usize,
}
