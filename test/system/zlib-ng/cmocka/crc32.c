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

#include "../src/deflate.h"
#include "../src/functable.h"
#include "../src/zbuild.h"
#include "../src/zlib-ng.h"

void crc_reset(deflate_state *const s);

#include "crc32.inc"

static void test_crc_reset_001(void **arg)
{
    deflate_state state, mock_state;
    PREFIX3(stream) strm, mock_strm;

    (void) arg;

    state.strm = &strm;
    state.strm->adler = ~0;

#ifdef X86_PCLMULQDQ_CRC
    x86_cpu_has_pclmulqdq = 1;

    expect_function_call(x86_check_features);

    expect_function_call(crc_fold_init);
    expect_value(crc_fold_init, arg1, &state);

    mock_state.strm = &mock_strm;
    mock_state.strm->adler = 0;

    will_return(crc_fold_init, &mock_state);
#endif

    crc_reset(&state);

    assert_int_equal(0, state.strm->adler);
}

static void test_crc_reset_002(void **arg)
{
    deflate_state state;
    PREFIX3(stream) strm;

    (void) arg;

    state.strm = &strm;
    state.strm->adler = ~0;

#ifdef X86_PCLMULQDQ_CRC
    x86_cpu_has_pclmulqdq = 0;

    expect_function_call(x86_check_features);
#endif

    crc_reset(&state);

    assert_int_equal(0, state.strm->adler);
}

int main(int argc, char *argv[])
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_crc_reset_001),
        cmocka_unit_test(test_crc_reset_002),
    };

    (void) argc;
    (void) argv;

    return cmocka_run_group_tests(tests, NULL, NULL);
}
