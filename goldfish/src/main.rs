use anyhow::Result;
use goldfish::Uci;

fn main() -> Result<()> {
    let subscriber = tracing_subscriber::FmtSubscriber::builder()
        .with_writer(std::io::stderr)
        .finish();
    tracing::subscriber::set_global_default(subscriber)?;

    println!(
        "Goldfish {} - A UCI chess engine",
        env!("CARGO_PKG_VERSION")
    );

    let mut uci = Uci::default();
    uci.start()
}
