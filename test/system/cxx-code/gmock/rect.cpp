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
#include "../src/rect.cpp"
#include "rect.inc"

TEST_F(CCMockFixture, Run001)
{
    /* 
     * We need our own rectangle instance which can be used as a return
     * value for the overloaded assignment operator.
     */
    EXPECT_CALL(rect, constructor(0, 0, 0, 0));
    auto r1 = ::rect(0, 0, 0, 0);

    EXPECT_CALL(rect, constructor(0, 0, 800, 600));
    EXPECT_CALL(rect, constructor(0, 0, 600, 480));
    /* Default constructor is implicitly defined by "= default;" */

    EXPECT_CALL(rect, op_equal(
                testing::Matcher<const class rect &>(testing::_)))
        .WillOnce(testing::ReturnRef(r1));

    EXPECT_CALL(rect, op_equal(
                testing::Matcher<class rect &&>(testing::_)))
        .WillOnce(testing::ReturnRef(r1));

    EXPECT_CALL(util, 
                dispatch(testing::Matcher<const class rect &>(testing::_)))
        .Times(testing::Exactly(3));

    EXPECT_CALL(rect, area())
        .WillOnce(testing::Return(800 * 600));

    EXPECT_CALL(util, dispatch(800 * 600));

    /* Local variable 'r1' will also be destructed. */ 
    EXPECT_CALL(rect, destructor())
        .Times(testing::Exactly(4));

    run();
}
