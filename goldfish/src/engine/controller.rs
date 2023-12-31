use std::sync::Mutex;

#[derive(Debug, Default)]
pub struct Controller {
    stop: Mutex<bool>,
}

impl Controller {
    pub fn new() -> Self {
        Self {
            stop: Mutex::new(false),
        }
    }

    pub fn stop(&self) {
        let mut stop = self.stop.lock().unwrap();
        *stop = true;
    }

    pub fn should_stop(&self) -> bool {
        *self.stop.lock().unwrap()
    }
}
