mod evaluate;
mod limits;
mod logger;
mod movelist;
mod search;
mod stop_signal;
mod tt;
mod value;

use chess::Game;

use stop_signal::StopSignal;

/// `Engine` implements the `uci::Engine` trait for the Goldfish engine.
///
/// This is the only public type in the crate, as the only intended way to use the engine crate
/// is to provide the `uci::start` function with an instance of this struct.
///
/// # Examples
/// ```no_run
/// use engine::Engine;
/// use uci::start;
///
/// start(Engine::default()).unwrap();
/// ```
#[derive(Debug, Default)]
pub struct Engine {
    stop_signal: StopSignal,
    searcher: Option<std::thread::JoinHandle<()>>,
    transposition_table: tt::TranspositionTable,
}

impl uci::UciEngine for Engine {
    fn name(&self) -> String {
        format!("Goldfish {}", env!("CARGO_PKG_VERSION"))
    }

    fn author(&self) -> String {
        env!("CARGO_PKG_AUTHORS").to_string()
    }

    fn options(&self) -> Vec<uci::EngineOptionSpesification> {
        vec![uci::EngineOptionSpesification {
            name: "Hash".to_string(),
            option_type: uci::EngineOptionType::Spin,
            default: Some(format!("{DEFAULT_HASH_SIZE_MB}")),
            min: Some(1),
            max: Some(33554432), // Lots of memory, stockfish uses this max 🤷.
            var: None,
        }]
    }

    fn set_option(&mut self, name: &str, value: &str) {
        if let Err(e) = self._set_option(name, value) {
            tracing::error!("failed to set option: {}", e);
        }
    }

    fn go(
        &mut self,
        game: Game,
        options: Vec<uci::GoOption>,
        info_writer: uci::InfoWriter,
        best_move: std::sync::mpsc::Sender<chess::ChessMove>,
    ) {
        if self.searcher.is_some() {
            self.stop();
        }

        self.stop_signal = StopSignal::default();
        let ss = self.stop_signal.clone();
        let game = game.clone();
        self.searcher = Some(std::thread::spawn(move || {
            let mut searcher = search::Searcher::new(game, options, info_writer, ss);
            let bm = searcher.run();
            best_move
                .send(bm)
                .expect("should be able to send best move back to GUI");
        }));
    }

    fn stop(&mut self) {
        assert!(
            self.searcher.is_some(),
            "no search in progress, uci protocol violation"
        );
        self.stop_signal.stop();
        self.searcher.take().unwrap().join().unwrap();
    }
}

impl Engine {
    fn _set_option(&mut self, name: &str, value: &str) -> anyhow::Result<()> {
        match name {
            "Hash" => {
                let value = value.parse::<usize>()?;
                tracing::info!("setting hash size to {} MB", value);
                self.transposition_table.resize(value * MB);
                Ok(())
            }
            _ => anyhow::bail!("invalid option {name}"),
        }
    }
}

const MB: usize = 1024 * 1024;
const DEFAULT_HASH_SIZE_MB: usize = 1;
