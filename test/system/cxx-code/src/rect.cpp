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

#include <utility>

class rect {
public:
    rect() = default;
    rect(int x, int y, int width, int height);
    rect(const rect &other);
    rect(rect &&other) noexcept;
    ~rect();

    rect &operator=(const rect &other);
    rect &operator=(rect &&other) noexcept;

    inline void set_x(int x)
    {
        x1 = x;
    }
    inline void set_y(int y)
    {
        y1 = y;
    }

    void set_width(int width);
    void set_height(int height);

    int x() const;
    int y() const;
    int width() const;
    int height() const;

    int area() const;

private:
    int x1;
    int y1;
    int x2;
    int y2;
};

namespace util {
void dispatch(const rect &rect);
void dispatch(int value);
} /* namespace util */

void run()
{
    const int w1 = 800, h1 = 600, w2 = 600, h2 = 480;

    auto r1 = rect(0, 0, w1, h1);
    auto r2 = rect(0, 0, w2, h2);
    auto r3 = rect();

    r1 = r2;
    r3 = std::move(r2);

    util::dispatch(r1);
    util::dispatch(r2);
    util::dispatch(r3);

    int area = r1.area();
    util::dispatch(area);
}
