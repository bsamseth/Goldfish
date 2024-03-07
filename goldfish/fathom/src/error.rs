#[derive(Debug)]
pub enum Error {
    InvalidPath,
    FailedToInitialize,
    NoFilesFound,
}

impl std::fmt::Display for Error {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{self:?}")
    }
}
impl std::error::Error for Error {}
