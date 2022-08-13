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

#include "../src/functable.h"
#include "../src/zbuild.h"
#include "../src/zlib-ng.h"

/*
 * Function declarations which are not availabie via header files.
 * Be aware that these functions need C linkage .
 */
uint32_t adler32_c(uint32_t adler, const unsigned char *buf, size_t len);

#include "adler32.inc"

static void test_adler32_c_001(void **arg)
{
    functable.adler32 = &adler32_c;

    (void) arg;

    assert_int_equal(1, adler32_c(0, NULL, 0));
}

int main(int argc, char *argv[])
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_adler32_c_001),
    };

    (void) argc;
    (void) argv;

    return cmocka_run_group_tests(tests, NULL, NULL);
}
