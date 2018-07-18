#include "movegenerator.hpp"
#include "notation.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

struct Entry {
    int depth;
    uint64_t nodes;

    Entry(int depth, uint64_t nodes) : depth(depth), nodes(nodes) {}
};

struct P {
    std::string fen;
    std::vector<Entry> perft_entries;

    P(std::string fen, std::initializer_list<Entry> perft_entries) : fen(fen), perft_entries(perft_entries) {}
};

const std::vector <P> perft_positions = {
        P("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
          {Entry(1, 20), Entry(2, 400), Entry(3, 8902), Entry(4, 197281), Entry(5, 4865609), Entry(6, 119060324)}),
        P("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
          {Entry(1, 48), Entry(2, 2039), Entry(3, 97862), Entry(4, 4085603), Entry(5, 193690690)}),
        P("4k3/8/8/8/8/8/8/4K2R w K - 0 1", {Entry(1, 15), Entry(2, 66), Entry(3, 1197), Entry(4, 7059), Entry(5, 133987), Entry(6, 764643)}),
        P("4k3/8/8/8/8/8/8/R3K3 w Q - 0 1", {Entry(1, 16), Entry(2, 71), Entry(3, 1287), Entry(4, 7626), Entry(5, 145232), Entry(6, 846648)}),
        P("4k2r/8/8/8/8/8/8/4K3 w k - 0 1", {Entry(1, 5), Entry(2, 75), Entry(3, 459), Entry(4, 8290), Entry(5, 47635), Entry(6, 899442)}),
        P("r3k3/8/8/8/8/8/8/4K3 w q - 0 1", {Entry(1, 5), Entry(2, 80), Entry(3, 493), Entry(4, 8897), Entry(5, 52710), Entry(6, 1001523)}),
        P("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1",
          {Entry(1, 26), Entry(2, 112), Entry(3, 3189), Entry(4, 17945), Entry(5, 532933), Entry(6, 2788982)}),
        P("r3k2r/8/8/8/8/8/8/4K3 w kq - 0 1",
          {Entry(1, 5), Entry(2, 130), Entry(3, 782), Entry(4, 22180), Entry(5, 118882), Entry(6, 3517770)}),
        P("8/8/8/8/8/8/6k1/4K2R w K - 0 1", {Entry(1, 12), Entry(2, 38), Entry(3, 564), Entry(4, 2219), Entry(5, 37735), Entry(6, 185867)}),
        P("8/8/8/8/8/8/1k6/R3K3 w Q - 0 1", {Entry(1, 15), Entry(2, 65), Entry(3, 1018), Entry(4, 4573), Entry(5, 80619), Entry(6, 413018)}),
        P("4k2r/6K1/8/8/8/8/8/8 w k - 0 1", {Entry(1, 3), Entry(2, 32), Entry(3, 134), Entry(4, 2073), Entry(5, 10485), Entry(6, 179869)}),
        P("r3k3/1K6/8/8/8/8/8/8 w q - 0 1", {Entry(1, 4), Entry(2, 49), Entry(3, 243), Entry(4, 3991), Entry(5, 20780), Entry(6, 367724)}),
        P("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
          {Entry(1, 26), Entry(2, 568), Entry(3, 13744), Entry(4, 314346), Entry(5, 7594526), Entry(6, 179862938)}),
        P("r3k2r/8/8/8/8/8/8/1R2K2R w Kkq - 0 1",
          {Entry(1, 25), Entry(2, 567), Entry(3, 14095), Entry(4, 328965), Entry(5, 8153719), Entry(6, 195629489)}),
        P("r3k2r/8/8/8/8/8/8/2R1K2R w Kkq - 0 1",
          {Entry(1, 25), Entry(2, 548), Entry(3, 13502), Entry(4, 312835), Entry(5, 7736373), Entry(6, 184411439)}),
        P("r3k2r/8/8/8/8/8/8/R3K1R1 w Qkq - 0 1",
          {Entry(1, 25), Entry(2, 547), Entry(3, 13579), Entry(4, 316214), Entry(5, 7878456), Entry(6, 189224276)}),
        P("1r2k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1",
          {Entry(1, 26), Entry(2, 583), Entry(3, 14252), Entry(4, 334705), Entry(5, 8198901), Entry(6, 198328929)}),
        P("2r1k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1",
          {Entry(1, 25), Entry(2, 560), Entry(3, 13592), Entry(4, 317324), Entry(5, 7710115), Entry(6, 185959088)}),
        P("r3k1r1/8/8/8/8/8/8/R3K2R w KQq - 0 1",
          {Entry(1, 25), Entry(2, 560), Entry(3, 13607), Entry(4, 320792), Entry(5, 7848606), Entry(6, 190755813)}),
        P("4k3/8/8/8/8/8/8/4K2R b K - 0 1", {Entry(1, 5), Entry(2, 75), Entry(3, 459), Entry(4, 8290), Entry(5, 47635), Entry(6, 899442)}),
        P("4k3/8/8/8/8/8/8/R3K3 b Q - 0 1", {Entry(1, 5), Entry(2, 80), Entry(3, 493), Entry(4, 8897), Entry(5, 52710), Entry(6, 1001523)}),
        P("4k2r/8/8/8/8/8/8/4K3 b k - 0 1", {Entry(1, 15), Entry(2, 66), Entry(3, 1197), Entry(4, 7059), Entry(5, 133987), Entry(6, 764643)}),
        P("r3k3/8/8/8/8/8/8/4K3 b q - 0 1", {Entry(1, 16), Entry(2, 71), Entry(3, 1287), Entry(4, 7626), Entry(5, 145232), Entry(6, 846648)}),
        P("4k3/8/8/8/8/8/8/R3K2R b KQ - 0 1",
          {Entry(1, 5), Entry(2, 130), Entry(3, 782), Entry(4, 22180), Entry(5, 118882), Entry(6, 3517770)}),
        P("r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1",
          {Entry(1, 26), Entry(2, 112), Entry(3, 3189), Entry(4, 17945), Entry(5, 532933), Entry(6, 2788982)}),
        P("8/8/8/8/8/8/6k1/4K2R b K - 0 1", {Entry(1, 3), Entry(2, 32), Entry(3, 134), Entry(4, 2073), Entry(5, 10485), Entry(6, 179869)}),
        P("8/8/8/8/8/8/1k6/R3K3 b Q - 0 1", {Entry(1, 4), Entry(2, 49), Entry(3, 243), Entry(4, 3991), Entry(5, 20780), Entry(6, 367724)}),
        P("4k2r/6K1/8/8/8/8/8/8 b k - 0 1", {Entry(1, 12), Entry(2, 38), Entry(3, 564), Entry(4, 2219), Entry(5, 37735), Entry(6, 185867)}),
        P("r3k3/1K6/8/8/8/8/8/8 b q - 0 1", {Entry(1, 15), Entry(2, 65), Entry(3, 1018), Entry(4, 4573), Entry(5, 80619), Entry(6, 413018)}),
        P("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1",
          {Entry(1, 26), Entry(2, 568), Entry(3, 13744), Entry(4, 314346), Entry(5, 7594526), Entry(6, 179862938)}),
        P("r3k2r/8/8/8/8/8/8/1R2K2R b Kkq - 0 1",
          {Entry(1, 26), Entry(2, 583), Entry(3, 14252), Entry(4, 334705), Entry(5, 8198901), Entry(6, 198328929)}),
        P("r3k2r/8/8/8/8/8/8/2R1K2R b Kkq - 0 1",
          {Entry(1, 25), Entry(2, 560), Entry(3, 13592), Entry(4, 317324), Entry(5, 7710115), Entry(6, 185959088)}),
        P("r3k2r/8/8/8/8/8/8/R3K1R1 b Qkq - 0 1",
          {Entry(1, 25), Entry(2, 560), Entry(3, 13607), Entry(4, 320792), Entry(5, 7848606), Entry(6, 190755813)}),
        P("1r2k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1",
          {Entry(1, 25), Entry(2, 567), Entry(3, 14095), Entry(4, 328965), Entry(5, 8153719), Entry(6, 195629489)}),
        P("2r1k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1",
          {Entry(1, 25), Entry(2, 548), Entry(3, 13502), Entry(4, 312835), Entry(5, 7736373), Entry(6, 184411439)}),
        P("r3k1r1/8/8/8/8/8/8/R3K2R b KQq - 0 1",
          {Entry(1, 25), Entry(2, 547), Entry(3, 13579), Entry(4, 316214), Entry(5, 7878456), Entry(6, 189224276)}),
        P("8/1n4N1/2k5/8/8/5K2/1N4n1/8 w - - 0 1",
          {Entry(1, 14), Entry(2, 195), Entry(3, 2760), Entry(4, 38675), Entry(5, 570726), Entry(6, 8107539)}),
        P("8/1k6/8/5N2/8/4n3/8/2K5 w - - 0 1",
          {Entry(1, 11), Entry(2, 156), Entry(3, 1636), Entry(4, 20534), Entry(5, 223507), Entry(6, 2594412)}),
        P("8/8/4k3/3Nn3/3nN3/4K3/8/8 w - - 0 1",
          {Entry(1, 19), Entry(2, 289), Entry(3, 4442), Entry(4, 73584), Entry(5, 1198299), Entry(6, 19870403)}),
        P("K7/8/2n5/1n6/8/8/8/k6N w - - 0 1", {Entry(1, 3), Entry(2, 51), Entry(3, 345), Entry(4, 5301), Entry(5, 38348), Entry(6, 588695)}),
        P("k7/8/2N5/1N6/8/8/8/K6n w - - 0 1", {Entry(1, 17), Entry(2, 54), Entry(3, 835), Entry(4, 5910), Entry(5, 92250), Entry(6, 688780)}),
        P("8/1n4N1/2k5/8/8/5K2/1N4n1/8 b - - 0 1",
          {Entry(1, 15), Entry(2, 193), Entry(3, 2816), Entry(4, 40039), Entry(5, 582642), Entry(6, 8503277)}),
        P("8/1k6/8/5N2/8/4n3/8/2K5 b - - 0 1",
          {Entry(1, 16), Entry(2, 180), Entry(3, 2290), Entry(4, 24640), Entry(5, 288141), Entry(6, 3147566)}),
        P("8/8/3K4/3Nn3/3nN3/4k3/8/8 b - - 0 1",
          {Entry(1, 4), Entry(2, 68), Entry(3, 1118), Entry(4, 16199), Entry(5, 281190), Entry(6, 4405103)}),
        P("K7/8/2n5/1n6/8/8/8/k6N b - - 0 1", {Entry(1, 17), Entry(2, 54), Entry(3, 835), Entry(4, 5910), Entry(5, 92250), Entry(6, 688780)}),
        P("k7/8/2N5/1N6/8/8/8/K6n b - - 0 1", {Entry(1, 3), Entry(2, 51), Entry(3, 345), Entry(4, 5301), Entry(5, 38348), Entry(6, 588695)}),
        P("B6b/8/8/8/2K5/4k3/8/b6B w - - 0 1",
          {Entry(1, 17), Entry(2, 278), Entry(3, 4607), Entry(4, 76778), Entry(5, 1320507), Entry(6, 22823890)}),
        P("8/8/1B6/7b/7k/8/2B1b3/7K w - - 0 1",
          {Entry(1, 21), Entry(2, 316), Entry(3, 5744), Entry(4, 93338), Entry(5, 1713368), Entry(6, 28861171)}),
        P("k7/B7/1B6/1B6/8/8/8/K6b w - - 0 1",
          {Entry(1, 21), Entry(2, 144), Entry(3, 3242), Entry(4, 32955), Entry(5, 787524), Entry(6, 7881673)}),
        P("K7/b7/1b6/1b6/8/8/8/k6B w - - 0 1",
          {Entry(1, 7), Entry(2, 143), Entry(3, 1416), Entry(4, 31787), Entry(5, 310862), Entry(6, 7382896)}),
        P("B6b/8/8/8/2K5/5k2/8/b6B b - - 0 1",
          {Entry(1, 6), Entry(2, 106), Entry(3, 1829), Entry(4, 31151), Entry(5, 530585), Entry(6, 9250746)}),
        P("8/8/1B6/7b/7k/8/2B1b3/7K b - - 0 1",
          {Entry(1, 17), Entry(2, 309), Entry(3, 5133), Entry(4, 93603), Entry(5, 1591064), Entry(6, 29027891)}),
        P("k7/B7/1B6/1B6/8/8/8/K6b b - - 0 1",
          {Entry(1, 7), Entry(2, 143), Entry(3, 1416), Entry(4, 31787), Entry(5, 310862), Entry(6, 7382896)}),
        P("K7/b7/1b6/1b6/8/8/8/k6B b - - 0 1",
          {Entry(1, 21), Entry(2, 144), Entry(3, 3242), Entry(4, 32955), Entry(5, 787524), Entry(6, 7881673)}),
        P("7k/RR6/8/8/8/8/rr6/7K w - - 0 1",
          {Entry(1, 19), Entry(2, 275), Entry(3, 5300), Entry(4, 104342), Entry(5, 2161211), Entry(6, 44956585)}),
        P("R6r/8/8/2K5/5k2/8/8/r6R w - - 0 1",
          {Entry(1, 36), Entry(2, 1027), Entry(3, 29215), Entry(4, 771461), Entry(5, 20506480), Entry(6, 525169084)}),
        P("7k/RR6/8/8/8/8/rr6/7K b - - 0 1",
          {Entry(1, 19), Entry(2, 275), Entry(3, 5300), Entry(4, 104342), Entry(5, 2161211), Entry(6, 44956585)}),
        P("R6r/8/8/2K5/5k2/8/8/r6R b - - 0 1",
          {Entry(1, 36), Entry(2, 1027), Entry(3, 29227), Entry(4, 771368), Entry(5, 20521342), Entry(6, 524966748)}),
        P("6kq/8/8/8/8/8/8/7K w - - 0 1", {Entry(1, 2), Entry(2, 36), Entry(3, 143), Entry(4, 3637), Entry(5, 14893), Entry(6, 391507)}),
        P("K7/8/8/3Q4/4q3/8/8/7k w - - 0 1", {Entry(1, 6), Entry(2, 35), Entry(3, 495), Entry(4, 8349), Entry(5, 166741), Entry(6, 3370175)}),
        P("6qk/8/8/8/8/8/8/7K b - - 0 1", {Entry(1, 22), Entry(2, 43), Entry(3, 1015), Entry(4, 4167), Entry(5, 105749), Entry(6, 419369)}),
        P("6KQ/8/8/8/8/8/8/7k b - - 0 1", {Entry(1, 2), Entry(2, 36), Entry(3, 143), Entry(4, 3637), Entry(5, 14893), Entry(6, 391507)}),
        P("K7/8/8/3Q4/4q3/8/8/7k b - - 0 1", {Entry(1, 6), Entry(2, 35), Entry(3, 495), Entry(4, 8349), Entry(5, 166741), Entry(6, 3370175)}),
        P("8/8/8/8/8/K7/P7/k7 w - - 0 1", {Entry(1, 3), Entry(2, 7), Entry(3, 43), Entry(4, 199), Entry(5, 1347), Entry(6, 6249)}),
        P("8/8/8/8/8/7K/7P/7k w - - 0 1", {Entry(1, 3), Entry(2, 7), Entry(3, 43), Entry(4, 199), Entry(5, 1347), Entry(6, 6249)}),
        P("K7/p7/k7/8/8/8/8/8 w - - 0 1", {Entry(1, 1), Entry(2, 3), Entry(3, 12), Entry(4, 80), Entry(5, 342), Entry(6, 2343)}),
        P("7K/7p/7k/8/8/8/8/8 w - - 0 1", {Entry(1, 1), Entry(2, 3), Entry(3, 12), Entry(4, 80), Entry(5, 342), Entry(6, 2343)}),
        P("8/2k1p3/3pP3/3P2K1/8/8/8/8 w - - 0 1", {Entry(1, 7), Entry(2, 35), Entry(3, 210), Entry(4, 1091), Entry(5, 7028), Entry(6, 34834)}),
        P("8/8/8/8/8/K7/P7/k7 b - - 0 1", {Entry(1, 1), Entry(2, 3), Entry(3, 12), Entry(4, 80), Entry(5, 342), Entry(6, 2343)}),
        P("8/8/8/8/8/7K/7P/7k b - - 0 1", {Entry(1, 1), Entry(2, 3), Entry(3, 12), Entry(4, 80), Entry(5, 342), Entry(6, 2343)}),
        P("K7/p7/k7/8/8/8/8/8 b - - 0 1", {Entry(1, 3), Entry(2, 7), Entry(3, 43), Entry(4, 199), Entry(5, 1347), Entry(6, 6249)}),
        P("7K/7p/7k/8/8/8/8/8 b - - 0 1", {Entry(1, 3), Entry(2, 7), Entry(3, 43), Entry(4, 199), Entry(5, 1347), Entry(6, 6249)}),
        P("8/2k1p3/3pP3/3P2K1/8/8/8/8 b - - 0 1", {Entry(1, 5), Entry(2, 35), Entry(3, 182), Entry(4, 1091), Entry(5, 5408), Entry(6, 34822)}),
        P("8/8/8/8/8/4k3/4P3/4K3 w - - 0 1", {Entry(1, 2), Entry(2, 8), Entry(3, 44), Entry(4, 282), Entry(5, 1814), Entry(6, 11848)}),
        P("4k3/4p3/4K3/8/8/8/8/8 b - - 0 1", {Entry(1, 2), Entry(2, 8), Entry(3, 44), Entry(4, 282), Entry(5, 1814), Entry(6, 11848)}),
        P("8/8/7k/7p/7P/7K/8/8 w - - 0 1", {Entry(1, 3), Entry(2, 9), Entry(3, 57), Entry(4, 360), Entry(5, 1969), Entry(6, 10724)}),
        P("8/8/k7/p7/P7/K7/8/8 w - - 0 1", {Entry(1, 3), Entry(2, 9), Entry(3, 57), Entry(4, 360), Entry(5, 1969), Entry(6, 10724)}),
        P("8/8/3k4/3p4/3P4/3K4/8/8 w - - 0 1", {Entry(1, 5), Entry(2, 25), Entry(3, 180), Entry(4, 1294), Entry(5, 8296), Entry(6, 53138)}),
        P("8/3k4/3p4/8/3P4/3K4/8/8 w - - 0 1", {Entry(1, 8), Entry(2, 61), Entry(3, 483), Entry(4, 3213), Entry(5, 23599), Entry(6, 157093)}),
        P("8/8/3k4/3p4/8/3P4/3K4/8 w - - 0 1", {Entry(1, 8), Entry(2, 61), Entry(3, 411), Entry(4, 3213), Entry(5, 21637), Entry(6, 158065)}),
        P("k7/8/3p4/8/3P4/8/8/7K w - - 0 1", {Entry(1, 4), Entry(2, 15), Entry(3, 90), Entry(4, 534), Entry(5, 3450), Entry(6, 20960)}),
        P("8/8/7k/7p/7P/7K/8/8 b - - 0 1", {Entry(1, 3), Entry(2, 9), Entry(3, 57), Entry(4, 360), Entry(5, 1969), Entry(6, 10724)}),
        P("8/8/k7/p7/P7/K7/8/8 b - - 0 1", {Entry(1, 3), Entry(2, 9), Entry(3, 57), Entry(4, 360), Entry(5, 1969), Entry(6, 10724)}),
        P("8/8/3k4/3p4/3P4/3K4/8/8 b - - 0 1", {Entry(1, 5), Entry(2, 25), Entry(3, 180), Entry(4, 1294), Entry(5, 8296), Entry(6, 53138)}),
        P("8/3k4/3p4/8/3P4/3K4/8/8 b - - 0 1", {Entry(1, 8), Entry(2, 61), Entry(3, 411), Entry(4, 3213), Entry(5, 21637), Entry(6, 158065)}),
        P("8/8/3k4/3p4/8/3P4/3K4/8 b - - 0 1", {Entry(1, 8), Entry(2, 61), Entry(3, 483), Entry(4, 3213), Entry(5, 23599), Entry(6, 157093)}),
        P("k7/8/3p4/8/3P4/8/8/7K b - - 0 1", {Entry(1, 4), Entry(2, 15), Entry(3, 89), Entry(4, 537), Entry(5, 3309), Entry(6, 21104)}),
        P("7k/3p4/8/8/3P4/8/8/K7 w - - 0 1", {Entry(1, 4), Entry(2, 19), Entry(3, 117), Entry(4, 720), Entry(5, 4661), Entry(6, 32191)}),
        P("7k/8/8/3p4/8/8/3P4/K7 w - - 0 1", {Entry(1, 5), Entry(2, 19), Entry(3, 116), Entry(4, 716), Entry(5, 4786), Entry(6, 30980)}),
        P("k7/8/8/7p/6P1/8/8/K7 w - - 0 1", {Entry(1, 5), Entry(2, 22), Entry(3, 139), Entry(4, 877), Entry(5, 6112), Entry(6, 41874)}),
        P("k7/8/7p/8/8/6P1/8/K7 w - - 0 1", {Entry(1, 4), Entry(2, 16), Entry(3, 101), Entry(4, 637), Entry(5, 4354), Entry(6, 29679)}),
        P("k7/8/8/6p1/7P/8/8/K7 w - - 0 1", {Entry(1, 5), Entry(2, 22), Entry(3, 139), Entry(4, 877), Entry(5, 6112), Entry(6, 41874)}),
        P("k7/8/6p1/8/8/7P/8/K7 w - - 0 1", {Entry(1, 4), Entry(2, 16), Entry(3, 101), Entry(4, 637), Entry(5, 4354), Entry(6, 29679)}),
        P("k7/8/8/3p4/4p3/8/8/7K w - - 0 1", {Entry(1, 3), Entry(2, 15), Entry(3, 84), Entry(4, 573), Entry(5, 3013), Entry(6, 22886)}),
        P("k7/8/3p4/8/8/4P3/8/7K w - - 0 1", {Entry(1, 4), Entry(2, 16), Entry(3, 101), Entry(4, 637), Entry(5, 4271), Entry(6, 28662)}),
        P("7k/3p4/8/8/3P4/8/8/K7 b - - 0 1", {Entry(1, 5), Entry(2, 19), Entry(3, 117), Entry(4, 720), Entry(5, 5014), Entry(6, 32167)}),
        P("7k/8/8/3p4/8/8/3P4/K7 b - - 0 1", {Entry(1, 4), Entry(2, 19), Entry(3, 117), Entry(4, 712), Entry(5, 4658), Entry(6, 30749)}),
        P("k7/8/8/7p/6P1/8/8/K7 b - - 0 1", {Entry(1, 5), Entry(2, 22), Entry(3, 139), Entry(4, 877), Entry(5, 6112), Entry(6, 41874)}),
        P("k7/8/7p/8/8/6P1/8/K7 b - - 0 1", {Entry(1, 4), Entry(2, 16), Entry(3, 101), Entry(4, 637), Entry(5, 4354), Entry(6, 29679)}),
        P("k7/8/8/6p1/7P/8/8/K7 b - - 0 1", {Entry(1, 5), Entry(2, 22), Entry(3, 139), Entry(4, 877), Entry(5, 6112), Entry(6, 41874)}),
        P("k7/8/6p1/8/8/7P/8/K7 b - - 0 1", {Entry(1, 4), Entry(2, 16), Entry(3, 101), Entry(4, 637), Entry(5, 4354), Entry(6, 29679)}),
        P("k7/8/8/3p4/4p3/8/8/7K b - - 0 1", {Entry(1, 5), Entry(2, 15), Entry(3, 102), Entry(4, 569), Entry(5, 4337), Entry(6, 22579)}),
        P("k7/8/3p4/8/8/4P3/8/7K b - - 0 1", {Entry(1, 4), Entry(2, 16), Entry(3, 101), Entry(4, 637), Entry(5, 4271), Entry(6, 28662)}),
        P("7k/8/8/p7/1P6/8/8/7K w - - 0 1", {Entry(1, 5), Entry(2, 22), Entry(3, 139), Entry(4, 877), Entry(5, 6112), Entry(6, 41874)}),
        P("7k/8/p7/8/8/1P6/8/7K w - - 0 1", {Entry(1, 4), Entry(2, 16), Entry(3, 101), Entry(4, 637), Entry(5, 4354), Entry(6, 29679)}),
        P("7k/8/8/1p6/P7/8/8/7K w - - 0 1", {Entry(1, 5), Entry(2, 22), Entry(3, 139), Entry(4, 877), Entry(5, 6112), Entry(6, 41874)}),
        P("7k/8/1p6/8/8/P7/8/7K w - - 0 1", {Entry(1, 4), Entry(2, 16), Entry(3, 101), Entry(4, 637), Entry(5, 4354), Entry(6, 29679)}),
        P("k7/7p/8/8/8/8/6P1/K7 w - - 0 1", {Entry(1, 5), Entry(2, 25), Entry(3, 161), Entry(4, 1035), Entry(5, 7574), Entry(6, 55338)}),
        P("k7/6p1/8/8/8/8/7P/K7 w - - 0 1", {Entry(1, 5), Entry(2, 25), Entry(3, 161), Entry(4, 1035), Entry(5, 7574), Entry(6, 55338)}),
        P("3k4/3pp3/8/8/8/8/3PP3/3K4 w - - 0 1", {Entry(1, 7), Entry(2, 49), Entry(3, 378), Entry(4, 2902), Entry(5, 24122), Entry(6, 199002)}),
        P("7k/8/8/p7/1P6/8/8/7K b - - 0 1", {Entry(1, 5), Entry(2, 22), Entry(3, 139), Entry(4, 877), Entry(5, 6112), Entry(6, 41874)}),
        P("7k/8/p7/8/8/1P6/8/7K b - - 0 1", {Entry(1, 4), Entry(2, 16), Entry(3, 101), Entry(4, 637), Entry(5, 4354), Entry(6, 29679)}),
        P("7k/8/8/1p6/P7/8/8/7K b - - 0 1", {Entry(1, 5), Entry(2, 22), Entry(3, 139), Entry(4, 877), Entry(5, 6112), Entry(6, 41874)}),
        P("7k/8/1p6/8/8/P7/8/7K b - - 0 1", {Entry(1, 4), Entry(2, 16), Entry(3, 101), Entry(4, 637), Entry(5, 4354), Entry(6, 29679)}),
        P("k7/7p/8/8/8/8/6P1/K7 b - - 0 1", {Entry(1, 5), Entry(2, 25), Entry(3, 161), Entry(4, 1035), Entry(5, 7574), Entry(6, 55338)}),
        P("k7/6p1/8/8/8/8/7P/K7 b - - 0 1", {Entry(1, 5), Entry(2, 25), Entry(3, 161), Entry(4, 1035), Entry(5, 7574), Entry(6, 55338)}),
        P("3k4/3pp3/8/8/8/8/3PP3/3K4 b - - 0 1", {Entry(1, 7), Entry(2, 49), Entry(3, 378), Entry(4, 2902), Entry(5, 24122), Entry(6, 199002)}),
        P("8/Pk6/8/8/8/8/6Kp/8 w - - 0 1", {Entry(1, 11), Entry(2, 97), Entry(3, 887), Entry(4, 8048), Entry(5, 90606), Entry(6, 1030499)}),
        P("n1n5/1Pk5/8/8/8/8/5Kp1/5N1N w - - 0 1",
          {Entry(1, 24), Entry(2, 421), Entry(3, 7421), Entry(4, 124608), Entry(5, 2193768), Entry(6, 37665329)}),
        P("8/PPPk4/8/8/8/8/4Kppp/8 w - - 0 1",
          {Entry(1, 18), Entry(2, 270), Entry(3, 4699), Entry(4, 79355), Entry(5, 1533145), Entry(6, 28859283)}),
        P("n1n5/PPPk4/8/8/8/8/4Kppp/5N1N w - - 0 1",
          {Entry(1, 24), Entry(2, 496), Entry(3, 9483), Entry(4, 182838), Entry(5, 3605103), Entry(6, 71179139)}),
        P("8/Pk6/8/8/8/8/6Kp/8 b - - 0 1", {Entry(1, 11), Entry(2, 97), Entry(3, 887), Entry(4, 8048), Entry(5, 90606), Entry(6, 1030499)}),
        P("n1n5/1Pk5/8/8/8/8/5Kp1/5N1N b - - 0 1",
          {Entry(1, 24), Entry(2, 421), Entry(3, 7421), Entry(4, 124608), Entry(5, 2193768), Entry(6, 37665329)}),
        P("8/PPPk4/8/8/8/8/4Kppp/8 b - - 0 1",
          {Entry(1, 18), Entry(2, 270), Entry(3, 4699), Entry(4, 79355), Entry(5, 1533145), Entry(6, 28859283)}),
        P("n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1",
          {Entry(1, 24), Entry(2, 496), Entry(3, 9483), Entry(4, 182838), Entry(5, 3605103), Entry(6, 71179139)}),
        P("1k6/8/8/5pP1/4K1P1/8/8/8 w - f6 0 1", {Entry(1, 10), Entry(2, 63), Entry(3, 533), Entry(4, 3508), Entry(5, 30821)})
};

constexpr int MAX_DEPTH = 6;
std::array <MoveGenerator, MAX_DEPTH> move_generators;

uint64_t mini_max(int depth, Position &position, int ply) {
    if (depth <= 0) {
        return 1;
    }

    uint64_t total_nodes = 0;

    bool is_check = position.is_check();
    MoveList <MoveEntry> &moves = move_generators[ply].get_moves(position, depth, is_check);

    for (int i = 0; i < moves.size; i++) {
        Move move = moves.entries[i]->move;

        position.make_move(move);
        if (!position.is_check(~position.active_color)) {
            total_nodes += mini_max(depth - 1, position, ply + 1);
        }
        position.undo_move(move);
    }

    return total_nodes;
}

TEST(movegeneratortest, test_perft) {
    for (unsigned int i = 0; i < 4; i++) {
        for (const auto &p : perft_positions) {
            if (p.perft_entries.size() > i) {
                int depth = p.perft_entries[i].depth;
                uint64_t nodes = p.perft_entries[i].nodes;

                Position position(Notation::to_position(p.fen));

                uint64_t result = mini_max(depth, position, 0);
                EXPECT_EQ(nodes, result) << Notation::from_position(position)
                                         << ", depth " << i
                                         << ": expected " << nodes
                                         << ", actual " << result;
            }
        }
    }
}
