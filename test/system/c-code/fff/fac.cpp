/*
 * Copyright (C) 2023  Steffen Nuessle
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "fac.h"

#ifdef __cplusplus
}
#endif

#include "fac.inc"

int custom_mul(int x, int y)
{
    return x * y;
}

TEST(fac, basic)
{
    ASSERT_EQ(1, fac(0));
    ASSERT_EQ(0, mul_fake.call_count);

    ASSERT_EQ(1, fac(1));
    ASSERT_EQ(0, mul_fake.call_count);

    mul_fake.custom_fake = &custom_mul;
    ASSERT_EQ(6, fac(3));
    ASSERT_EQ(2, mul_fake.call_count);

    FFF_FAKE_LIST(RESET_FAKE);
    mul_fake.custom_fake = &custom_mul;
    ASSERT_EQ(24, fac(4));
    ASSERT_EQ(3, mul_fake.call_count);
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
