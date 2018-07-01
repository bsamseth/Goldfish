/*
 * Copyright (C) 2013-2016 Phokham Nonava
 *
 * Use of this source code is governed by the MIT license that can be
 * found in the LICENSE file.
 */

#include "evaluation.hpp"
#include "notation.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(evaluationtest, test_evaluate
) {
Position position(Notation::to_position(Notation::STANDARDPOSITION));
Evaluation evaluation;

EXPECT_EQ(+Evaluation::TEMPO, evaluation.
evaluate(position)
);
}
