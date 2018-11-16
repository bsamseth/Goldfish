"""
This script facilitates running a tournament between a given set of engines.
"""
import os
import sys
import glob
import argparse
import tempfile
from itertools import combinations
from match_script import FullPath

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('binary_dir', action=FullPath,
            help='Directory containing binaries for all competitors.')
    parser.add_argument('-n', '--n-most-recent-competitors', type=int, default=-1,
            help='Maximum number of competitors to use, sorted by decending version number. Default is to use all.')
    parser.add_argument('-g', '--games', type=int, default=1,
            help='Number of games to play in each matchup.')
    parser.add_argument('-o', '--output-file', type=str, default=None,
            help="Name of output file.")
    parser.add_argument('--extra-match-options', default='',
            help="Extra parameters sent to match_script.py")

    args = parser.parse_args()

    if not os.path.isdir(args.binary_dir):
        print('Bad binary dir:', args.binary_dir)
        sys.exit(1)

    binaries = sorted(glob.glob(os.path.join(args.binary_dir, 'goldfish-v*')), reverse=True)
    if args.n_most_recent_competitors != -1:
        binaries = binaries[:args.n_most_recent_competitors]

    output_file = tempfile.mktemp()+'.pgn' if args.output_file is None else args.output_file

    for p1, p2 in combinations(binaries, 2):
        os.system('python3 match_script.py -fcp {} -scp {} -games {} --output-file {} {} --auto-run'.format(p1, p2, args.games, output_file, args.extra_match_options))
    os.system('mv {} .'.format(output_file))

