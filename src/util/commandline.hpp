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

#ifndef COMMANDLINE_HPP_
#define COMMANDLINE_HPP_

#include <llvm/Support/raw_ostream.h>

namespace util {
namespace cl {

struct Info {
public:
    friend inline llvm::raw_ostream &operator<<(llvm::raw_ostream &OS,
                                                const Info Item);
};

struct Warning {
public:
    friend inline llvm::raw_ostream &operator<<(llvm::raw_ostream &OS,
                                                const Warning Item);
};

struct Error {
public:
    friend inline llvm::raw_ostream &operator<<(llvm::raw_ostream &OS,
                                                const Error Item);
};

static inline llvm::raw_ostream &write(llvm::raw_ostream &OS,
                                       llvm::raw_ostream::Colors Color,
                                       llvm::StringRef Text)
{
    OS.changeColor(Color, true);
    OS << Text;
    OS.resetColor();

    return OS;
}

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, const Info X)
{
    (void) X;

    return write(OS, llvm::raw_ostream::GREEN, "info: ");
}

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, const Warning X)
{
    (void) X;

    return write(OS, llvm::raw_ostream::MAGENTA, "warning: ");
}

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, const Error X)
{
    (void) X;

    return write(OS, llvm::raw_ostream::RED, "error: ");
}

} // namespace cl
} // namespace util

#endif /* COMMANDLINE_HPP_ */
