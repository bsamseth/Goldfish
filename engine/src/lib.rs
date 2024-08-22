pub mod bench;
mod board;
mod chessmove;
mod evaluate;
mod limits;
mod logger;
mod movelist;
mod newtypes;
mod opts;
mod search;
mod tt;

use anyhow::Context;
use search::Searcher;
use uci::UciOptions;

/// `Engine` implements the `uci::Engine` trait for the Goldfish engine.
///
/// This is the only public type in the crate, as the only intended way to use the engine crate
/// is to provide the `uci::start` function with an instance of this struct.
///
/// # Examples
/// ```no_run
/// use engine::Engine;
///
/// Engine::default().repl(engine::InputSource::Stdin);
/// ```
#[derive(Debug)]
pub struct Engine {
    transposition_table: tt::TranspositionTable,
    tablebase: Option<&'static fathom::Tablebase>,
    options: opts::Opts,
}

impl Default for Engine {
    fn default() -> Self {
        let options = opts::Opts::default();
        let transposition_table = tt::TranspositionTable::new(1024 * 1024 * options.hash_size_mb);
        Self {
            transposition_table,
            tablebase: None,
            options,
        }
    }
}

pub enum InputSource {
    Stdin,
    Reader(String),
}

impl Engine {
    /// Start the engine's UCI loop.
    pub fn repl(&mut self, input: InputSource) {
        let (interface, do_agg) = match input {
            InputSource::Stdin => (uci::Interface::default(), false),
            InputSource::Reader(reader) => (uci::Interface::from_reader(reader.as_bytes()), true),
        };

        let mut total_nodes = 0;
        let mut total_time = 0;

        let mut position = uci::Position::default();

        // Safety: No other threads are running (with access to the options), so it's safe to
        // store something into the options as nobody else has a reference to it.
        unsafe { opts::OPTS.store(std::ptr::addr_of_mut!(self.options)) };

        while let Ok(cmd) = interface.commands.recv() {
            match cmd {
                uci::Command::Stop => {
                    // The fact that we receieve the stop means we are not searching, so we don't
                    // need to do anything.
                    tracing::debug!("received stop command");
                }
                uci::Command::Quit => break,
                uci::Command::Uci => {
                    println!("id name Goldfish {}", env!("CARGO_PKG_VERSION"));
                    println!("id author {}", env!("CARGO_PKG_AUTHORS"));
                    println!();
                    self.options.print_options();
                    println!("uciok");
                }
                uci::Command::SetOption(uci::EngineOption { name, value }) => {
                    // Safety: No other threads are running (with access to the options), so it's safe
                    // to modify the options.
                    if let Err(e) = self.options.set_option(&name, &value) {
                        tracing::warn!("{e}");
                        continue;
                    }
                    if let Err(e) = self.synchronize_options(&name) {
                        tracing::warn!("{e}");
                        continue;
                    }
                }
                uci::Command::IsReady => println!("readyok"),
                uci::Command::UciNewGame => {}
                uci::Command::Position(pos) => position = pos,
                uci::Command::Go(go_options) => {
                    tracing::info!(
                        "position: {} {}",
                        position.start_pos,
                        position
                            .moves
                            .iter()
                            .map(std::string::ToString::to_string)
                            .collect::<Vec<String>>()
                            .join(" ")
                    );
                    self.transposition_table.new_search();
                    let (bm, logger) = Searcher::new(
                        &position,
                        &go_options,
                        interface.stop.clone(),
                        &mut self.transposition_table,
                        self.tablebase,
                    )
                    .best_move();

                    println!("bestmove {bm}");

                    if do_agg {
                        total_nodes += logger.total_nodes;
                        total_time += logger.search_start_time.elapsed().as_millis();
                    }
                }
                uci::Command::Unknown(e) => tracing::warn!("{e}"),
                uci::Command::PonderHit => todo!(),
                uci::Command::Debug => todo!(),
            }
        }

        if do_agg {
            println!(
                "info string {total_nodes} nodes {} nps {total_time} ms",
                (total_nodes as u128 * 1000) / total_time
            );
        }
    }

    fn synchronize_options(&mut self, option_name: &str) -> anyhow::Result<()> {
        match option_name {
            "Hash" => {
                let value = self.options.hash_size_mb;
                tracing::info!("setting hash size to {} MB", value);
                self.transposition_table.resize(value * 1024 * 1024);
            }
            "SyzygyPath" => {
                let path = self
                    .options
                    .syzygy_path
                    .clone()
                    .expect("should be set at this point");
                tracing::info!("loading tablebase from path: {path:?}");
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
