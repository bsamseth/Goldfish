#[derive(thiserror::Error, Debug)]
pub enum Error {
    #[error("EOF while reading from stdin")]
    Eof(String),
    #[error("Failed to read line from stdin")]
    ReadLine(#[from] std::io::Error),
    #[error("Engine did not send best move")]
    NoBestMove(#[from] std::sync::mpsc::RecvError),
}
