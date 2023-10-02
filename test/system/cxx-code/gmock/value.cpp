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

#include "../src/value.cpp"
#include "value.inc"

TEST_F(CCMockFixture, Run001)
{
    testing::Sequence s1;

    auto result = detail::value();

    EXPECT_CALL(detail.value, op_equal(testing::_))
        .InSequence(s1)
        .WillOnce(testing::ReturnRef(result));

    EXPECT_CALL(detail.value, op_plus_plus())
        .InSequence(s1)
        .WillOnce(testing::Return(result));
    EXPECT_CALL(detail.value, op_plus_plus(testing::_))
        .InSequence(s1)
        .WillOnce(testing::Return(result));

    EXPECT_CALL(detail.value, op_call()).Times(2).InSequence(s1);

    run();
}
