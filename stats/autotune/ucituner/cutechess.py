import csv
import hashlib
import random
import subprocess
import tempfile

import chess.pgn

from ucituner.config import Config, RangeKind


class Tuner:
    def __init__(self, config: Config) -> None:
        self.config = config
        with open(self.config.log_csv_path, "a") as f:
            writer = csv.writer(f)
            # writer.writerow(list(self.config.parameter_ranges.keys()))
            writer.writerow([v.start for v in self.config.parameter_ranges.values()])

    def update(self):
        """Run a match between random instances of the engine and update the ranges."""

        engines = {}
        for _ in range(self.config.concurrency):
            new = {}
            for k, v in self.config.parameter_ranges.items():
                x = random.gauss(v.start, v.std)
                if v.kind == RangeKind.Integer:
                    x = int(x)
                new[k] = x
            options = " ".join(f"option.{k}={v}" for k, v in new.items())
            name = "Candidate-" + hashlib.md5(options.encode()).hexdigest()[:8]

            engines[name] = {
                "options": new,
                "config": f"-engine cmd={self.config.engine.command} name={name} {options}",
            }
        engines_str = " ".join(v["config"] for v in engines.values())
        each = f"-each proto=uci tc=1+0.1 timemargin=20 book={self.config.book_path} bookdepth=10 "
        each += " ".join(
            f"option.{k}={v}" for k, v in self.config.engine.fixed_parameters.items()
        )

        pgn = tempfile.mktemp(suffix=".pgn")
        command = f"""
            cutechess-cli 
            {engines_str}
            {each}
            -concurrency {self.config.concurrency}
            -draw movenumber=15 movecount=10 score=20
            -resign movecount=10 score=600 twosided=true
            -tb {self.config.syzygy_path}
            -games 1
            -rounds 1
            -pgnout {pgn} min fi
            """.strip().replace(
            "\n", " "
        )

        result = subprocess.Popen(
            command, shell=True, stdout=subprocess.PIPE, text=True
        )

        assert result.stdout is not None
        for line in result.stdout:
            print(line)
        result.stdout.close()

        if result.returncode != 0:
            print(
                f"WARNING: cutechess-cli exited with non zero code: {result.returncode}"
            )

        with open(pgn) as f:
            while game := chess.pgn.read_game(f):
                white, black = game.headers["White"], game.headers["Black"]
                result = game.headers["Result"]
                winner = (
                    white if result == "1-0" else black if result == "0-1" else None
                )
                if winner:
                    for k, v in self.config.parameter_ranges.items():
                        v.start += self.config.apply_factor * (
                            engines[winner]["options"][k] - v.start
                        )

        with open(self.config.log_csv_path, "a") as f:
            writer = csv.writer(f)
            writer.writerow([v.start for v in self.config.parameter_ranges.values()])

        return


if __name__ == "__main__":
    import sys

    with open(sys.argv[1]) as f:
        config = Config.model_validate_json(f.read())
    tuner = Tuner(config)
    for _ in range(config.n_generations):
        tuner.update()
