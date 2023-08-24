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

#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/JSONCompilationDatabase.h>

#include "CompilationDatabase.hpp"

void CompilationDatabase::load(const std::filesystem::path &Path,
                               std::string &Error)
{
    using namespace clang::tooling;

    auto &Item = Path.native();
    auto Ext = llvm::sys::path::extension(Item);
    if (Ext.equals(".json")) {
        auto Value = JSONCommandLineSyntax::AutoDetect;

        Database_ = JSONCompilationDatabase::loadFromFile(Item, Error, Value);
        return;
    }

    if (Ext.equals(".txt")) {
        Database_ = FixedCompilationDatabase::loadFromFile(Item, Error);
        return;
    }

    llvm::raw_string_ostream OS(Error);
    OS << "unsupported compilation database extension \"" << Ext << "\"\n";
}

void CompilationDatabase::detect(const std::filesystem::path &Path,
                                 std::string &Error)
{
    using clang::tooling::CompilationDatabase;
    using clang::tooling::FixedCompilationDatabase;

    Database_ = CompilationDatabase::autoDetectFromSource(Path.native(), Error);
    if (Database_)
        return;

    /* 
     * Fallback: Assume trivial compile commands. This allows to specifiy
     * additional compiler arguments via "--extra-arg" on the command-line
     * without running into an error.
     */
    auto Directory = llvm::sys::path::parent_path(Path.native());

    Database_ = FixedCompilationDatabase::loadFromBuffer(Directory, "", Error);
}

std::vector<clang::tooling::CompileCommand>
CompilationDatabase::getCompileCommands(llvm::StringRef File) const
{
    auto Vec = Database_->getCompileCommands(File);

    if (Index_ >= Vec.size()) {
        Vec.clear();
        return Vec;
    }

    if (Index_ != 0)
        std::swap(Vec[0], Vec[Index_]);

    Vec.resize(1);

    return Vec;
}

std::vector<std::string> CompilationDatabase::getAllFiles() const
{
    return Database_->getAllFiles();
}

std::vector<clang::tooling::CompileCommand>
CompilationDatabase::getAllCompileCommands() const
{
    return Database_->getAllCompileCommands();
}
