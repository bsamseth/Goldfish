/*!
A crate for implementing UCI chess engines.

Defines types and scaffolding for engines to implement the [UCI protocol][uci-protocol].

# Example

This example shows how the main function could be set up, provided you have a struct that implements the [`UciEngine`] trait.
You simply call [`start`] with an instance of your engine, and the UCI protocol will be handled for you.

```rust
use anyhow::Result;

struct MyEngine;
impl uci::UciEngine for MyEngine {
    ...
}

fn main() -> Result<()> {
    // If you want to enable logging, you must configure it to log to stderr.
    // Otherwise the logs will be read as if they were UCI responses.
    let subscriber = tracing_subscriber::FmtSubscriber::builder()
        .with_writer(std::io::stderr)
        .finish();
    tracing::subscriber::set_global_default(subscriber)?;

    // UCI engines aren't required to do anything before they are asked to,
    // but it is common to print a greeting when started.
    println!(
        "Example engine, version {}, by {}",
        env!("CARGO_PKG_VERSION"),
        env!("CARGO_PKG_AUTHORS"),
    );

    uci::start(MyEngine{})?;
    Ok(())
}
```

[uci-protocol]: https://www.wbec-ridderkerk.nl/html/UCIProtocol.html
*/

mod comm;
mod commands;
mod error;
mod responses;
mod uciengine;

pub use comm::start;
pub use commands::GoOption;
pub use responses::{EngineOptionSpesification, EngineOptionType, Info, InfoPart};
pub use uciengine::{UciEngine, UciPosition};
