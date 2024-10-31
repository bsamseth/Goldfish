use thiserror::Error;

/// For when things don't go as planned.
#[derive(Debug, Error)]
pub enum Error {
    #[error("Invalid square index: {0}")]
    InvalidSquare(u8),
    #[error("Invalid file index: {0}")]
    InvalidFile(u8),
    #[error("Invalid file notation: {0}")]
    InvalidFileChar(char),
    #[error("Invalid rank index: {0}")]
    InvalidRank(u8),
    #[error("Invalid rank notation: {0}")]
    InvalidRankChar(char),
}

/// A specialized [`Result`] type for this crate, used by all fallible functions.
pub type Result<T, E = Error> = std::result::Result<T, E>;
