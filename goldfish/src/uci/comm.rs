use crate::engine::Engine;
use crate::uci::types::UciCommand;
use crate::uci::Uci;

use anyhow::Result;
use chess::Game;
use std::io::BufRead;

impl Default for Uci {
    fn default() -> Self {
        Self {
            game: Game::new(),
            engine: Engine {},
        }
    }
}

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
                UciCommand::UciNewGame => self.game = Game::new(),
                UciCommand::Position(board) => {
                    self.game = Game::new_with_board(board);
                }
                UciCommand::Go(_options) => unimplemented!(),
                UciCommand::Stop => unimplemented!(),
                UciCommand::PonderHit => unimplemented!(),
                UciCommand::Quit => break,
            }
        }

        Ok(())
    }
}
