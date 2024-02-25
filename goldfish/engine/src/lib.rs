pub mod bench;
mod board;
mod evaluate;
mod limits;
mod logger;
mod movelist;
mod newtypes;
mod search;
mod stop_signal;
mod tablebase;
mod tt;

use std::sync::{Arc, RwLock};

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
    transposition_table: Arc<RwLock<tt::TranspositionTable>>,
    tablebase: Option<Arc<tablebase::Tablebase>>,
}

impl uci::UciEngine for Engine {
    fn name(&self) -> String {
        format!("Goldfish {}", env!("CARGO_PKG_VERSION"))
    }

    fn author(&self) -> String {
        env!("CARGO_PKG_AUTHORS").to_string()
    }

    fn options(&self) -> Vec<uci::EngineOptionSpesification> {
        vec![
            uci::EngineOptionSpesification {
                name: "Hash".to_string(),
                option_type: uci::EngineOptionType::Spin,
                default: Some(format!("{DEFAULT_HASH_SIZE_MB}")),
                min: Some(1),
                max: Some(33_554_432), // Lots of memory, stockfish uses this max 🤷.
                var: None,
            },
            uci::EngineOptionSpesification {
                name: "SyzygyPath".to_string(),
                option_type: uci::EngineOptionType::String,
                default: None,
                min: None,
                max: None,
                var: None,
            },
        ]
    }

    fn set_option(&mut self, name: &str, value: &str) {
        if let Err(e) = self._set_option(name, value) {
            tracing::error!("failed to set option: {}", e);
        }
    }

    fn go(
        &mut self,
        position: uci::UciPosition,
        options: Vec<uci::GoOption>,
        best_move: std::sync::mpsc::Sender<chess::ChessMove>,
    ) {
        if self.searcher.is_some() {
            self.stop();
        }

        self.stop_signal = StopSignal::default();
        self.searcher = Some(std::thread::spawn({
            let ss = self.stop_signal.clone();
            let tt = self.transposition_table.clone();
            let tb = self.tablebase.clone();
            move || {
                let mut searcher = search::Searcher::new(position, &options, ss, tt, tb);
                let bm = searcher.run();
                best_move
                    .send(bm)
                    .expect("should be able to send best move back to GUI");
            }
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
                self.transposition_table.write().unwrap().resize(value * MB);
                Ok(())
            }
            "SyzygyPath" => {
                let path = std::path::PathBuf::from(value);
                self.tablebase = Some(Arc::new(tablebase::Tablebase::new(path)?));
                Ok(())
            }
            _ => Err(anyhow::anyhow!("invalid option {name}")),
        }
    }
}

const MB: usize = 1024 * 1024;
const DEFAULT_HASH_SIZE_MB: usize = 16;
