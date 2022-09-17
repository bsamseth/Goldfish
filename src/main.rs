use std::sync::mpsc;
use std::thread;

use goldfish::driver::engine;
use goldfish::io::{handle_inputs, handle_outputs};

fn main() {
    let (gui_tx, gui_rx) = mpsc::channel();
    let (engine_tx, engine_rx) = mpsc::channel();

    thread::spawn(move || handle_inputs(gui_tx));
    thread::spawn(move || handle_outputs(engine_rx));

    engine(gui_rx, engine_tx).unwrap_or(());
}
