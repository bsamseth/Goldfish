pub const TB_LOSS: u32 = 0;
pub const TB_BLESSED_LOSS: u32 = 1;
pub const TB_DRAW: u32 = 2;
pub const TB_CURSED_WIN: u32 = 3;
pub const TB_WIN: u32 = 4;

pub const TB_PROMOTES_NONE: u32 = 0;
pub const TB_PROMOTES_QUEEN: u32 = 1;
pub const TB_PROMOTES_ROOK: u32 = 2;
pub const TB_PROMOTES_BISHOP: u32 = 3;
pub const TB_PROMOTES_KNIGHT: u32 = 4;

pub const TB_RESULT_WDL_MASK: u32 = 0x0000_000F;
pub const TB_RESULT_TO_MASK: u32 = 0x0000_03F0;
pub const TB_RESULT_FROM_MASK: u32 = 0x0000_FC00;
pub const TB_RESULT_PROMOTES_MASK: u32 = 0x0007_0000;
pub const TB_RESULT_WDL_SHIFT: u32 = 0;
pub const TB_RESULT_TO_SHIFT: u32 = 4;
pub const TB_RESULT_FROM_SHIFT: u32 = 10;
pub const TB_RESULT_PROMOTES_SHIFT: u32 = 16;

pub const TB_RESULT_CHECKMATE: u32 = 0x0000_0004;
pub const TB_RESULT_STALEMATE: u32 = 0x0000_0002;
pub const TB_RESULT_FAILED: u32 = 0xFFFF_FFFF;

pub const TB_MAX_MOVES: u32 = 192 + 1;

/// Probe the Win-Draw-Loss (WDL) table.
///
/// Important: Only positions without castling rights and with a rule50 of 0 are supported.
///
/// # Arguments
/// - white, black, kings, queens, rooks, bishops, knights, pawns:
///   The current position (bitboards).
/// - rule50:
///   The 50-move half-move clock.
/// - castling:
///   Castling rights.  Set to zero if no castling is possible.
/// - ep:
///   The en passant square (if exists).  Set to zero if there is no en passant square.
/// - turn:
///   true=white, false=black
///
/// # Return
/// - One of `{TB_LOSS, TB_BLESSED_LOSS, TB_DRAW, TB_CURSED_WIN, TB_WIN}`.
///   Otherwise returns `TB_RESULT_FAILED` if the probe failed.
///
/// # Notes
/// - Engines should use this function during search.
/// - This function is thread safe.
///
/// # Safety
/// This assumes that `tb_init` has been called sucessfully, and that
/// the arguments describe a valid position. Otherwise this is undefined.
#[allow(clippy::too_many_arguments)]
#[inline]
pub unsafe fn tb_probe_wdl(
    white: u64,
    black: u64,
    kings: u64,
    queens: u64,
    rooks: u64,
    bishops: u64,
    knights: u64,
    pawns: u64,
    ep: u32,
    turn: u8,
) -> u32 {
    tb_probe_wdl_impl(
        white, black, kings, queens, rooks, bishops, knights, pawns, ep, turn,
    )
}

/// Probe the Distance-To-Zero (DTZ) table.
///
/// # Arguments
/// - white, black, kings, queens, rooks, bishops, knights, pawns:
///   The current position (bitboards).
/// - rule50:
///   The 50-move half-move clock.
/// - castling:
///   Castling rights.  Set to zero if no castling is possible.
/// - ep:
///   The en passant square (if exists).  Set to zero if there is no en passant square.
/// - turn:
///   true=white, false=black
/// - results (OPTIONAL):
///   Alternative results, one for each possible legal move.  The passed array must be [`TB_MAX_MOVES`]
///   in size. If alternative results are not desired then set results=NULL.
///
/// # Return
/// - A [`TB_RESULT`] value comprising:
///   1) The WDL value ([`TB_GET_WDL`])
///   2) The suggested move ([`TB_GET_FROM`], [`TB_GET_TO`], [`TB_GET_PROMOTES`], [`TB_GET_EP`])
///   3) The DTZ value ([`TB_GET_DTZ`])
///   The suggested move is guaranteed to preserved the WDL value.
///
///   Otherwise:
///   1) [`TB_RESULT_STALEMATE`] is returned if the position is in stalemate.
///   2) [`TB_RESULT_CHECKMATE`] is returned if the position is in checkmate.
///   3) [`TB_RESULT_FAILED`] is returned if the probe failed.
///
///   If results!=NULL, then a [`TB_RESULT`] for each legal move will be generated
///   and stored in the results array.  The results array will be terminated
///   by [`TB_RESULT_FAILED`].
///
/// # Notes
/// - Engines can use this function to probe at the root.  This function should not be used during search.
/// - DTZ tablebases can suggest unnatural moves, especially for losing positions.  Engines may prefer to
///   traditional search combined with WDL move filtering using the alternative results array.
/// - This function is NOT thread safe. For engines this function should only be called once at the
///   root per search.
///
/// # Safety
/// This assumes that `tb_init` has been called sucessfully, and that the arguments describe a valid
/// position. It also assumes that no other thread is calling this function at the same time.
#[allow(clippy::too_many_arguments)]
pub unsafe fn tb_probe_root(
    white: u64,
    black: u64,
    kings: u64,
    queens: u64,
    rooks: u64,
    bishops: u64,
    knights: u64,
    pawns: u64,
    rule50: u32,
    ep: u32,
    turn: u8,
    results: *mut u32,
) -> u32 {
    tb_probe_root_impl(
        white, black, kings, queens, rooks, bishops, knights, pawns, rule50, ep, turn, results,
    )
}

// Extractors from a `TB_RESULT` value.
pub const fn tb_get_wdl(res: u32) -> u32 {
    (res & TB_RESULT_WDL_MASK) >> TB_RESULT_WDL_SHIFT
}
pub const fn tb_get_to(res: u32) -> u32 {
    (res & TB_RESULT_TO_MASK) >> TB_RESULT_TO_SHIFT
}
pub const fn tb_get_from(res: u32) -> u32 {
    (res & TB_RESULT_FROM_MASK) >> TB_RESULT_FROM_SHIFT
}
pub const fn tb_get_promotes(res: u32) -> u32 {
    (res & TB_RESULT_PROMOTES_MASK) >> TB_RESULT_PROMOTES_SHIFT
}

extern "C" {
    pub static TB_LARGEST: u32;

    pub fn tb_init(filename: *const std::ffi::c_char) -> bool;

    fn tb_probe_wdl_impl(
        white: u64,
        black: u64,
        kings: u64,
        queens: u64,
        rooks: u64,
        bishops: u64,
        knights: u64,
        pawns: u64,
        ep: u32,
        turn: u8,
    ) -> u32;

    fn tb_probe_root_impl(
        white: u64,
        black: u64,
        kings: u64,
        queens: u64,
        rooks: u64,
        bishops: u64,
        knights: u64,
        pawns: u64,
        rule50: u32,
        ep: u32,
        turn: u8,
        results: *mut u32,
    ) -> u32;
}
