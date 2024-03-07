//! # Memory leak tests
//!
//! ## Why?
//! The fathom library is a little strange with how it deals with freeing memory.
//!
//! Calling `tb_init`, `tb_free`, `tb_init`, `tb_free` in that sequence triggers a memory
//! corruption. See [this POC][bug-poc] using the `fathom-syzygy` crate. That crate does many
//! things on top of fathom, but the issue resides in fathom itself.
//!
//! ## Solution
//! This crate treats this issue with the following solution: Don't call `tb_free`.
//!
//! ## Wait, what?
//! Yes, that's right. This crate doesn't even link against the `tb_free` function. This isn't as
//! bad as it sounds, though. This is because fathom is strange, and `tb_init` actually calls free
//! on the things it reinitializes.
//!
//! This test is about verifying that hypothesis.
//!
//! I have run this test with a _lot_ of iterations, and not seen any change in the programs memory
//! usage during or after that time. The test is ignored by default, as it is very slow.
//!
//! [bug-poc]: https://github.com/malu/fathom-syzygy/commit/1a35239920424ed321ccea8906ccbacb36f7b77f

#[test]
#[ignore]
fn test_memory_leaks() {
    for _ in 0..100_000 {
        let tb = unsafe {
            fathom::Tablebase::load(concat!(env!("CARGO_MANIFEST_DIR"), "/../../syzygy"))
        };
        let tb = tb.unwrap();
        std::hint::black_box(tb);
    }
    std::thread::sleep(std::time::Duration::from_secs(20));
}
