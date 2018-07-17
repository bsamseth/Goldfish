#include "evaluation.hpp"
#include "notation.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(evaluationtest, test_evaluate) {
    Position position(Notation::to_position(Notation::STANDARDPOSITION));

    EXPECT_EQ(Evaluation::TEMPO, Evaluation::evaluate(position));
}
