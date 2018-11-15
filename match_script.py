import os
import argparse

class FullPath(argparse.Action):
    """Expand user- and relative-paths"""
    def __call__(self, parser, namespace, values, option_string=None):
        setattr(namespace, self.dest, os.path.abspath(os.path.expanduser(values)))

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-fcp', action=FullPath, required=True,
            help='Path to first contestant engine.')
    parser.add_argument('-scp', action=FullPath, required=True,
            help='Path to second contestant engine.')
    parser.add_argument('--fcp-stdin', default='',
            help='String to send to first contestant on startup.')
    parser.add_argument('--scp-stdin', default='',
            help='String to send to second contestant on startup.')
    parser.add_argument('-tc', default='40/60',
            help='Time control to use for the match.')
    parser.add_argument('-games', type=int, default=1,
            help='Number of games to play.')
    parser.add_argument('-o', '--output-file', default='match-history.pgn',
            help='File name to write the match pgn to.')
    parser.add_argument('--bookfile', default='openingbooks/gm2001.bin',
            help='Path to opening book to use.')
    parser.add_argument('--book-depth', type=int, default=4,
            help='Number moves to play from opening book.')
    parser.add_argument('--resign', type=int, nargs=2, default=(3, 700),
            help='Resign games where eval exceeds some limit for some number of moves.')
    parser.add_argument('--draw', type=int, nargs=2, default=(30, 5),
            help='Draw games where eval is below some limit after a given number of moves (moves, limit).')
    parser.add_argument('--auto-run', action='store_true', default=False)

    args = parser.parse_args()


    pgnout = 'match-history.pgn'

    bookfile = 'openingbooks/gm2001.bin'
    bookdepth = 4


    fcp = 'cmd={} initstr={}'.format(args.fcp, args.fcp_stdin)
    scp = 'cmd={} initstr={}'.format(args.scp, args.scp_stdin)
    both = 'proto=uci book={bookfile} bookdepth={bookdepth} tc={tc}'.format(bookfile=args.bookfile, bookdepth=args.book_depth, tc=args.tc)
    general = '-games {} -repeat -pgnout {} -resign {} {} -draw {} {}'.format(args.games, args.output_file, args.resign[0], args.resign[1], args.draw[0], args.draw[1])

    command = 'cutechess-cli -fcp {fcp} -scp {scp} -both {both} {general}'.format(fcp=fcp, scp=scp, both=both, general=general)

    print(command)
    if args.auto_run:
        os.system(command)
    else:
        start = input('\nDoes the command look right? (y/n) ')
        if start.lower().startswith('y'):
            os.system(command)
