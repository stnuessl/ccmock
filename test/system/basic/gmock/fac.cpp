/*
 * Copyright (C) 2022   Steffen Nuessle
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

#ifdef __cplusplus
extern "C" {
#endif

#include "fac.h"

#ifdef __cplusplus
}
#endif

#include "fac.inc"

TEST(fac, basic)
{
    ASSERT_EQ(1, fac(0));
    ASSERT_EQ(1, fac(1));

    EXPECT_CALL(mock, mul(testing::_, testing::_))
        .Times(testing::Exactly(2))
        .WillRepeatedly(testing::Invoke([](int a, int b) { return a * b; }));

    ASSERT_EQ(6, fac(3));

    EXPECT_CALL(mock, mul(testing::_, testing::_))
        .Times(testing::Exactly(3))
        .WillRepeatedly(testing::Invoke([](int a, int b) { return a * b; }));

    ASSERT_EQ(24, fac(4));
}
