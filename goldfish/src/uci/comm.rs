use crate::engine::Engine;
use crate::uci::types::GoOption;
use crate::uci::types::UciCommand;
use crate::uci::StopSignal;
use crate::uci::Uci;

use anyhow::Result;
use chess::Game;
use std::io::BufRead;

fn go(game: Game, options: Vec<GoOption>) -> (StopSignal, impl FnOnce()) {}

impl Uci {
    /// Start the UCI communication loop.
    ///
    /// This function will block until the `quit` command is received, or an error occurs.
    ///
    /// # Errors
    /// This function will return an error if it fails to read from stdin.
    ///
    /// It will _not_ return an error if it encounters an invalid UCI command. In this case the
    /// error message will be logged to stderr, and otherwise ignored.
    pub fn start(&mut self) -> Result<()> {
        let mut game = Game::new();
        let mut engine_thread = None;
        let mut stop_signal = None;

        let stdin = std::io::stdin();
        loop {
            let line = stdin
                .lock()
                .lines()
                .next()
                .ok_or_else(|| anyhow::anyhow!("Failed to read line from stdin"))??;

            let command = UciCommand::from(line.as_str());
            match command {
                UciCommand::Unknown(e) => {
                    tracing::error!("{}", e);
                }
                UciCommand::Uci => println!(
                    "id name Goldfish {}\nid author {}\nuciok",
                    env!("CARGO_PKG_VERSION"),
                    env!("CARGO_PKG_AUTHORS")
                ),
                UciCommand::Debug => println!("Not implemented"),
                UciCommand::IsReady => println!("readyok"),
                UciCommand::SetOption(_option) => unimplemented!(),
                UciCommand::UciNewGame => game = Game::new(),
                UciCommand::Position(board) => {
                    game = Game::new_with_board(board);
                }
                UciCommand::Go(options) => {
                    if engine_thread.is_some() {
                        tracing::error!("Search already in progress, ignoring go command.");
                        continue;
                    }

                    assert!(
                        options
                            .iter()
                            .all(|option| matches!(option, GoOption::Infinite)),
                        "Only infinite search is supported at the moment"
                    );

                    let ss = StopSignal::new();
                    let game = game.clone();
                    stop_signal = Some(ss.clone());
                    engine_thread = Some(std::thread::spawn(move || {
                        Engine::new(game, options, ss).search()
                    }));
                }
                UciCommand::Stop => {
                    if let Some(stop_signal) = stop_signal.take() {
                        stop_signal.stop();
                    } else {
                        // No search in progress, nothing to do.
                        tracing::warn!("Stop command received, but no search in progress.");
                        continue;
                    }

                    engine_thread
                        .take()
                        .expect("we have a stop signal, so we should have a thread")
                        .join()
                        .expect("search thread to exit successfully");
                }
                UciCommand::PonderHit => unimplemented!(),
                UciCommand::Quit => break,
            }
        }

        Ok(())
    }
}

impl Default for Uci {
    fn default() -> Self {
        Self { game: Game::new() }
    }
}
