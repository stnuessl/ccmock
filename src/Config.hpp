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

#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <filesystem>
#include <string>
#include <vector>

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/YAMLTraits.h>

class Config {
public:
    enum ColorMode {
        COLORMODE_AUTO = 0,
        COLORMODE_NEVER,
        COLORMODE_ALWAYS,
    };

    enum Backend {
        BACKEND_GMOCK,
        BACKEND_FFF,
        BACKEND_CMOCKA,
        BACKEND_RAW,
    };

    struct ClangSection {
    public:
        ClangSection();

        std::filesystem::path CompileCommands;
        std::vector<std::string> ExtraArguments;
        std::vector<std::string> RemoveArguments;
        std::filesystem::path ResourceDirectory;

        unsigned int CompileCommandIndex;
    };

    struct GeneralSection {
    public:
        GeneralSection();

        std::filesystem::path BaseDirectory;
        std::filesystem::path Input;
        std::filesystem::path Output;

        enum ColorMode ColorMode;

        bool Quiet;
        bool Verbose;
        bool WriteDate;
    };

    struct MockingSection {
    public:
        MockingSection();

        std::vector<std::string> Blacklist;

        enum Backend Backend;

        bool MockBuiltins;
        bool MockCStdLib;
        bool MockCXXStdLib;
        bool MockVariadicFunctions;
    };

    struct GMockSection {
    public:
        GMockSection();

        std::string MockType;
        std::string ClassName;
        std::string GlobalNamespaceName;
        std::string TestFixtureName;
        bool WriteMain;
    };

    struct FFFSection {
    public:
        FFFSection();

        std::string CallingConvention;
        std::string GCCFunctionAttributes;
        int ArgHistoryLen;
        int CallHistoryLen;
    };

    struct CMockaSection {
    public:
        CMockaSection();

        bool OutputParameters;
        bool StrictMocks;
    };

    Config();

    void read(llvm::StringRef Path);
    void write(llvm::StringRef Path);
    void write(llvm::raw_ostream &OS);

    ClangSection Clang;
    GeneralSection General;
    MockingSection Mocking;
    GMockSection GMock;
    FFFSection FFF;
    CMockaSection CMocka;
};

#endif /* CONFIG_HPP_ */
