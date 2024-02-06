use std::io::BufRead;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;

use crate::commands::{GoOption, UciCommand};
use crate::error::Error;
use crate::uciengine::UciEngine;
use crate::UciPosition;

/// Start the UCI communication loop.
///
/// This function will block until the `quit` command is received, or an error occurs.
///
/// # Important
/// This crate uses `tracing` for logging. If you enable logging, e.g. using `tracing_subscriber`,
/// you **must** configure it to log to stderr. Otherwise, the UCI protocol will be violated, as
/// communication is done over stdin/stdout.
///
/// # Note
/// For convenience during debug, this UCI implementation will act as if a `position startpos` has
/// been given already. That is, it will accept a `go` command without a `position` command
/// first.
///
/// # Errors
/// This function will return an error if it fails to read from stdin.
///
/// It will _not_ return an error if it encounters an invalid UCI command. In this case the
/// error message will be logged to stderr, and otherwise ignored.
pub fn start(mut engine: impl UciEngine) -> Result<(), Error> {
    let mut position = UciPosition::default();
    let searching = Arc::new(AtomicBool::new(false));

    let (best_move_tx, best_move_rx) = std::sync::mpsc::channel::<chess::ChessMove>();
    let searching_clone = searching.clone();
    std::thread::spawn(move || {
        while let Ok(best_move) = best_move_rx.recv() {
            searching_clone.store(false, Ordering::Relaxed);
            println!("bestmove {best_move}");
        }
    });

    let stdin = std::io::stdin();
    loop {
        let line = stdin
            .lock()
            .lines()
            .next()
            .ok_or_else(|| Error::Eof("Reached EOF while reading from stdin".to_string()))??;

        let command = UciCommand::from(line.as_str());

        if searching.load(Ordering::Relaxed) {
            match command {
                UciCommand::Stop => {
                    engine.stop();
                    searching.store(false, Ordering::Relaxed);
                }
                UciCommand::Quit => {
                    engine.stop();
                    break;
                }
                _ => {
                    tracing::info!("Search in progress, ignoring command: {command:?}");
                }
            }
        } else {
            match command {
                UciCommand::Unknown(e) => {
                    tracing::error!("{}", e);
                }
                UciCommand::Uci => {
                    println!("id name {}", engine.name().replace('\n', " "));
                    println!("id author {}", engine.author().replace('\n', " "));

                    println!();
                    for option in engine.options() {
                        println!("{option}");
                    }
                    println!("uciok");
                }
                UciCommand::Debug => unimplemented!(),
                UciCommand::IsReady => {
                    engine.ready();
                    println!("readyok");
                }
                UciCommand::SetOption(option) => {
                    engine.set_option(&option.name, &option.value);
                }
                UciCommand::UciNewGame => {
                    engine.ucinewgame();
                }
                UciCommand::Position(pos) => {
                    position = pos;
                }
                UciCommand::Go(options) => {
                    let options = if options.is_empty() {
                        vec![GoOption::Infinite]
                    } else {
                        options
                    };
                    engine.go(position.clone(), options, best_move_tx.clone());
                    searching.store(true, Ordering::Relaxed);
                }
                UciCommand::Stop => {
                    tracing::info!("No search in progress, ignoring stop command.");
                }
                UciCommand::PonderHit => unimplemented!(),
                UciCommand::Quit => break,
            }
        }
    }

    Ok(())
}
