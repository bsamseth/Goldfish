#![allow(dead_code)]

use std::collections::VecDeque;
use std::str::FromStr;

use chess::{ChessMove, Game};

type Fen = String;
const START_POS_FEN: &str = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

#[derive(Debug)]
pub enum UciCommand {
    Uci,
    Debug,
    IsReady,
    SetOption(EngineOption),
    UciNewGame,
    Position(Game),
    Go(Vec<GoOption>),
    Stop,
    PonderHit,
    Quit,
    Unknown(String),
}

#[derive(Debug)]
pub enum GoOption {
    SearchMoves(Vec<chess::ChessMove>),
    Ponder,
    WTime(usize),
    BTime(usize),
    WInc(usize),
    BInc(usize),
    MovesToGo(usize),
    Depth(usize),
    Nodes(usize),
    Mate(usize),
    MoveTime(usize),
    Infinite,
}

#[derive(Debug)]
pub struct EngineOption {
    pub name: String,
    pub value: String,
}

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

impl From<&str> for UciCommand {
    fn from(s: &str) -> Self {
        match s.parse::<UciCommand>() {
            Ok(command) => command,
            Err(e) => UciCommand::Unknown(e.to_string()),
        }
    }
}

impl std::str::FromStr for UciCommand {
    type Err = String;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        let (command, rest) = s.split_once(' ').unwrap_or((s, ""));
        match command {
            "uci" => Ok(UciCommand::Uci),
            "debug" => Ok(UciCommand::Debug),
            "isready" => Ok(UciCommand::IsReady),
            "setoption" => Ok(UciCommand::SetOption(rest.parse()?)),
            "ucinewgame" => Ok(UciCommand::UciNewGame),
            "position" => Ok(UciCommand::Position(parse_position(rest)?)),
            "go" => Ok(UciCommand::Go(parse_go_opitons(rest)?)),
            "stop" => Ok(UciCommand::Stop),
            "ponderhit" => Ok(UciCommand::PonderHit),
            "quit" => Ok(UciCommand::Quit),
            _ => Err(format!("Invalid UCI command: {s}")),
        }
    }
}

impl std::str::FromStr for EngineOption {
    type Err = String;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        let mut name = None;
        let mut value = None;

        for chunk in s.split_whitespace().collect::<Vec<_>>().chunks(2) {
            if let &[key, val] = chunk {
                match (key, val) {
                    ("name", x) => name = Some(x),
                    ("value", x) => value = Some(x),
                    _ => {
                        return Err(format!("Invalid engine option: {s}"));
                    }
                }
            } else {
                return Err(format!("Invalid engine option: {chunk:?}"));
            }
        }

        if name.is_none() || value.is_none() {
            return Err(format!("Invalid engine option: {s}"));
        }

        Ok(EngineOption {
            name: name.unwrap().to_string(),
            value: value.unwrap().to_string(),
        })
    }
}

fn parse_position(s: &str) -> Result<Game, String> {
    let split = s.split_whitespace().collect::<Vec<_>>();
    let (fen, rest) = match split.first() {
        Some(&"startpos") => (START_POS_FEN.to_string(), &split[1..]),
        _ if split.len() >= 6 => (split[..6].join(" "), &split[6..]),
        _ => {
            return Err(format!("Invalid position: {s}"));
        }
    };

    let moves = if rest.len() >= 2 && rest[0] == "moves" {
        Some(
            rest.iter()
                .skip(1)
                .map(|s| s.parse::<ChessMove>())
                .collect::<Result<Vec<_>, _>>()
                .map_err(|e| format!("Invalid move: {e}"))?,
        )
    } else if rest.is_empty() {
        None
    } else {
        return Err(format!("Invalid position: {s}"));
    };

    let mut game = Game::from_str(&fen).map_err(|e| format!("{e}"))?;
    if let Some(moves) = moves {
        for mv in moves {
            if !game.make_move(mv) {
                return Err(format!("Invalid move: {mv}"));
            }
        }
    }

    Ok(game)
}

fn parse_go_opitons(s: &str) -> Result<Vec<GoOption>, String> {
    let mut split = s.split_whitespace().collect::<VecDeque<_>>();

    let mut next = move || {
        split
            .pop_front()
            .ok_or_else(|| "Invalid go options".to_string())
    };
    let mut options = Vec::new();

    while let Ok(opt) = next() {
        match opt {
            "infinite" => options.push(GoOption::Infinite),
            "ponder" => options.push(GoOption::Ponder),
            "wtime" => {
                let time = next()?.parse().map_err(|e| format!("Invalid wtime: {e}"))?;
                options.push(GoOption::WTime(time));
            }
            "btime" => {
                let time = next()?.parse().map_err(|e| format!("Invalid btime: {e}"))?;
                options.push(GoOption::BTime(time));
            }
            "winc" => {
                let time = next()?.parse().map_err(|e| format!("Invalid winc: {e}"))?;
                options.push(GoOption::WInc(time));
            }
            "binc" => {
                let time = next()?.parse().map_err(|e| format!("Invalid binc: {e}"))?;
                options.push(GoOption::BInc(time));
            }
            "movestogo" => {
                let moves = next()?.parse().map_err(|e| format!("Invalid moves: {e}"))?;
                options.push(GoOption::MovesToGo(moves));
            }
            "depth" => {
                let depth = next()?.parse().map_err(|e| format!("Invalid depth: {e}"))?;
                options.push(GoOption::Depth(depth));
            }
            "nodes" => {
                let nodes = next()?.parse().map_err(|e| format!("Invalid nodes: {e}"))?;
                options.push(GoOption::Nodes(nodes));
            }
            "mate" => {
                let mate = next()?.parse().map_err(|e| format!("Invalid mate: {e}"))?;
                options.push(GoOption::Mate(mate));
            }
            "movetime" => {
                let time = next()?
                    .parse()
                    .map_err(|e| format!("Invalid movetime: {e}"))?;
                options.push(GoOption::MoveTime(time));
            }
            "searchmoves" => {
                let mut moves = Vec::new();
                while let Ok(m) = next()?.parse::<ChessMove>() {
                    moves.push(m);
                }
                options.push(GoOption::SearchMoves(moves));
            }
            _ => {
                return Err(format!("Invalid go option: {opt}"));
            }
        }
    }

    Ok(options)
}
