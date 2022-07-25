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

#include "../src/deflate.h"
#include "../src/functable.h"
#include "../src/zbuild.h"
#include "../src/zlib-ng.h"

void crc_reset(deflate_state *const s);

#ifdef __cplusplus
}
#endif

/* zlib-ng and gtest both define their own "Assert" ... */
#ifdef Assert
#undef Assert
#endif

#include "crc32.inc"

struct functable_s functable;

int x86_cpu_has_pclmulqdq;

TEST(crc_reset, 001)
{
    deflate_state state;
    PREFIX3(stream) strm;

    state.strm = &strm;
    state.strm->adler = ~0;

#ifdef X86_PCLMULQDQ_CRC
    x86_cpu_has_pclmulqdq = 1;

    EXPECT_CALL(mock, x86_check_features()).Times(1);

    EXPECT_CALL(mock, crc_fold_init(&state))
        .WillOnce(testing::Assign(&state.strm->adler, 0));
#endif

    crc_reset(&state);

    ASSERT_EQ(0, state.strm->adler);
}

TEST(crc_reset, 002)
{
    deflate_state state;
    PREFIX3(stream) strm;

    state.strm = &strm;
    state.strm->adler = ~0;

#ifdef X86_PCLMULQDQ_CRC
    x86_cpu_has_pclmulqdq = 0;

    EXPECT_CALL(mock, x86_check_features()).Times(1);
#endif

    crc_reset(&state);

    ASSERT_EQ(0, state.strm->adler);
}
