fn main() {
    cc::Build::new()
        .include("external/fathom/src")
        .file("external/fathom/src/tbprobe.c")
        .flag("-DTB_NO_HELPER_API")
        .flag_if_supported("-TP")
        .compile("fathom");
    println!("cargo:rerun-if-changed=external/fathom");
}
