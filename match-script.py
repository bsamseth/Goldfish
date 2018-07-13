import os

test = '/root/goldfish/build-release/goldfish.x'
base = '/root/goldfish/build-release/goldfish-old.x'

pgnout = 'match-result-with-book.pgn'

tc = '40/60'
games = 30
bookfile = 'openingbooks/gm2001.bin'
bookdepth = 4


fcp = 'name=goldfish-new cmd={}'.format(test)
scp = 'name=goldfish-old cmd={}'.format(base)
both = 'proto=uci book={bookfile} bookdepth={bookdepth} tc={tc}'.format(bookfile=bookfile, bookdepth=bookdepth, tc=tc)
general = '-games {games} -pgnout {pgnout} -resign 3 500 -draw 20 5'.format(games=games, pgnout=pgnout)

command = 'cutechess-cli -fcp {fcp} -scp {scp} -both {both} {general}'.format(fcp=fcp, scp=scp, both=both, general=general)

print(command)
start = input('\nDoes the command look right? (y/n) ')
if start.lower().startswith('y'):
    os.system(command)
