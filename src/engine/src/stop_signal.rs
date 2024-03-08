#[derive(Debug, Clone, Default)]
pub struct StopSignal(std::sync::Arc<std::sync::Mutex<bool>>);

impl StopSignal {
    pub fn stop(&self) {
        *self
            .0
            .lock()
            .expect("should be able to obtain lock on stop signal") = true;
    }

    pub fn check(&self) -> bool {
        self.0.try_lock().map_or(false, |guard| *guard)
    }
}
