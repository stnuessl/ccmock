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

#include "util/commandline.hpp"

#include "Config.hpp"

namespace llvm {
namespace yaml {

template <> struct ScalarTraits<std::filesystem::path> {
public:
    static void
    output(const std::filesystem::path &Value, void *, llvm::raw_ostream &Out)
    {
        Out << Value.native();
    }

    static llvm::StringRef
    input(llvm::StringRef Scalar, void *, std::filesystem::path &Value)
    {
        Value = Scalar.str();

        return llvm::StringRef();
    }

    static QuotingType mustQuote(StringRef Str)
    {
        return needsQuotes(Str);
    }
};

template <> struct ScalarEnumerationTraits<Config::UseColorType> {
public:
    static void enumeration(IO &io, Config::UseColorType &Value)
    {
        io.enumCase(Value, "auto", Config::UseColorType::AUTO);
        io.enumCase(Value, "never", Config::UseColorType::NEVER);
        io.enumCase(Value, "always", Config::UseColorType::ALWAYS);
    }
};

template <> struct MappingTraits<Config::ClangSection> {
public:
    static void mapping(llvm::yaml::IO &IO, Config::ClangSection &Section)
    {
        IO.mapOptional("ResourceDirectory", Section.ResourceDirectory);
    }
    
    static std::string validate(llvm::yaml::IO &IO, Config::ClangSection &Section)
    {
        std::string Message;
        llvm::raw_string_ostream OS(Message);
        std::error_code Error;

        (void) IO;

        if (std::filesystem::is_directory(Section.ResourceDirectory, Error))
            return "";

        OS << "Clang.ResourceDirectory: ";
        if (Error) {
            OS << Error.message();
            return Message;
        }

        OS << "\"" << Section.ResourceDirectory.native() 
           << "\" is not a directory";

        return Message;
    }
};

template <> struct MappingTraits<Config::GMockSection> {
public:
    static void mapping(llvm::yaml::IO &IO, Config::GMockSection &Section)
    {
        IO.mapOptional("MockType", Section.MockType);
        IO.mapOptional("MockName", Section.MockName);
        IO.mapOptional("MockSuffix", Section.MockSuffix);
        IO.mapOptional("WriteMain", Section.WriteMain);
    }

    static std::string validate(llvm::yaml::IO &IO, Config::GMockSection &Section)
    {
        (void) IO;

        if (Section.MockType == "NaggyMock")
            return "";

        if (Section.MockType == "NiceMock")
            return "";

        if (Section.MockType == "StrictMock")
            return "";

        return "\"MockType\": invalid value";
    }
};

template <> struct MappingTraits<Config::GeneralSection> {
public:
    static void mapping(llvm::yaml::IO &IO, Config::GeneralSection &Section)
    {
        IO.mapOptional("BaseDirectory", Section.BaseDirectory);
        IO.mapOptional("CompileCommands", Section.CompileCommands);
        IO.mapOptional("Input", Section.Input);
        IO.mapOptional("Output", Section.Output);

        IO.mapOptional("UseColor", Section.UseColor);

        IO.mapOptional("Quiet", Section.Quiet);
        IO.mapOptional("Verbose", Section.Verbose);
        IO.mapOptional("WriteDate", Section.WriteDate);
    }

    static std::string validate(llvm::yaml::IO &IO, Config::GeneralSection &Section)
    {
        llvm::ArrayRef<const std::filesystem::path *> List = {
            &Section.CompileCommands,
            &Section.Input,
            &Section.Output,
        };
        std::string Message;
        llvm::raw_string_ostream OS(Message);
        std::error_code Error;

        (void) IO;

        for (auto File : List) {
            if (File->empty())
                continue;

            if (!std::filesystem::is_directory(*File, Error))
                continue;


            if (Error) {
                OS << File->native() << ": " << Error.message();

                return Message;
            }

            OS << File->native() << ": is a directory";

            return Message;
        }
        return "";
    }
};

template <> struct MappingTraits<Config::MockingSection> {
public:
    static void mapping(llvm::yaml::IO &IO, Config::MockingSection &Section)
    {
        IO.mapOptional("Blacklist", Section.Blacklist);
        IO.mapOptional("MockBuiltins", Section.MockBuiltins);
        IO.mapOptional("MockStdlib", Section.MockStdlib);
    }
};

template <> struct MappingTraits<Config> {
public:
    static void mapping(llvm::yaml::IO &IO, Config &Config)
    {
        IO.mapOptional("Clang", Config.Clang);
        IO.mapOptional("GMock", Config.GMock);
        IO.mapOptional("General", Config.General);
        IO.mapOptional("Mocking", Config.Mocking);
    }
};

} // namespace yaml
} // namespace llvm

Config::ClangSection::ClangSection()
    : ResourceDirectory()
{
}

Config::GMockSection::GMockSection()
    : MockType("StrictMock"),
      MockName("mock"),
      MockSuffix("_mock"),
      WriteMain(true)
{
}

Config::GeneralSection::GeneralSection()
    : BaseDirectory(),
      CompileCommands(),
      Output(),
      UseColor(Config::UseColorType::AUTO),
      Quiet(false),
      Verbose(false),
      WriteDate(false)
{
}

Config::MockingSection::MockingSection()
    : Blacklist(),
      MockBuiltins(false),
      MockStdlib(false)
{
}

Config::Config()
    : Clang(),
      GMock(),
      General(),
      Mocking()
{
}

void Config::read(llvm::StringRef Path)
{
    auto MemBuffer = llvm::MemoryBuffer::getFile(Path);
    if (!MemBuffer) {
        llvm::errs() << util::cl::error() << "failed to open \"" << Path
                     << "\": " << MemBuffer.getError().message() << "\n";
        std::exit(EXIT_FAILURE);
    }

    llvm::yaml::Input YamlInput(MemBuffer.get()->getBuffer());
    YamlInput >> *this;

    if (YamlInput.error()) {
        llvm::errs() << util::cl::error() << "failed to parse \"" << Path
                     << "\".\n";
        std::exit(EXIT_FAILURE);
    }
}

void Config::write(llvm::StringRef Path)
{
    std::error_code Error;

    llvm::raw_fd_ostream OS(Path, Error);

    if (Error) {
        llvm::errs() << util::cl::error() << "failed to open \"" << Path
                     << "\": " << Error.message() << "\n";
        std::exit(EXIT_FAILURE);
    }

    write(OS);
}

void Config::write(llvm::raw_ostream &OS)
{
    auto YamlOutput = llvm::yaml::Output(OS);

    YamlOutput << *this;
}


