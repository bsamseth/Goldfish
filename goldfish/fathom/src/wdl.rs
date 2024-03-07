use super::sys;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Wdl {
    Loss,
    BlessedLoss,
    Draw,
    CursedWin,
    Win,
}

impl TryFrom<u32> for Wdl {
    type Error = ();
    fn try_from(result: u32) -> Result<Self, Self::Error> {
        match (result & sys::TB_RESULT_WDL_MASK) >> sys::TB_RESULT_WDL_SHIFT {
            sys::TB_LOSS => Ok(Wdl::Loss),
            sys::TB_BLESSED_LOSS => Ok(Wdl::BlessedLoss),
            sys::TB_DRAW => Ok(Wdl::Draw),
            sys::TB_CURSED_WIN => Ok(Wdl::CursedWin),
            sys::TB_WIN => Ok(Wdl::Win),
            _ => Err(()),
        }
    }
}
