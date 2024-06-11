use chess::ChessMove;

#[derive(Debug, Clone, Default)]
pub struct Position {
    pub start_pos: chess::Board,
    pub moves: Vec<chess::ChessMove>,
    pub starting_halfmove_clock: usize,
}

#[derive(Debug)]
pub enum Command {
    Uci,
    Debug,
    IsReady,
    SetOption(Option),
    UciNewGame,
    Position(Position),
    Go(Vec<GoOption>),
    Stop,
    PonderHit,
    Quit,
    Unknown(String),
}

#[derive(Debug, PartialEq, Eq)]
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
    // Non-standard: Provides a "be less verbose" flag to the engine.
    Silent,
}

#[derive(Debug)]
pub struct Option {
    pub name: String,
    pub value: String,
}

impl From<&str> for Command {
    fn from(s: &str) -> Self {
        match s.parse::<Command>() {
            Ok(command) => command,
            Err(e) => Command::Unknown(e.to_string()),
        }
    }
}

impl std::str::FromStr for Command {
    type Err = String;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        let (command, rest) = s.split_once(' ').unwrap_or((s, ""));
        match command {
            "uci" => Ok(Command::Uci),
            "debug" => Ok(Command::Debug),
            "isready" => Ok(Command::IsReady),
            "setoption" => Ok(Command::SetOption(rest.parse()?)),
            "ucinewgame" => Ok(Command::UciNewGame),
            "position" => Ok(Command::Position(rest.parse()?)),
            "go" => Ok(Command::Go(parse_go_opitons(rest)?)),
            "stop" => Ok(Command::Stop),
            "ponderhit" => Ok(Command::PonderHit),
            "quit" => Ok(Command::Quit),
            _ => Err(format!("Invalid UCI command: {s}")),
        }
    }
}

impl std::str::FromStr for Option {
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

        Ok(Option {
            name: name.unwrap().to_string(),
            value: value.unwrap().to_string(),
        })
    }
}

impl std::str::FromStr for Position {
    type Err = String;
    fn from_str(s: &str) -> Result<Position, String> {
        const START_POS_FEN: &str = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        let split = s.split_whitespace().collect::<Vec<_>>();
        let (fen, rest) = match split.first() {
            Some(&"startpos") => (START_POS_FEN.to_string(), &split[1..]),
            _ if split.len() >= 7 => (split[1..7].join(" "), &split[7..]),
            _ => {
                return Err(format!("Invalid position: {s}"));
            }
        };

        let moves = if rest.len() >= 2 && rest[0] == "moves" {
            rest.iter()
                .skip(1)
                .map(|s| s.parse::<ChessMove>())
                .collect::<Result<Vec<_>, _>>()
                .map_err(|e| format!("Invalid move: {e}"))?
        } else if rest.is_empty() {
            vec![]
        } else {
            return Err(format!("Invalid position: {s}"));
        };

        let start_pos = chess::Board::from_str(&fen).map_err(|e| format!("{e}"))?;
        verify_moves(start_pos, &moves)?;
        let starting_halfmove_clock = fen
            .split_whitespace()
            .nth(4)
            .unwrap()
            .parse()
            .map_err(|e| format!("{e}"))?;

        Ok(Position {
            start_pos,
            moves,
            starting_halfmove_clock,
        })
    }
}

fn verify_moves(mut board: chess::Board, moves: &[chess::ChessMove]) -> Result<(), String> {
    for mv in moves {
        if !board.legal(*mv) {
            return Err(format!("Illegal move: {mv}"));
        }
        board = board.make_move_new(*mv);
    }
    Ok(())
}

fn parse_go_opitons(s: &str) -> Result<Vec<GoOption>, String> {
    let mut split = s
        .split_whitespace()
        .collect::<std::collections::VecDeque<_>>();

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
            "silent" => options.push(GoOption::Silent),
            _ => {
                return Err(format!("Invalid go option: {opt}"));
            }
        }
    }

    Ok(options)
}
