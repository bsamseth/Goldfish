use engine::Engine;

fn standard_search_suite() {
    let mut engine = Engine::default();
    engine::bench::run(&mut engine, 6);
}

iai::main!(standard_search_suite);
