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

#include <stdlib.h>

#include "callback.h"

void callback_invoke1(void *(*) (void *, void *) );
void callback_invoke2(int (*(*func)(long num))(int a, int b));

void callback_main(void)
{
    callback_invoke1(NULL);
    callback_invoke2(NULL);
}
