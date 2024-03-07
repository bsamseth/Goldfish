use engine::bench::{Bench, BENCH_CASES};
use engine::Engine;

fn standard_search_suite() {
    for fen in &BENCH_CASES {
        let with_prefix = format!("fen {fen}");
        let mut engine = Engine::default();
        engine.set_bench_options();
        engine.bench(&with_prefix, 6);
    }
}

iai::main!(standard_search_suite);
