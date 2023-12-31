mod evaluate;
mod movelist;
mod search;
mod stop_signal;
mod value;

use chess::Game;

use stop_signal::StopSignal;

/// `Engine` implements the `uci::Engine` trait for the Goldfish engine.
///
/// The struct maintains a handle to a search thread, which does the actual work, along
/// with a `StopSignal` which is used to signal the search thread to stop. This is needed as
/// the search needs to happen in a separate thread to the UCI controll loop, but the thread also
/// needs to be held somewhere so as to not drop it.
///
/// The search thread manages by the `Engine` will be given a reference to the `StopSignal`, and
/// promises to check it regularly, stopping the search if it is set.
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
            searcher.start();
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
