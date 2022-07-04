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

#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <filesystem>
#include <string>
#include <vector>

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/YAMLTraits.h>

class Config {
public:
    enum UseColorType {
        AUTO = 0,
        NEVER,
        ALWAYS,
    };

    Config();

    void read(llvm::StringRef Path);
    void write(llvm::StringRef Path);
    void write(llvm::raw_ostream &OS);

    std::vector<std::string> Blacklist;
    std::filesystem::path BaseDirectory;
    std::string CompileCommands;
    std::string ClangResourceDirectory;
    std::string MockType;
    std::string MockName;
    std::string MockSuffix;
    std::filesystem::path Output;
    bool MockBuiltins;
    bool MockStandardLibrary;
    bool WriteDate;
    bool Strict;
    bool Verbose;
    bool Force;
    bool Quiet;
    bool WriteMain;
    enum UseColorType UseColor;

private:
    friend struct llvm::yaml::MappingTraits<Config>;
};

#endif /* CONFIG_HPP_ */
