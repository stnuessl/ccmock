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

#include "fac.h"
#include "fac.inc"

static void test_fac_basic(void **state)
{
    (void) state;

    assert_int_equal(1, fac(0));
    assert_int_equal(1, fac(1));

    expect_function_call(mul);
    expect_value(mul, x, 2);
    expect_value(mul, y, 1);
    will_return(mul, 2);

    expect_function_call(mul);
    expect_value(mul, x, 3);
    expect_value(mul, y, 2);
    will_return(mul, 6);

    assert_int_equal(6, fac(3));

    expect_function_calls(mul, 3);
    expect_any_always(mul, x);
    expect_any_always(mul, y);

    will_return(mul, 2);
    will_return(mul, 6);
    will_return(mul, 24);

    assert_int_equal(24, fac(4));
}

int main(int argc, char *argv[])
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_fac_basic),
    };

    (void) argc;
    (void) argv;

    return cmocka_run_group_tests(tests, NULL, NULL);
}
