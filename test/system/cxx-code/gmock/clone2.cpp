/*
 * Copyright (C) 2023   Steffen Nuessle
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* NOLINTNEXTLINE(bugprone-suspicious-include) */
#include "../src/clone2.cpp"
#include "clone2.inc"

TEST_F(CCMockFixture, Run001)
{
    testing::Sequence s1;

    EXPECT_CALL(n1.n2.c2, run())
        .InSequence(s1);
    EXPECT_CALL(n1.n2.c1, run())
        .InSequence(s1);
    EXPECT_CALL(n1.n2.c2.c1, run())
        .InSequence(s1);

    run();
}

