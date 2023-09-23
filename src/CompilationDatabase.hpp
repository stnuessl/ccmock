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

#ifndef COMPILATION_DATABASE_HPP
#define COMPILATION_DATABASE_HPP

#include <filesystem>
#include <memory>

#include <clang/Tooling/CompilationDatabase.h>

/*
 * Adapter class which allows to exactly control which compile command is used
 * to process a source file in case multiple commands are available.
 */

class CompilationDatabase : public clang::tooling::CompilationDatabase {
public:
    inline void setIndex(unsigned int Num);
    void load(const std::filesystem::path &Path, std::string &Error);
    void detect(const std::filesystem::path &Path, std::string &Error);

    std::vector<clang::tooling::CompileCommand>
    getCompileCommands(llvm::StringRef File) const override;

    std::vector<std::string> getAllFiles() const override;

    std::vector<clang::tooling::CompileCommand>
    getAllCompileCommands() const override;

    inline operator bool() const noexcept;

private:
    std::unique_ptr<clang::tooling::CompilationDatabase> Database_;
    unsigned int Index_;
};

inline void CompilationDatabase::setIndex(unsigned int Num)
{
    Index_ = Num;
}

inline CompilationDatabase::operator bool() const noexcept
{
    return Database_ != nullptr;
}

#endif /* COMPILATION_DATABASE_HPP */
