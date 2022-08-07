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

#include <gtest/gtest.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "../src/zbuild.h"
#include "../src/zlib-ng.h"

#ifdef __cplusplus
}
#endif

#include "uncompr.inc"

TEST(uncompress, 001)
{
    const unsigned char src[] = {0xff};
    unsigned char dst[1];
    z_size_t size = std::size(dst);

    /* There is no way to work around the ugly "__" ... */
    PREFIX(inflateInit__fake).return_val = Z_OK;
    PREFIX(inflate_fake).return_val = Z_STREAM_END;

    ASSERT_EQ(Z_OK, PREFIX(uncompress)(dst, &size, src, std::size(src)));

    ASSERT_EQ(1, PREFIX(inflateInit__fake).call_count);
    ASSERT_EQ(0, strcmp(ZLIBNG_VERSION, PREFIX(inflateInit__fake).arg1_val));
    ASSERT_EQ(sizeof(PREFIX3(stream)), PREFIX(inflateInit__fake).arg2_val);

    ASSERT_EQ(1, PREFIX(inflate_fake).call_count);
    ASSERT_EQ(Z_NO_FLUSH, PREFIX(inflate_fake).arg1_val);

    ASSERT_EQ(1, PREFIX(inflateEnd_fake).call_count);
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
