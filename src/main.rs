use std::sync::mpsc;
use std::thread;

use goldfish::driver;
use vampirc_uci::UciMessage;

fn main() {
    let (engine_tx, engine_rx) = mpsc::channel();

    // Handle outputing messages to the GUI from the engine.
    // This is delgated to a separate thread to avoid blocking the engine.
    thread::spawn(move || {handle_output(engine_rx)});
    
    driver(engine_tx);
}

fn handle_output(engine_rx: mpsc::Receiver<UciMessage>) -> Result<(), mpsc::RecvError> {
    loop {
        let action = engine_rx.recv()?;
        println!("{action}");
    }
}
