from chester.timecontrol import TimeControl
from chester.tournament import play_tournament

players = ["all-versions/goldfish-v1.9.0", "all-versions/goldfish-v1.11.0"]
time_control = TimeControl(initial_time=3 * 60, increment=2)  # 3 + 2 (Lichess Blitz)
n_games = 50

for pgn in play_tournament(
    players,
    time_control,
    n_games=n_games,
    opening_book="openingbooks/gm2001.bin",
    repeat=True,
):
    print(pgn, "\n", flush=True)
