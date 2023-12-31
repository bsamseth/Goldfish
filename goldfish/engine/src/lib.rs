mod evaluate;
mod movelist;
mod search;
mod stop_signal;
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
}

impl uci::Engine for Engine {
    fn name(&self) -> String {
        format!("Goldfish {}", env!("CARGO_PKG_VERSION"))
    }

    fn author(&self) -> String {
        env!("CARGO_PKG_AUTHORS").to_string()
    }

    fn go(&mut self, game: Game, options: Vec<uci::GoOption>, info_writer: uci::InfoWriter) {
        assert!(
            self.searcher.is_none(),
            "search already in progress, uci protocol violation"
        );

        self.stop_signal = StopSignal::default();
        let ss = self.stop_signal.clone();
        let game = game.clone();
        self.searcher = Some(std::thread::spawn(move || {
            let mut searcher = search::Searcher::new(game, options, info_writer, ss);
            searcher.run();
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
