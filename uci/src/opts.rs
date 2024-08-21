use anyhow::Result;

pub use uciderive::UciOptions;

/// Interface for UCI options.
///
/// Engines should contain a struct that implements this trait to allow the GUI to
/// configure the engine. This trait is derivable.
///
/// # Example
///
/// ```rust
/// use uci::UciOptions;
///
/// #[derive(Debug, UciOptions)]
/// struct MyOptions {
///    #[uci(name = "Hash", kind = "spin", default = "16", min = "1", max = "128")]
///    pub hash_size_mb: usize,
///    #[uci(name = "SyzygyPath", kind = "string")]
///    pub syzygy_path: Option<std::path::PathBuf>,
/// }
/// ```
pub trait UciOptions: Default {
    /// Print the available options to stdout.
    fn print_options(&self);

    /// Set an option by name.
    ///
    /// # Errors
    /// If the option name is not recognized, or the value is invalid.
    fn set_option(&mut self, name: &str, value: &str) -> Result<()>;
}
