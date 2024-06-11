use anyhow::Result;

use engine::Engine;

fn main() -> Result<()> {
    let subscriber = tracing_subscriber::FmtSubscriber::builder()
        // _Must_ log to stderr, otherwise UCI protocol will be violated.
        .with_writer(std::io::stderr)
        .with_env_filter(tracing_subscriber::EnvFilter::from_default_env())
        .finish();
    tracing::subscriber::set_global_default(subscriber)?;

    println!(
        "Goldfish {} - A UCI chess engine",
        env!("CARGO_PKG_VERSION")
    );

    Engine::default().repl();
    Ok(())
}
