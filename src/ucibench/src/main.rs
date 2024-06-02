#![allow(dead_code)]
#![allow(
    clippy::cast_possible_truncation,
    clippy::cast_sign_loss,
    clippy::cast_precision_loss
)]
use std::{
    io::{Read, Write},
    time::Duration,
};

use clap::Parser;
use color_eyre::{eyre::ContextCompat, Result};
use itertools::Itertools;
use serde::{Deserialize, Serialize};

fn main() -> Result<()> {
    let args: Args = Args::parse();

    // Read the bench file:
    let bench: BenchSuite = serde_json::from_reader(std::fs::File::open(&args.bench_file)?)?;

    // Run the bench:
    let results = bench.run(&args.engine, args.runs)?;

    // Summarize the results:
    let total_nodes: u64 = results.iter().map(|r| r.nodes).sum();
    let total_time = results.iter().map(|r| r.time).sum::<Duration>();
    let total_nps = total_nodes as f64 / total_time.as_secs_f64();
    let summary = BenchSummary {
        engine: args.engine,
        nodes: total_nodes,
        time: total_time.as_secs_f64(),
        nps: total_nps.round() as u64,
    };

    if args.json {
        serde_json::to_writer(std::io::stdout(), &summary)?;
        println!();
    } else {
        println!(
            "Nodes: {}\nNPS: {}\nTime: {:?}",
            thousands_sep(summary.nodes),
            thousands_sep(summary.nps),
            summary.time,
        );
    }

    Ok(())
}

#[derive(Parser)]
struct Args {
    #[arg(help = "The bench definition file")]
    bench_file: String,
    #[arg(help = "The UCI engine to bench")]
    engine: String,
    #[arg(
        short,
        long,
        default_value_t = 5,
        help = "The number of runs per case to use"
    )]
    runs: usize,
    #[arg(short, long, default_value_t = false, help = "Output as JSON")]
    json: bool,
}

#[derive(Debug, Deserialize)]
struct BenchSuite {
    depth: u8,
    uci_options: Option<Vec<UciOption>>,
    cases: Vec<BenchCase>,
}

impl BenchSuite {
    fn run(&self, engine: &str, runs: usize) -> Result<Vec<BenchResult>> {
        self.cases
            .iter()
            .flat_map(|case| std::iter::repeat(case).take(runs))
            .map(|case| self.run_case(case, engine))
            .inspect(|result| match result {
                Err(err) => eprintln!("Error: {err}"),
                Ok(_) => {}
            })
            .collect()
    }

    fn run_case(&self, case: &BenchCase, engine: &str) -> Result<BenchResult> {
        let start = std::time::Instant::now();
        let mut process = std::process::Command::new(engine)
            .stdin(std::process::Stdio::piped())
            .stdout(std::process::Stdio::piped())
            .spawn()?;

        let input = process
            .stdin
            .as_mut()
            .context("Writing to subprocess stdin")?;

        writeln!(input, "uci")?;
        if let Some(options) = &self.uci_options {
            for option in options {
                writeln!(
                    input,
                    "setoption name {} value {}",
                    option.name, option.value
                )?;
            }
        }
        write!(input, "position fen {}\ngo depth {}\n", case, self.depth)?;

        let output = process
            .stdout
            .as_mut()
            .context("Reading from subprocess stdout")?;

        let output = wait_for_bestmove(output)?;
        writeln!(input, "quit")?;
        process.wait()?;

        let info = output
            .lines()
            .rev()
            .find(|line| line.contains("pv"))
            .with_context(
                || format!("Looking for PV info line in process output for case {case}",),
            )?
            .split_whitespace()
            .skip(1)
            .tuple_windows()
            .collect::<Vec<(_, _)>>();

        let nodes = info
            .iter()
            .find(|(k, _)| k == &"nodes")
            .with_context(|| format!("Looking for nodes in PV info: {info:?}"))?
            .1
            .parse()?;

        Ok(BenchResult {
            case: case.clone(),
            nodes,
            time: std::time::Instant::now().duration_since(start),
        })
    }
}

fn wait_for_bestmove<R: Read>(reader: &mut R) -> Result<String> {
    let mut output = String::new();
    loop {
        let mut buf = [0; 1024];
        let n = reader.read(&mut buf)?;
        if n == 0 {
            continue;
        }

        let buf = std::str::from_utf8(&buf[..n])?;
        output.push_str(buf);

        if buf.contains("bestmove") {
            break;
        }
    }
    Ok(output)
}

type BenchCase = String;

#[derive(Debug, Deserialize)]
struct UciOption {
    name: String,
    value: String,
}

#[derive(Debug, Clone, Serialize)]
struct BenchResult {
    case: BenchCase,
    nodes: u64,
    time: Duration,
}

#[derive(Debug, Serialize)]
struct BenchSummary {
    engine: String,
    nodes: u64,
    time: f64,
    nps: u64,
}

impl BenchResult {
    fn nps(&self) -> u64 {
        (self.nodes as f64 / self.time.as_secs_f64()).round() as u64
    }
}

fn thousands_sep(n: u64) -> String {
    n.to_string()
        .as_bytes()
        .rchunks(3)
        .rev()
        .map(std::str::from_utf8)
        .collect::<Result<Vec<&str>, _>>()
        .unwrap()
        .join(",")
}
