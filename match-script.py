import os
import argparse

class FullPath(argparse.Action):
    """Expand user- and relative-paths"""
    def __call__(self, parser, namespace, values, option_string=None):
        setattr(namespace, self.dest, os.path.abspath(os.path.expanduser(values)))

parser = argparse.ArgumentParser()
parser.add_argument('-fcp', action=FullPath, required=True,
        help='Path to first contestant engine.')
parser.add_argument('-scp', action=FullPath, required=True,
        help='Path to second contestant engine.')
parser.add_argument('-tc', default='40/60',
        help='Time control to use for the match.')
parser.add_argument('-games', type=int, default=1,
        help='Number of games to play.')
args = parser.parse_args()


pgnout = 'match-history.pgn'

bookfile = 'openingbooks/gm2001.bin'
bookdepth = 4


fcp = 'cmd={}'.format(args.fcp)
scp = 'cmd={}'.format(args.scp)
both = 'proto=uci book={bookfile} bookdepth={bookdepth} tc={tc}'.format(bookfile=bookfile, bookdepth=bookdepth, tc=args.tc)
general = '-games {games} -repeat -pgnout {pgnout} -resign 3 700 -draw 30 5'.format(games=args.games, pgnout=pgnout)

command = 'cutechess-cli -fcp {fcp} -scp {scp} -both {both} {general}'.format(fcp=fcp, scp=scp, both=both, general=general)

print(command)
start = input('\nDoes the command look right? (y/n) ')
if start.lower().startswith('y'):
    os.system(command)
