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
#include "../src/globals.cpp"
#include "globals.inc"

/*
 * Explicitly define this global variable due to it missing a default
 * constructor.
 */
class c3 c3(0);

void nop(int arg1, int arg2)
{
    (void) arg1;
    (void) arg2;
}

TEST_F(CCMockFixture, Run001)
{
    n1::f1 = &nop;

    EXPECT_CALL(c1, run());
    EXPECT_CALL(n1.c2, run());
    EXPECT_CALL(c3, run());

    run();
}
