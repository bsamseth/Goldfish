use std::sync::{Arc, Mutex};

#[derive(Debug, Clone)]
pub struct StopSignal(Arc<Mutex<bool>>);

impl StopSignal {
    /// Create a new stop signal, which is initially false.
    pub fn new() -> Self {
        Self(Arc::new(Mutex::new(false)))
    }

    /// Set the stop signal.
    pub fn stop(&self) {
        let mut stop = self.0.lock().unwrap();
        *stop = true;
    }

    /// Check if the stop signal has been sent.
    ///
    /// This does _not_ block and returns false immediately if someone else is holding the lock.
    /// This is intentional, as this will be called many times during the search, and we want it
    /// to be as fast as possible.
    pub fn check(&self) -> bool {
        self.0.try_lock().map(|stop| *stop).unwrap_or(false)
    }
}
