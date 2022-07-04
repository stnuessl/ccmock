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

#include <llvm/Support/Path.h>

#include <util/commandline.hpp>

#include <util/Path.hpp>

namespace util {

namespace path {

std::filesystem::path make_relative(llvm::StringRef Path)
{
    return util::path::make_relative(Path, std::filesystem::current_path());
}

std::filesystem::path make_relative(llvm::StringRef Path, 
                              const std::filesystem::path &Base) 
{
    if (llvm::sys::path::is_relative(Path))
        return std::filesystem::path(Path.begin(), Path.end());

    std::error_code Error;
    auto Output = std::filesystem::relative(Path.str(), Base, Error);
    if (Error) {
        llvm::errs() << util::cl::error() 
                     << "failed to convert \""
                     << Path
                     << "\" to relative path: "
                     << Error.message() << "\n";
        std::exit(EXIT_FAILURE);
    }
    
    return Output;
}

} /* namespace path */

} /* namespace util */

