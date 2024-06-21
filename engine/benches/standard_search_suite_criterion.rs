use criterion::{criterion_group, criterion_main, Criterion};

use engine::Engine;

fn standard_search_suite(c: &mut Criterion) {
    let mut engine = Engine::default();
    c.bench_function("goldfish bench", |b| {
        b.iter(|| engine::bench::run(&mut engine, 6));
    });
}

criterion_group!(benches, standard_search_suite);
criterion_main!(benches);
