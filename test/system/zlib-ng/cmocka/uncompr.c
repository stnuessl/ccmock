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

#include "../src/zbuild.h"
#include "../src/zlib-ng.h"

#include "uncompr.inc"

static void test_uncompr_001(void **arg)
{
    const unsigned char src[] = {0xff};
    unsigned char dst[1];
    z_size_t src_size = sizeof(src) / sizeof(src[0]);
    z_size_t dst_size = sizeof(dst) / sizeof(dst[0]);

    /* Looks like cmocka has issues with the "PREFIX" macro from zlig-ng */
    expect_function_call(zng_inflateInit_);
    expect_any(zng_inflateInit_, strm);
    expect_string(zng_inflateInit_, version, ZLIBNG_VERSION);
    expect_value(zng_inflateInit_, stream_size, sizeof(PREFIX3(stream)));
    will_return(zng_inflateInit_, NULL);
    will_return(zng_inflateInit_, Z_OK);

    expect_function_call(zng_inflate);
    expect_any(zng_inflate, strm);
    expect_value(zng_inflate, flush, Z_NO_FLUSH);
    will_return(zng_inflate, NULL);
    will_return(zng_inflate, Z_STREAM_END);

    expect_function_call(zng_inflateEnd);
    expect_any(zng_inflateEnd, strm);
    will_return(zng_inflateEnd, NULL);
    will_return(zng_inflateEnd, 0);

    assert_int_equal(Z_OK, PREFIX(uncompress)(dst, &dst_size, src, src_size));
}

int main(int argc, char *argv[])
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_uncompr_001),
    };

    (void) argc;
    (void) argv;

    return cmocka_run_group_tests(tests, NULL, NULL);
}
