/*!
A crate to help implementing UCI chess engines.

It defines types and scaffolding for engines to implement the [UCI protocol][UCI-protocol].
This means that instead of parsing UCI commands yourself form the standard input, you can instead
listen to a mpsc receiver and match over the commands.

# Example

This shows the outline of a main program loop for a UCI chess engine.
```
use uci::UciOptions;

#[derive(Debug, UciOptions)]
struct MyOptions {
    #[uci(name = "Hash", kind = "spin", default = "16", min = "1", max = "128")]
    pub hash_size_mb: usize,
    #[uci(name = "SyzygyPath", kind = "string")]
    pub syzygy_path: Option<std::path::PathBuf>,
}

let interface = uci::Interface::default();
let mut position = uci::Position::default();
let mut options = MyOptions::default();

while let Ok(cmd) = interface.commands.recv() {
    match cmd {
        uci::Command::Uci => {
            println!("id name Example {}", env!("CARGO_PKG_VERSION"));
            println!("id author {}", env!("CARGO_PKG_AUTHORS"));
            println!();
            options.print_options();
            println!("uciok");
        }
        uci::Command::SetOption(uci::EngineOption { name, value }) => {
            if let Err(e) = options.set_option(&name, &value) {
                eprintln!("{e}");
                continue;
            }
        }
        uci::Command::IsReady => println!("readyok"),
        uci::Command::UciNewGame => {},
        uci::Command::Position(pos) => position = pos,
        uci::Command::Go(go_options) => {
            // Start searching for the best move.
            // This can run the the main thread, as the UCI parser is running in a separate thread.
            // If it encounters a `quit` or `stop` command, it will set the `stop` atomic bool,
            // which the search function should check regularly.

            // let bm = search(&position, &go_options, &interface.stop);
            // println!("bestmove {bm}");
        }
        uci::Command::Stop => todo!(),
        uci::Command::Quit => todo!(),
        uci::Command::PonderHit => todo!(),
        uci::Command::Debug => todo!(),
        uci::Command::Unknown(e) => eprintln!("{e}"),
    }
}
```
[UCI-protocol]: https://www.wbec-ridderkerk.nl/html/UCIProtocol.html
*/

mod commands;
mod opts;
mod responses;

use anyhow::{Context, Result};
use std::io::BufRead;

pub use commands::{Command, EngineOption, GoOption, Position};
pub use opts::UciOptions;
pub use responses::{Info, InfoPart};

pub struct Interface {
    pub commands: std::sync::mpsc::Receiver<commands::Command>,
    pub stop: std::sync::Arc<std::sync::atomic::AtomicBool>,
}

impl Interface {
    /// Create a new [`Interface`] reading from stdin.
    #[must_use]
    pub fn new() -> Self {
        let (commands_sender, commands) = std::sync::mpsc::channel();
        let stop = std::sync::Arc::new(std::sync::atomic::AtomicBool::new(false));

        std::thread::spawn({
            let stop = stop.clone();
            move || {
                if let Err(e) = parse_commands_from_stdin(&commands_sender, &stop) {
                    tracing::error!("UCI command parser exited with error: {e}");
                }
            }
        });

        Self { commands, stop }
    }

    pub fn from_reader(reader: impl BufRead) -> Self {
        let (commands_sender, commands) = std::sync::mpsc::channel();
        let stop = std::sync::Arc::new(std::sync::atomic::AtomicBool::new(false));

        for line in reader.lines().map_while(Result::ok) {
            if let Err(e) = parse_command(&line, &commands_sender, &stop) {
                tracing::error!("Error parsing UCI command: {e}");
            }
        }

        Self {
            commands,
            stop: std::sync::Arc::new(std::sync::atomic::AtomicBool::new(false)),
        }
    }
}

impl Default for Interface {
    fn default() -> Self {
        Interface::new()
    }
}
/// Send parsed commands from the user until a `quit` command is received.
///
/// If a `stop` or `quit` command is received, the `stop` atomic bool is set to true.
/// The command is always sent to the engine's mpsc receiver.
///
/// # Errors
/// This function will return an error if it fails to read from stdin, or if it fails to send a
/// command to the engine's mpsc receiver.
///
/// It will _not_ return an error if it encounters an invalid UCI command. Instead a
/// [`uci::Command::Unknown`] is sent.
fn parse_commands_from_stdin(
    command_sender: &std::sync::mpsc::Sender<Command>,
    stop: &std::sync::atomic::AtomicBool,
) -> Result<()> {
    let mut stdin = std::io::stdin().lock().lines();
    loop {
        let line = stdin
            .next()
            .context("Reached EOF while reading from stdin")??;

        parse_command(&line, command_sender, stop)?;
    }
}
fn parse_command(
    line: &str,
    command_sender: &std::sync::mpsc::Sender<Command>,
    stop: &std::sync::atomic::AtomicBool,
) -> Result<()> {
    if line.is_empty() {
        return Ok(());
    }

    match Command::from(line) {
        Command::Stop => {
            stop.store(true, std::sync::atomic::Ordering::Relaxed);
            command_sender.send(Command::Stop)?;
        }
        Command::Quit => {
            stop.store(true, std::sync::atomic::Ordering::Relaxed);
            command_sender.send(Command::Quit)?;
        }
        cmd => command_sender.send(cmd)?,
    }
    Ok(())
}
