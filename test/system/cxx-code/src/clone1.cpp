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

namespace n1 {
namespace n2 {

void run(int a);
void run(double a);
void dispatch();

namespace n3 {

void run(int a);
void run(double a);
void dispatch(void);

} /* namespace n3 */
} /* namespace n2 */
} /* namespace n1 */

void run()
{
    n1::n2::run(0);
    n1::n2::run(0.0);
    n1::n2::n3::run(1);
    n1::n2::n3::run(1.0);

    n1::n2::dispatch();
    n1::n2::n3::dispatch();
}
