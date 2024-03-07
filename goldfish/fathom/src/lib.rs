mod error;
mod probe;
mod sys;
mod wdl;

use std::{ffi::CString, path::Path};

pub use error::Error;
pub use wdl::Wdl;

#[derive(Debug)]
pub struct Tablebase {
    max_pieces: u32,
}

// The undelying C library doesn't support more than one instance, so we emulate this
// by having one singleton instance, and no public way of making new ones.
static mut TB: Tablebase = Tablebase { max_pieces: 0 };

impl Tablebase {
    /// Load a Syzygy tablebase from the given path.
    ///
    /// # Errors
    /// Returns an error if the path is invalid, the tablebase failed to initialize, or no files were
    /// found at the given path.
    ///
    /// # Safety
    /// This function is not thread-safe. No other thread should call it at the same time.
    /// Furthermore, calling this may invalidate any other references to the tablebase, or
    /// corrupt probes that are in progress.
    ///
    /// This should only ever be called when not searching.
    pub unsafe fn load<P: AsRef<Path>>(path: P) -> Result<&'static Self, Error> {
        let pathref = path.as_ref();
        let pathstr = pathref.to_str().ok_or(Error::InvalidPath)?;
        let c_string = CString::new(pathstr).map_err(|_| Error::InvalidPath)?;

        let success = unsafe { sys::tb_init(c_string.as_ptr()) };

        if !success {
            return Err(Error::FailedToInitialize);
        }

        let max_pieces = unsafe { sys::TB_LARGEST };
        unsafe { TB.max_pieces = max_pieces };
        if max_pieces == 0 {
            return Err(Error::NoFilesFound);
        }

        Ok(&TB)
    }
}
