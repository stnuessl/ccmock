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

#include "util/commandline.hpp"

#include "Config.hpp"

namespace llvm {
namespace yaml {

template <> struct ScalarTraits<std::filesystem::path> {
public:
    static void output(const std::filesystem::path &Value,
                       void *Unused,
                       llvm::raw_ostream &Out)
    {
        (void) Unused;

        Out << Value.native();
    }

    static llvm::StringRef
    input(llvm::StringRef Scalar, void *Unused, std::filesystem::path &Value)
    {
        (void) Unused;

        Value = Scalar.str();

        return llvm::StringRef();
    }

    static QuotingType mustQuote(StringRef Str)
    {
        return needsQuotes(Str);
    }
};

template <> struct ScalarEnumerationTraits<Config::ColorMode> {
public:
    static void enumeration(IO &io, Config::ColorMode &Value)
    {
        io.enumCase(Value, "auto", Config::COLORMODE_AUTO);
        io.enumCase(Value, "never", Config::COLORMODE_NEVER);
        io.enumCase(Value, "always", Config::COLORMODE_ALWAYS);
    }
};

template <> struct ScalarEnumerationTraits<Config::Backend> {
public:
    static void enumeration(IO &io, Config::Backend &Value)
    {
        io.enumCase(Value, "gmock", Config::BACKEND_GMOCK);
        io.enumCase(Value, "fff", Config::BACKEND_FFF);
        io.enumCase(Value, "cmocka", Config::BACKEND_CMOCKA);
    }
};

template <> struct MappingTraits<Config::ClangSection> {
public:
    static void mapping(llvm::yaml::IO &IO, Config::ClangSection &Section)
    {
        IO.mapOptional("CompileCommands", Section.CompileCommands);
        IO.mapOptional("ExtraArguments", Section.ExtraArguments);
        IO.mapOptional("RemoveArguments", Section.RemoveArguments);
        IO.mapOptional("ResourceDirectory", Section.ResourceDirectory);

        IO.mapOptional("CompileCommandIndex", Section.CompileCommandIndex);
    }

    static std::string validate(llvm::yaml::IO &IO,
                                Config::ClangSection &Section)
    {
        std::string Message;
        llvm::raw_string_ostream OS(Message);
        std::error_code Error;

        (void) IO;

        if (Section.ResourceDirectory.empty())
            return "";

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

template <> struct MappingTraits<Config::GeneralSection> {
public:
    static void mapping(llvm::yaml::IO &IO, Config::GeneralSection &Section)
    {
        IO.mapOptional("BaseDirectory", Section.BaseDirectory);
        IO.mapOptional("Input", Section.Input);
        IO.mapOptional("Output", Section.Output);

        IO.mapOptional("ColorMode", Section.ColorMode);

        IO.mapOptional("Quiet", Section.Quiet);
        IO.mapOptional("Verbose", Section.Verbose);
        IO.mapOptional("WriteDate", Section.WriteDate);
    }

    static std::string validate(llvm::yaml::IO &IO,
                                Config::GeneralSection &Section)
    {
        (void) IO;
        (void) Section;

        return "";
    }
};

template <> struct MappingTraits<Config::MockingSection> {
public:
    static void mapping(llvm::yaml::IO &IO, Config::MockingSection &Section)
    {
        IO.mapOptional("Blacklist", Section.Blacklist);
        IO.mapOptional("Backend", Section.Backend);
        IO.mapOptional("MockBuiltins", Section.MockBuiltins);
        IO.mapOptional("MockCStdLib", Section.MockCStdLib);
        IO.mapOptional("MockC++StdLib", Section.MockCXXStdLib);
        IO.mapOptional("MockVariadicFunctions", Section.MockVariadicFunctions);
    }
};

template <> struct MappingTraits<Config::GMockSection> {
public:
    static void mapping(llvm::yaml::IO &IO, Config::GMockSection &Section)
    {
        IO.mapOptional("MockType", Section.MockType);
        IO.mapOptional("ClassName", Section.ClassName);
        IO.mapOptional("GlobalNamespaceName", Section.GlobalNamespaceName);
        IO.mapOptional("TestFixtureName", Section.TestFixtureName);
        IO.mapOptional("WriteMain", Section.WriteMain);
    }

    static std::string validate(llvm::yaml::IO &IO,
                                Config::GMockSection &Section)
    {
        (void) IO;

        if (Section.TestFixtureName.find('_') != std::string::npos)
            return "\"TestFixtureName\": contains invalid \"_\" character";

        auto ValidMockTypes = {"NaggyMock", "NiceMock", "StrictMock"};
        auto Pred = [&Section](llvm::StringRef Value) {
            return Value == Section.MockType;
        };

        if (llvm::none_of(ValidMockTypes, Pred))
            return "\"MockType\": invalid value";

        return "";
    }
};

template <> struct MappingTraits<Config::FFFSection> {
public:
    static void mapping(llvm::yaml::IO &IO, Config::FFFSection &Section)
    {
        IO.mapOptional("CallingConvention", Section.CallingConvention);
        IO.mapOptional("GCCFunctionAttributes", Section.GCCFunctionAttributes);
        IO.mapOptional("ArgHistoryLen", Section.ArgHistoryLen);
        IO.mapOptional("CallHistoryLen", Section.CallHistoryLen);
    }
};

template <> struct MappingTraits<Config::CMockaSection> {
public:
    static void mapping(llvm::yaml::IO &IO, Config::CMockaSection &Section)
    {
        IO.mapOptional("OutputParameters", Section.OutputParameters);
        IO.mapOptional("StrictMocks", Section.StrictMocks);
    }
};

template <> struct MappingTraits<Config> {
public:
    static void mapping(llvm::yaml::IO &IO, Config &Config)
    {
        IO.mapOptional("Clang", Config.Clang);
        IO.mapOptional("General", Config.General);
        IO.mapOptional("Mocking", Config.Mocking);
        IO.mapOptional("GMock", Config.GMock);
        IO.mapOptional("FFF", Config.FFF);
        IO.mapOptional("CMocka", Config.CMocka);
    }
};

} // namespace yaml
} // namespace llvm

Config::ClangSection::ClangSection()
    : CompileCommands(),
      ExtraArguments(),
      RemoveArguments(),
      ResourceDirectory(),
      CompileCommandIndex(0)
{
}

Config::CMockaSection::CMockaSection()
    : OutputParameters(true), StrictMocks(true)
{
}

Config::GMockSection::GMockSection()
    : MockType("StrictMock"),
      ClassName("ccmock_"),
      GlobalNamespaceName("_"),
      TestFixtureName("CCMockFixture"),
      WriteMain(true)
{
}

Config::GeneralSection::GeneralSection()
    : BaseDirectory(),
      Input(),
      Output(),
      ColorMode(Config::COLORMODE_AUTO),
      Quiet(false),
      Verbose(false),
      WriteDate(true)
{
}

Config::MockingSection::MockingSection()
    : Blacklist(),
      Backend(Config::BACKEND_GMOCK),
      MockBuiltins(false),
      MockCStdLib(false),
      MockCXXStdLib(false),
      MockVariadicFunctions(true)
{
}

Config::FFFSection::FFFSection()
    : CallingConvention(),
      GCCFunctionAttributes(),
      ArgHistoryLen(-1),
      CallHistoryLen(-1)
{
}

Config::Config() : Clang(), General(), Mocking(), GMock(), FFF(), CMocka()
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
