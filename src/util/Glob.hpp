/*
 * Copyright (C) 2023  Steffen Nuessle
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GLOB_HPP_
#define GLOB_HPP_

#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/StringRef.h>

namespace util {
namespace glob {

static inline bool isPattern(llvm::StringRef Str)
{
    auto Pred = [](char C) { return C == '?' || C == '*' || C == '['; };

    return llvm::any_of(Str, Pred);
}

} /* namespace glob */
} /* namespace util */

#endif /* GLOB_HPP_ */
