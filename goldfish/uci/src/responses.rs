use chess::ChessMove;

#[derive(Debug)]
pub struct EngineOptionSpesification {
    pub name: String,
    pub option_type: EngineOptionType,
    pub default: Option<String>,
    pub min: Option<usize>,
    pub max: Option<usize>,
    pub var: Option<Vec<String>>,
}

#[derive(Debug)]
pub enum EngineOptionType {
    Check,
    Spin,
    Combo,
    Button,
    String,
}

#[derive(Debug)]
pub enum InfoPart<'a> {
    Cp(i16),
    CurrLine(&'a [ChessMove]),
    CurrMove(ChessMove),
    CurrMoveNumber(usize),
    Depth(usize),
    HashFull(usize),
    LowerBound(i32),
    Mate(i16),
    MultiPv(usize),
    Nodes(usize),
    Nps(usize),
    Pv(&'a [ChessMove]),
    Refutation(&'a [ChessMove]),
    SelDepth(usize),
    String(String),
    TbHits(usize),
    Time(usize),
    UpperBound(i32),
}

#[derive(Debug, Default)]
pub struct Info {
    line: String,
}

impl Info {
    #[must_use]
    pub fn new() -> Self {
        Self {
            line: String::with_capacity(128),
        }
    }

    #[must_use]
    pub fn with(mut self, part: InfoPart) -> Self {
        let fmt_moves = |moves: &[ChessMove]| {
            moves
                .iter()
                .map(ToString::to_string)
                .collect::<Vec<_>>()
                .join(" ")
        };
        let add = match part {
            InfoPart::Cp(x) => format!("score cp {x}"),
            InfoPart::CurrLine(moves) => format!("currline {}", fmt_moves(moves)),
            InfoPart::CurrMove(mv) => format!("currmove {mv}"),
            InfoPart::CurrMoveNumber(n) => format!("currmovenumber {n}"),
            InfoPart::Depth(n) => format!("depth {n}"),
            InfoPart::HashFull(n) => format!("hashfull {n}"),
            InfoPart::LowerBound(x) => format!("lowerbound {x}"),
            InfoPart::Mate(x) => format!("score mate {x}"),
            InfoPart::MultiPv(n) => format!("multipv {n}"),
            InfoPart::Nodes(n) => format!("nodes {n}"),
            InfoPart::Nps(n) => format!("nps {n}"),
            InfoPart::Pv(moves) => format!("pv {}", fmt_moves(moves)),
            InfoPart::Refutation(moves) => format!("refutation {}", fmt_moves(moves)),
            InfoPart::SelDepth(n) => format!("seldepth {n}"),
            InfoPart::String(s) => format!("string {s}"),
            InfoPart::TbHits(n) => format!("tbhits {n}"),
            InfoPart::Time(n) => format!("time {n}"),
            InfoPart::UpperBound(x) => format!("upperbound {x}"),
        };
        self.line.push(' ');
        self.line.push_str(&add);
        self
    }
}

impl std::fmt::Display for Info {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str("info")?;
        f.write_str(&self.line)
    }
}

impl std::fmt::Display for EngineOptionSpesification {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str("option name ")?;
        f.write_str(&self.name)?;
        f.write_str(" type ")?;
        match self.option_type {
            EngineOptionType::Check => f.write_str("check")?,
            EngineOptionType::Spin => f.write_str("spin")?,
            EngineOptionType::Combo => f.write_str("combo")?,
            EngineOptionType::Button => f.write_str("button")?,
            EngineOptionType::String => f.write_str("string")?,
        }
        if let Some(ref default) = self.default {
            f.write_str(" default ")?;
            f.write_str(default)?;
        }
        if let Some(ref min) = self.min {
            f.write_str(" min ")?;
            f.write_str(&min.to_string())?;
        }
        if let Some(ref max) = self.max {
            f.write_str(" max ")?;
            f.write_str(&max.to_string())?;
        }
        if let Some(ref var) = self.var {
            f.write_str(" var ")?;
            f.write_str(&var.join(" var "))?;
        }
        Ok(())
    }
}
