import argparse
import subprocess
from pathlib import Path

from pydantic import BaseModel
from tqdm import tqdm


class BenchSuite(BaseModel):
    depth: int
    uci_options: dict[str, str]
    cases: list[str]


class Case(BaseModel):
    depth: int
    uci_options: dict[str, str]
    fen: str


def run_bench(engine: Path, case: Case) -> int:
    proc = subprocess.Popen([engine], stdout=subprocess.PIPE, stdin=subprocess.PIPE)
    assert proc.stdout is not None
    assert proc.stdin is not None

    indata = "\n".join(
        ["uci"]
        + [f"setoption name {k} value {v}" for k, v in case.uci_options.items()]
        + [f"position fen {case.fen}", f"go depth {case.depth}\n"]
    ).encode()
    proc.stdin.write(indata)
    proc.stdin.flush()

    nodes = None
    for line in map(bytes.decode, iter(proc.stdout.readline, b"")):
        if line.startswith("bestmove"):
            break
        if line.startswith("info"):
            words = line.split()
            for label, value in zip(words[1:], words[2:]):
                if label == "nodes":
                    nodes = int(value)

    proc.stdin.write(b"quit\n")
    proc.stdin.flush()
    proc.wait()
    assert nodes is not None, f"no nodes counted for {case.fen}"
    return nodes


def run_suite(engine: Path, suite: BenchSuite, progress: bool = True) -> int:
    return sum(
        run_bench(
            engine, Case(depth=suite.depth, uci_options=suite.uci_options, fen=fen)
        )
        for fen in (suite.cases if not progress else tqdm(suite.cases))
    )


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("engine", type=Path)
    parser.add_argument("benchfile", type=Path)
    parser.add_argument("--no-progress", action="store_true")
    args = parser.parse_args()

    assert args.engine.exists() and args.engine.is_file()
    assert args.benchfile.exists() and args.benchfile.is_file()

    with open(args.benchfile) as f:
        suite = BenchSuite.model_validate_json(f.read())

    print(f"{run_suite(args.engine, suite, progress=not args.no_progress):_} nodes")
