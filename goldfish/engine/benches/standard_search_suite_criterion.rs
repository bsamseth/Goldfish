use criterion::{criterion_group, criterion_main, Criterion};

use engine::bench::{Bench, BENCH_CASES};
use engine::Engine;

fn standard_search_suite(c: &mut Criterion) {
    for fen in &BENCH_CASES {
        let with_prefix = format!("fen {fen}");
        let mut engine = Engine::default();
        c.bench_function(&format!("Case: {fen}"), |b| {
            b.iter(|| engine.bench(&with_prefix, 6));
        });
    }
}

criterion_group!(benches, standard_search_suite);
criterion_main!(benches);
