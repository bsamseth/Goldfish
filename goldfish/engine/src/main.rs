use anyhow::Result;

use engine::Engine;

fn main() -> Result<()> {
    let subscriber = tracing_subscriber::FmtSubscriber::builder()
        // _Must_ log to stderr, otherwise UCI protocol will be violated.
        .with_writer(std::io::stderr)
        .finish();
    tracing::subscriber::set_global_default(subscriber)?;

    println!(
        "Goldfish {} - A UCI chess engine",
        env!("CARGO_PKG_VERSION")
    );

    uci::start(Engine::default())?;
    Ok(())
}
