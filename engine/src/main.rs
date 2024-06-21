use anyhow::Result;

use engine::Engine;

use clap::{Parser, Subcommand};

#[derive(Parser)]
#[command(version, about, long_about = None)]
#[command(propagate_version = true)]
struct Cli {
    #[command(subcommand)]
    command: Option<Commands>,
}

#[derive(Subcommand)]
enum Commands {
    Bench,
    Uci,
}

fn main() -> Result<()> {
    let args = Cli::parse();
    let subscriber = tracing_subscriber::FmtSubscriber::builder()
        // _Must_ log to stderr, otherwise UCI protocol will be violated.
        .with_writer(std::io::stderr)
        .with_max_level(tracing::Level::INFO)
        .finish();
    tracing::subscriber::set_global_default(subscriber)?;

    println!(
        "Goldfish {} - A UCI chess engine",
        env!("CARGO_PKG_VERSION")
    );

    let mut engine = Engine::default();

    match args.command.unwrap_or(Commands::Uci) {
        Commands::Bench => engine::bench::run(&mut engine, 6),
        Commands::Uci => {
            engine.repl(engine::InputSource::Stdin);
        }
    }

    Ok(())
}
