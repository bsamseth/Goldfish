#include "bitboard.hpp"

#include "square.hpp"

namespace goldfish::Bitboard
{
/// Bitboards::pretty() returns an ASCII representation of a bitboard suitable
/// to be printed to standard output. Useful for debugging.
const std::string pretty(U64 b)
{
    std::string s = "+---+---+---+---+---+---+---+---+\n";

    for (Rank r = Rank::RANK_8; r >= Rank::RANK_1; --r)
    {
        for (File f = File::FILE_A; f <= File::FILE_H; ++f)
            s += b & (1ULL << (8 * r + f)) ? "| X " : "|   ";

        s += "|\n+---+---+---+---+---+---+---+---+\n";
    }

    return s;
}
}  // namespace goldfish::Bitboard
