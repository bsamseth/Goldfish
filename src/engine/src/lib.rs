pub mod bench;
mod board;
mod evaluate;
mod limits;
mod logger;
mod movelist;
mod newtypes;
mod opts;
mod search;
mod stop_signal;
mod tt;
mod tune;

use std::sync::{Arc, RwLock};

use anyhow::Context;
use stop_signal::StopSignal;
use uci::UciOptions;

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
    tablebase: Option<&'static fathom::Tablebase>,
    options: opts::Opts,
}

impl uci::UciEngine for Engine {
    fn name(&self) -> String {
        format!("Goldfish {}", env!("CARGO_PKG_VERSION"))
    }

    fn author(&self) -> String {
        env!("CARGO_PKG_AUTHORS").to_string()
    }

    fn print_options(&self) {
        self.options.print_options();
    }

    fn set_option(&mut self, name: &str, value: &str) -> anyhow::Result<()> {
        self.options.set_option(name, value)?;
        self.synchronize_options(name)
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
            let tb = self.tablebase;
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
        self.stop_signal.stop();
        if let Some(s) = self.searcher.take() {
            s.join().unwrap();
        }
    }
}

impl Engine {
    fn synchronize_options(&mut self, option_name: &str) -> anyhow::Result<()> {
        // The UCI protocol states that options are only set when then
        // engine is waiting.
        assert!(self.searcher.is_none());

        match option_name {
            "Hash" => {
                let value = self.options.hash_size_mb;
                tracing::info!("setting hash size to {} MB", value);
                self.transposition_table.write().unwrap().resize(value * MB);
            }
            "SyzygyPath" => {
                let path = self
                    .options
                    .syzygy_path
                    .clone()
                    .expect("should be set at this point");
                self.tablebase = Some({
                    // Safety: There's no ongoing search, and only one option can be set any given
                    // time. This means nobody else is potentially loading/probing, so this is safe.
                    let loaded = unsafe { fathom::Tablebase::load(path) }?;
                    // Safety: The tablebase is loaded, so it's safe to dereference because no
                    // other thread can change the pointer.
                    let loaded_ref = unsafe { loaded.as_ref() };
                    loaded_ref.context("derefrencing tablebase pointer after load")?
                });
            }
            _ => {}
        };
        Ok(())
    }
}

const MB: usize = 1024 * 1024;
const DEFAULT_HASH_SIZE_MB: usize = 16;
