#!/usr/bin/env python3

import os
import subprocess
import sys
from tqdm import tqdm
import numpy as np


def run_bench():
    result = (
        subprocess.check_output(
            ["cd {} && make bench".format(build_dir)],
            stderr=subprocess.STDOUT,
            shell=True,
        )
        .decode("utf-8")
        .split("\n")
    )
    for line in map(lambda s: s.strip(), result):
        if line.startswith("Total time"):
            time = int(line.split()[-1])
        elif line.startswith("Nodes searched"):
            nodes = int(line.split()[-1])
    return time, nodes


if __name__ == "__main__":

    build_dir = os.path.join(os.path.dirname(__file__), "build-release")

    if not os.path.isdir(build_dir):
        print("No release build directory found. Please setup build-release.")
        sys.exit(1)

    time, nodes = [sorted(x)[1:-1] for x in np.array([run_bench() for _ in tqdm(range(20))]).T]
    mtime, stdtime = np.mean(time), np.std(time)
    ci_time = mtime - 1.96 * stdtime, mtime + 1.96 * stdtime
    print("Times (ms):", time)
    print("Average: {:.4e}".format(mtime))
    print("Std: {:.4e}".format(stdtime))
    print("CI 95%: [{:.4e}, {:.4e}]".format(*ci_time))

    print(
        "Nodes constant: {}".format(
            "TRUE" if all(n == nodes[0] for n in nodes) else "FALSE"
        )
    )
    nodes = nodes[0]
    print("Nodes: {}".format(nodes))

    print("Nodes/second: {:.4e}".format(nodes / mtime))
    print("CI 95%: [{:.4e}, {:.4e}]".format(nodes / ci_time[0], nodes / ci_time[1]))
