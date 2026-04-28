mod error;
mod probe;
mod sys;
mod wdl;

use std::marker::PhantomData;
use std::sync::Mutex;
use std::{ffi::CString, path::Path};

pub use error::Error;
pub use wdl::Wdl;

// The undelying C library doesn't support more than one loaded tablebase, so we model this
// by enforcing that only one entity may control the tablebase at any one time.
#[derive(Debug)]
pub struct TablebaseOwnership(PhantomData<()>);

static TB_OWNER: Mutex<Option<TablebaseOwnership>> =
    Mutex::new(Some(TablebaseOwnership(PhantomData)));

impl Drop for TablebaseOwnership {
    fn drop(&mut self) {
        *TB_OWNER.lock().unwrap() = Some(TablebaseOwnership(PhantomData));
    }
}

/// An initialized fathom tablebase instance.
#[derive(Debug)]
pub struct Tablebase {
    max_pieces: u32,
    _ownership: TablebaseOwnership,
}

impl Tablebase {
    /// Attempt to aquire the tablebase.
    ///
    /// Returns [`None`] if someone else already owns it.
    ///
    /// # Panics
    /// Panics if the tablebase ownership system has been poisoned.
    pub fn acquire() -> Option<TablebaseOwnership> {
        TB_OWNER.lock().unwrap().take()
    }

    /// Give back the tablebase for someone else to use.
    ///
    /// This is equivalent to dropping the ownership, and just exists for symmetry in the API, and
    /// for those who prefer being explicit about their intent.
    pub fn release(ownership: TablebaseOwnership) {
        drop(ownership);
    }

    /// Load a Syzygy tablebase from the given path.
    ///
    /// Calling this requires obtaining [`TablebaseOwnership`] first, which is done with [`Tablebase::acquire`].
    /// If sucessful, the resulting [`Tablebase`] embeds the ownership token, and the tablebase is
    /// only made available to others once dropped.
    ///
    /// # Errors
    /// Returns an error if the path is invalid, the tablebase failed to initialize, no files were
    /// found at the given path, or if has already been initialized.
    ///
    /// In all cases the tablebase ownership is also returned, so that you may try again without
    /// losing your ownership.
    ///
    /// # Safety
    /// This function is not thread-safe. No other thread should call it at the same time.
    /// Furthermore, calling this may invalidate any other references to the tablebase, or
    /// corrupt probes that are in progress.
    ///
    /// The presence of the [`TablebaseOwnership`] parameter enforces these safety requirements,
    /// so therefore the function is not marked as unsafe.
    pub fn load<P: AsRef<Path>>(
        ownership: TablebaseOwnership,
        path: P,
    ) -> Result<Self, (Error, TablebaseOwnership)> {
        let pathref = path.as_ref();
        let Some(pathstr) = pathref.to_str() else {
            return Err((Error::InvalidPath, ownership));
        };
        let Ok(c_string) = CString::new(pathstr) else {
            return Err((Error::InvalidPath, ownership));
        };

        let success = unsafe { sys::tb_init(c_string.as_ptr()) };

        if !success {
            return Err((Error::FailedToInitialize, ownership));
        }

        let max_pieces = unsafe { sys::TB_LARGEST };
        if max_pieces == 0 {
            return Err((Error::NoFilesFound, ownership));
        }

        Ok(Tablebase {
            max_pieces,
            _ownership: ownership,
        })
    }
}
