import argparse
from functools import cache
from pathlib import Path

import numpy as np
from pydantic import BaseModel
from scipy.optimize import brute
from tqdm import tqdm

from bench import BenchSuite, run_suite


class OptionConfig(BaseModel):
    name: str
    bounds: tuple[int, int]


class TuneConfig(BaseModel):
    options: list[OptionConfig]


def tune(
    engine: Path, config: TuneConfig, suite: BenchSuite
) -> tuple[int, dict[str, int]]:

    @cache
    def inner(args: tuple[str]) -> int:
        print("Running with args:", args)
        for i, opt in enumerate(config.options):
            suite.uci_options[opt.name] = args[i]
        return run_suite(engine, suite, progress=True)

    def objective(x: np.ndarray) -> int:
        x = np.round(x).astype(int)
        args = tuple(str(xi) for xi in x)
        return inner(args)

    def callback(x: np.ndarray, f: float, context: int):
        print(f"\n\nnodes: {f} params: {x}, context: {context}\n")

    bounds = [slice(*opt.bounds) for opt in config.options]

    result = brute(objective, bounds, Ns=1, disp=True, full_output=True)

    with open("/tmp/tune_result.pickl", "wb") as f:
        import pickle

        pickle.dump(result, f)

    print(
        "Global minimum:",
        ", ".join(f"{opt.name}: {x} " for x, opt in zip(result[0], config.options)),
    )
    print("Nodes:", result[1])


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("engine", type=Path)
    parser.add_argument("benchfile", type=Path)
    parser.add_argument("configfile", type=Path)
    args = parser.parse_args()

    assert args.engine.exists() and args.engine.is_file()
    assert args.benchfile.exists() and args.benchfile.is_file()
    assert args.configfile.exists() and args.configfile.is_file()

    with open(args.benchfile) as f:
        suite = BenchSuite.model_validate_json(f.read())
    with open(args.configfile) as f:
        config = TuneConfig.model_validate_json(f.read())

    tune(args.engine, config, suite)
