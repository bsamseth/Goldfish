#include "goldfish.hpp"

#include "benchmark.hpp"
#include "projectmeta.hpp"
#include "tb.hpp"
#include "uci.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>
#include <vector>

extern std::vector<std::string> setup_bench();

namespace goldfish
{
void Goldfish::run()
{
    std::cin.exceptions(std::iostream::eofbit | std::iostream::failbit
                        | std::iostream::badbit);

    std::cout << "Goldfish v" << PROJECT_VERSION_MAJOR << "." << PROJECT_VERSION_MINOR
              << "." << PROJECT_VERSION_PATCH << " by B. Samseth\n";

    while (true)
    {
        std::string line;
        std::getline(std::cin, line);
        std::istringstream input(line);

        std::string token;
        input >> std::skipws >> token;
        if (token == "uci")
        {
            receive_initialize();
        }
        else if (token == "isready")
        {
            receive_ready();
        }
        else if (token == "ucinewgame")
        {
            receive_new_game();
        }
        else if (token == "position")
        {
            receive_position(input);
        }
        else if (token == "go")
        {
            receive_go(input);
        }
        else if (token == "stop")
        {
            receive_stop();
        }
        else if (token == "ponderhit")
        {
            receive_ponder_hit();
        }
        else if (token == "setoption")
        {
            receive_setoption(input);
        }
        else if (token == "bench")
        {
            receive_bench();
        }
        else if (token == "quit")
        {
            receive_quit();
            break;
        }
        else
        {
            std::cout << "Unknown command: " << token << std::endl;
        }
    }
}

void Goldfish::receive_quit()
{
    // We received a quit command. Stop calculating now and
    // cleanup!
    search.quit();
}

void Goldfish::receive_initialize()
{
    search.stop();

    // We received an initialization request.

    // We could do some global initialization here. Probably it would be best
    // to initialize all tables here as they will exist until the end of the
    // program.
    UCI::init(UCI::Options);

    // We must send an initialization answer back!
    std::cout << "id name Goldfish v" << PROJECT_VERSION_MAJOR << "."
              << PROJECT_VERSION_MINOR << "." << PROJECT_VERSION_PATCH << '\n';
    std::cout << "id author Bendik Samseth" << '\n';
    std::cout << UCI::Options << '\n';
    std::cout << "uciok" << std::endl;
}

void Goldfish::receive_ready()
{
    // We received a ready request. We must send the token back as soon as we
    // can. However, because we launch the search in a separate thread, our main
    // thread is able to handle the commands asynchronously to the search. If we
    // don't answer the ready request in time, our engine will probably be
    // killed by the GUI.
    std::cout << "readyok" << std::endl;
}

void Goldfish::receive_new_game()
{
    search.stop();

    // We received a new game command.

    // Initialize per-game settings here.
    current_position = Notation::to_position(Notation::STANDARDPOSITION);
}

void Goldfish::receive_position(std::istringstream& input)
{
    search.stop();

    // We received an position command. Just setup the position.

    std::string token;
    input >> token;
    if (token == "startpos")
    {
        current_position = Notation::to_position(Notation::STANDARDPOSITION);

        if (input >> token)
        {
            if (token != "moves")
            {
                throw std::exception();
            }
        }
    }
    else if (token == "fen")
    {
        std::string fen;

        while (input >> token)
        {
            if (token == "moves")
            {
                break;
            }
            else
            {
                fen += token + " ";
            }
        }

        current_position = Notation::to_position(fen);
    }
    else
    {
        throw std::exception();
    }

    MoveGenerator move_generator;

    while (input >> token)
    {
        // Verify moves
        MoveList<MoveEntry>& moves = move_generator.get_legal_moves(
            current_position, 1, current_position.is_check());
        bool found = false;
        for (int i = 0; i < moves.size; i++)
        {
            Move move = moves.entries[i].move;
            if (Notation::from_move(move) == token)
            {
                current_position.make_move(move);
                found = true;
                break;
            }
        }

        if (!found)
        {
            throw std::exception();
        }
    }

    // Don't start searching though!
}

void Goldfish::receive_go(std::istringstream& input)
{
    search.stop();

    // We received a start command. Extract all parameters from the
    // command and start the search.
    std::string token;
    input >> token;
    if (token == "depth")
    {
        int search_depth;
        if (input >> search_depth)
        {
            search.new_depth_search(current_position, Depth(search_depth));
        }
        else
        {
            throw std::exception();
        }
    }
    else if (token == "nodes")
    {
        uint64_t search_nodes;
        if (input >> search_nodes)
        {
            search.new_nodes_search(current_position, search_nodes);
        }
    }
    else if (token == "movetime")
    {
        uint64_t search_time;
        if (input >> search_time)
        {
            search.new_time_search(current_position, search_time);
        }
    }
    else if (token == "infinite")
    {
        search.new_infinite_search(current_position);
    }
    else
    {
        uint64_t white_time_left      = 1;
        uint64_t white_time_increment = 0;
        uint64_t black_time_left      = 1;
        uint64_t black_time_increment = 0;
        int      search_moves_toGo    = 40;
        bool     ponder               = false;

        do
        {
            if (token == "wtime")
            {
                if (!(input >> white_time_left))
                {
                    throw std::exception();
                }
            }
            else if (token == "winc")
            {
                if (!(input >> white_time_increment))
                {
                    throw std::exception();
                }
            }
            else if (token == "btime")
            {
                if (!(input >> black_time_left))
                {
                    throw std::exception();
                }
            }
            else if (token == "binc")
            {
                if (!(input >> black_time_increment))
                {
                    throw std::exception();
                }
            }
            else if (token == "movestogo")
            {
                if (!(input >> search_moves_toGo))
                {
                    throw std::exception();
                }
            }
            else if (token == "ponder")
            {
                ponder = true;
            }
        } while (input >> token);

        if (ponder)
        {
            search.new_ponder_search(current_position,
                                     white_time_left,
                                     white_time_increment,
                                     black_time_left,
                                     black_time_increment,
                                     search_moves_toGo);
        }
        else
        {
            search.new_clock_search(current_position,
                                    white_time_left,
                                    white_time_increment,
                                    black_time_left,
                                    black_time_increment,
                                    search_moves_toGo);
        }
    }

    // Go...
    search.start();
    start_time        = std::chrono::system_clock::now();
    status_start_time = start_time;
}

void Goldfish::receive_ponder_hit()
{
    // We received a ponder hit command. Just call ponderhit().
    search.ponderhit();
}

void Goldfish::receive_stop()
{
    // We received a stop command. If a search is running, stop it.
    search.stop();
}

void Goldfish::receive_bench()
{
    auto list          = setup_bench();
    auto num_positions = std::count_if(
        list.begin(), list.end(), [](std::string s) { return s.find("go ") == 0; });
    int      count       = 1;
    uint64_t total_nodes = 0;

    {
        std::istringstream hash_options("name Hash value 128");
        hash_options >> std::skipws;
        receive_setoption(hash_options);
        if ((std::string) UCI::Options["SyzygyPath"] == "<empty>")
        {
            std::istringstream syzygy_options("name SyzygyPath value ../syzygy");
            syzygy_options >> std::skipws;
            receive_setoption(syzygy_options);
        }
    }

    auto start_time = std::chrono::high_resolution_clock::now();
    for (const auto& cmd : list)
    {
        std::string        token;
        std::istringstream is(cmd);
        is >> std::skipws >> token;

        if (token == "go")
        {
            std::cerr << "\nPosition: " << count++ << '/' << num_positions << std::endl;

            receive_go(is);
            search.wait_for_finished();
            total_nodes += search.get_total_nodes();
        }
        else if (token == "position")
            receive_position(is);
        else if (token == "ucinewgame")
            receive_new_game();
    }
    auto   end_time = std::chrono::high_resolution_clock::now();
    double time
        = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time)
              .count()
          + 1;

    std::cerr << "\n==========================="
              << "\nTotal time (ms) : " << time << "\nNodes searched  : " << total_nodes
              << "\nNodes/second    : " << 1000 * total_nodes / time << std::endl;
}

// setoption() is called when engine receives the "setoption" UCI command. The
// function updates the UCI option ("name") to the given value ("value").
void Goldfish::receive_setoption(std::istringstream& input)
{
    std::string token, name, value;

    input >> token;  // Consume "name" token

    // Read option name (can contain spaces)
    while (input >> token && token != "value")
        name += (name.empty() ? "" : " ") + token;

    // Read option value (can contain spaces)
    while (input >> token)
        value += (value.empty() ? "" : " ") + token;

    if (UCI::Options.count(name))
        UCI::Options[name] = value;
    else
        std::cout << "No such option: " << name << std::endl;
}

void Goldfish::send_best_move(Move best_move, Move ponder_move)
{
    std::cout << "bestmove ";

    if (best_move != Move::NO_MOVE)
    {
        std::cout << Notation::from_move(best_move);

        if (ponder_move != Move::NO_MOVE)
        {
            std::cout << " ponder " << Notation::from_move(ponder_move);
        }
    }
    else
    {
        std::cout << "NO_MOVE";
    }

    std::cout << std::endl;
}

void Goldfish::send_status(int      current_depth,
                           int      current_max_depth,
                           uint64_t total_nodes,
                           uint64_t tb_hits,
                           Move     current_move,
                           int      current_move_number)
{
    if (std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - status_start_time)
            .count()
        >= 1000)
    {
        send_status(false,
                    current_depth,
                    current_max_depth,
                    total_nodes,
                    tb_hits,
                    current_move,
                    current_move_number);
    }
}

void Goldfish::send_status(bool     force,
                           int      current_depth,
                           int      current_max_depth,
                           uint64_t total_nodes,
                           uint64_t tb_hits,
                           Move     current_move,
                           int      current_move_number)
{
    auto time_delta = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now() - start_time);

    if (force || time_delta.count() >= 1000)
    {
        std::cout << "info";
        std::cout << " depth " << current_depth;
        std::cout << " seldepth " << current_max_depth;
        std::cout << " nodes " << total_nodes;
        std::cout << " time " << time_delta.count();
        std::cout << " nps "
                  << (time_delta.count() >= 1000
                          ? (total_nodes * 1000) / time_delta.count()
                          : 0);
        std::cout << " tbhits " << tb_hits;

        if (current_move != Move::NO_MOVE)
        {
            std::cout << " currmove " << Notation::from_move(current_move);
            std::cout << " currmovenumber " << current_move_number;
        }

        std::cout << std::endl;

        status_start_time = std::chrono::system_clock::now();
    }
}

void Goldfish::send_move(const RootEntry& entry,
                         int              current_depth,
                         int              current_max_depth,
                         uint64_t         total_nodes,
                         uint64_t         tb_hits)
{
    auto time_delta = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now() - start_time);

    std::cout << "info";
    std::cout << " depth " << current_depth;
    std::cout << " seldepth " << current_max_depth;
    std::cout << " nodes " << total_nodes;
    std::cout << " time " << time_delta.count();
    std::cout << " nps "
              << (time_delta.count() >= 1000 ? (total_nodes * 1000) / time_delta.count()
                                             : 0);
    std::cout << " tbhits " << tb_hits;

    if (std::abs(entry.value) >= Value::CHECKMATE_THRESHOLD)
    {
        // Calculate mate distance
        int mate_depth = Value::CHECKMATE - std::abs(entry.value);
        std::cout << " score mate "
                  << ((entry.value > 0) - (entry.value < 0)) * (mate_depth + 1) / 2;
    }
    else
    {
        std::cout << " score cp " << entry.value;
    }

    if (entry.pv.size > 0)
    {
        std::cout << " pv";
        for (int i = 0; i < entry.pv.size; i++)
        {
            std::cout << " " << Notation::from_move(entry.pv.moves[i]);
        }
    }

    std::cout << std::endl;

    status_start_time = std::chrono::system_clock::now();
}

}  // namespace goldfish
