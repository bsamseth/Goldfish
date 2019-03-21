"""
Search Verification

This set of tests will ensure that Goldfish is able to complete a series of
puzzles, meant to act as a safety net for catching critical bugs introduced
into the search algorithm. It will also catch cases where Goldfish takes to
long (in terms of depth) to find certain variations, if agressive pruning is
applied. Some of these tests might be expected to break with larger changes,
but this will then have to be explicitly addressed.

Puzzles given in EPD format, see https://www.chessprogramming.org/Extended_Position_Description
Types of puzzles:
  1. Single line mate (dm && pv)
      Must find correct mate distance and play out each move.
  2. Multi-variation mate (dm)
      Must checkmate against stockfish in given number of moves, but can choose
      line (i.e. more than one winning line).
  3. Single line tactic (pv)
      Must find the _one_ strong line and play out each move.

TODO: Add more test positions once these pass.
"""

import os
import unittest
import random
import logging
import chess
import chess.engine
from chester.match import play_match
from chester.timecontrol import TimeControl


# Enable debug logging.
# logging.basicConfig(level=logging.DEBUG)

build_path = os.path.abspath(
    os.path.join(os.path.dirname(os.path.dirname(__file__)), "build")
)
if not os.path.exists(build_path):
    build_type = os.environ.get("CMAKE_BUILD_TYPE", "Release")
    os.mkdir(build_path)
    os.system(f"cd {build_path} && cmake .. -DCMAKE_BUILD_TYPE={build_type}")

goldfish_path = os.path.join(build_path, "goldfish.x")


test_cases = """
1k1r4/pp1b1R2/3q2pp/4p3/2B5/4Q3/PPP2B2/2K5 b - - dm -3; pv "d6d1 c1d1 d7g4 d1e1 d8d1"; id "Bratko-Kopec.01";
1r6/5p1k/7p/2pBp3/p1P1P3/8/1PK1Rq2/1R6 b - - dm -3; pv "f2e2 c2c1 b8g8 b2b4 g8g1"; id "https://lichess.org/wkc61nls/black#103";
6KQ/8/8/8/1kn5/8/4B3/8 w - - dm 6; id "https://lichess.org/7ghS4Sop/white#148";
5k2/5p2/6p1/p1p3Q1/1p4K1/1P1r4/2q5/5R2 b - - dm -3; pv "c2e2 g4h4 e2h2 h4g4 h2g3"; id "https://lichess.org/ROTrEoyB/black#87";
8/2pp1k2/4p3/4P1p1/7p/3n1P1P/1r6/6K1 b - - dm -6; id "https://lichess.org/hEQgRWnK/black#107";
1rbq1rk1/p1b1nppp/1p2p3/8/1B1pN3/P2B4/1P3PPP/2RQ1R1K w - - pv "e4f6"; acd 10; id "Kaufman.01";
3r2k1/p2r1p1p/1p2p1p1/q4n2/3P4/PQ5P/1P1RNPP1/3R2K1 b - - pv "f5d4"; acd 10; id "Kaufman.02";
""".strip().splitlines()


class TestSearch(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        os.system(f"cd {build_path} && make -j5 goldfish.x")

    def setUp(self):
        self.goldfish = chess.engine.SimpleEngine.popen_uci(goldfish_path)
        self.stockfish = chess.engine.SimpleEngine.popen_uci("stockfish")

    def tearDown(self):
        self.goldfish.quit()
        self.stockfish.quit()

    def test_timed_search(self):
        """Search a limited amount of milliseconds."""
        info = self.goldfish.analyse(
            chess.Board(), chess.engine.Limit(time=0.1), info=chess.engine.Info.ALL
        )
        self.assertLessEqual(0.100, info["time"])

    def test_node_limited_search(self):
        """Search a limited number of nodes."""
        nodes = random.randrange(10000, 99999)
        info = self.goldfish.analyse(
            chess.Board(), chess.engine.Limit(nodes=nodes), info=chess.engine.Info.ALL
        )
        self.assertAlmostEqual(1.00, info["nodes"] / nodes, 3)

    def test_play_with_clock(self):
        """Play a timed game to completion without error (losing is fine)."""
        game = play_match(goldfish_path, "stockfish", TimeControl(1, increment=0.0))
        self.assertNotEqual("*", game.headers["Result"])  # Just demand _a_ result.

    def test_puzzles(self):
        """Solve each of the following puzzles: \n{test_cases}."""
        for epd in test_cases:
            board = chess.Board()
            case = board.set_epd(epd)

            with self.subTest(msg=f"Puzzle: {epd}"):

                # Set search limits based on puzzle type. 2x meant to give ample time to find.
                if "dm" in case:
                    search_limit = chess.engine.Limit(depth=2 * abs(case["dm"]))
                else:
                    assert "acd" in case, "Non-mate puzzles must have a depth limit"
                    search_limit = chess.engine.Limit(depth=case["acd"])

                if "dm" in case:
                    # Check that the correct mate distance is found.
                    info = self.goldfish.analyse(
                        board, search_limit, info=chess.engine.Info.ALL
                    )
                    self.assertIsInstance(
                        info["score"].white(),
                        chess.engine.Mate,
                        msg=f"Expected {chess.engine.Mate(case['dm'])!r}, score found was {info['score'].white()!r}",
                    )
                    self.assertEqual(
                        case["dm"],
                        info["score"].white().moves,
                        f"Incorrect mate distance",
                    )

                if "pv" in case:
                    # Check that Goldfish plays the expected line.
                    # pv_us is the moves we are expected to play,
                    # pv_them is any response played by the opponent (not asserted).
                    pv = case["pv"].split()
                    pv_us, pv_them = pv[::2], pv[1::2]

                    for i, move in enumerate(pv_us):
                        output = self.goldfish.play(board, search_limit, ponder=True)

                        self.assertEqual(
                            chess.Move.from_uci(move),
                            output.move,
                            f"Incorrect move at puzzle {case['id']} at this pos:\n\n{board}\n",
                        )

                        # Play our move and any follow-up from the opponent.
                        board.push(output.move)
                        # print(board, end="\n\n")
                        try:
                            board.push_uci(pv_them[i])
                            # print(board, end="\n\n")
                        except IndexError:
                            break

                elif "dm" in case:
                    # We have a mate, but more than one possible line. In this case Goldfish should beat Stockfish
                    # in the exact number of moves given by the mate distance.
                    winning_side = board.turn
                    for move_count in range(abs(case["dm"])):
                        for engine in (self.goldfish, self.stockfish):
                            if board.is_game_over(claim_draw=True):
                                break
                            output = engine.play(board, search_limit)
                            board.push(output.move)
                            # print(board, end="\n\n")

                        if board.is_game_over(claim_draw=True):
                            break

                    self.assertTrue(
                        board.is_checkmate() and board.turn != winning_side,
                        f"Failed to mate in puzzle {case['id']}",
                    )


if __name__ == "__main__":
    unittest.main()
