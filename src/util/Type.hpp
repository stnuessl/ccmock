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

#ifndef TYPE_HPP_
#define TYPE_HPP_

#include <clang/AST/Type.h>

namespace util {
namespace type {

inline bool isPointerOrReference(const clang::QualType Type)
{
    return Type->isPointerType() || Type->isReferenceType();
}

} // namespace type
} // namespace util

#endif /* TYPE_HPP_ */
