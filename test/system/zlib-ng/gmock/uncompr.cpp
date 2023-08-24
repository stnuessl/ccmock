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

#include "../src/zbuild.h"
#include "../src/zlib-ng.h"

#ifdef __cplusplus
}
#endif

#include "uncompr.inc"

TEST_F(CCMockFixture, Uncompress001)
{
    const unsigned char src[] = {0xff};
    unsigned char dst[1];
    z_size_t size = std::size(dst);

    EXPECT_CALL(_,
                PREFIX(inflateInit_)(testing::_,
                                     testing::StrEq(ZLIBNG_VERSION),
                                     (int32_t) sizeof(PREFIX3(stream))))
        .WillOnce(testing::Return(Z_OK));

    EXPECT_CALL(_, PREFIX(inflate)(testing::_, Z_NO_FLUSH))
        .WillOnce(testing::Return(Z_STREAM_END));

    EXPECT_CALL(_, PREFIX(inflateEnd)(testing::_));

    ASSERT_EQ(Z_OK, PREFIX(uncompress)(dst, &size, src, std::size(src)));
}
