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

    struct ClangSection {
    public:
        ClangSection();

        std::filesystem::path ResourceDirectory;
    };

    struct GeneralSection {
    public:
        GeneralSection();

        std::filesystem::path BaseDirectory;
        std::filesystem::path CompileCommands;
        std::filesystem::path Input;
        std::filesystem::path Output;
        
        enum UseColorType UseColor;

        bool Quiet;
        bool Verbose;
        bool WriteDate;
    };

    struct GMockSection {
    public:
        GMockSection();

        std::string MockType;
        std::string MockName;
        std::string MockSuffix;
        bool WriteMain;
    };

    struct MockingSection {
    public:
        MockingSection();
        
        std::vector<std::string> Blacklist;

        bool MockBuiltins;
        bool MockStdlib;
    };

    Config();

    void read(llvm::StringRef Path);
    void write(llvm::StringRef Path);
    void write(llvm::raw_ostream &OS);
    
    ClangSection Clang;
    GMockSection GMock;
    GeneralSection General;
    MockingSection Mocking;
};

#endif /* CONFIG_HPP_ */
