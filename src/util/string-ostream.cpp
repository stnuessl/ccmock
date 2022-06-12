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

#include "string-ostream.hpp"

namespace util {

string_ostream::string_ostream(std::size_t size)
    : llvm::raw_ostream(true), buffer_()
{
    buffer_.reserve(size);
}

uint64_t string_ostream::current_pos() const
{
    return buffer_.size();
}

void string_ostream::write_impl(const char *ptr, size_t size)
{
    buffer_.append(ptr, size);
}

void string_ostream::clear()
{
    buffer_.clear();
}

const std::string &string_ostream::str() const
{
    return buffer_;
}

} // namespace util
