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
    static void enumeration(IO &io, Config::UseColorType &Value)
    {
        io.enumCase(Value, "auto", Config::UseColorType::AUTO);
        io.enumCase(Value, "never", Config::UseColorType::NEVER);
        io.enumCase(Value, "always", Config::UseColorType::ALWAYS);
    }
};

template <> struct MappingTraits<Config> {
public:
    static void mapping(llvm::yaml::IO &IO, Config &Config)
    {
        IO.mapOptional("Blacklist", Config.Blacklist);
        IO.mapOptional("BaseDirectory", Config.BaseDirectory);
        IO.mapOptional("UseColor", Config.UseColor);
        IO.mapOptional("CompileCommands", Config.CompileCommands);
        IO.mapOptional("ClangResourceDirectory", Config.ClangResourceDirectory);
        IO.mapOptional("MockStandardLibrary", Config.MockStandardLibrary);
        IO.mapOptional("MockBuiltins", Config.MockBuiltins);
        IO.mapOptional("MockType", Config.MockType);
        IO.mapOptional("MockName", Config.MockName);
        IO.mapOptional("MockSuffix", Config.MockSuffix);
        IO.mapOptional("Output", Config.Output);
        IO.mapOptional("Strict", Config.Strict);
        IO.mapOptional("WriteDate", Config.WriteDate);
        IO.mapOptional("WriteMain", Config.WriteMain);
        IO.mapOptional("Verbose", Config.Verbose);
        IO.mapOptional("Force", Config.Force);
        IO.mapOptional("Quiet", Config.Quiet);
    }

    static std::string validate(llvm::yaml::IO &IO, Config &Config)
    {
        (void) IO;

        if (Config.MockType == "NaggyMock")
            return "";

        if (Config.MockType == "NiceMock")
            return "";

        if (Config.MockType == "StrictMock")
            return "";

        return "\"MockType\": invalid value";
    }
};

} // namespace yaml
} // namespace llvm

Config::Config()
    : Blacklist(),
      BaseDirectory(),
      ClangResourceDirectory(),
      MockType("StrictMock"),
      MockName("mock"),
      MockSuffix("_mock"),
      Output(),
      MockBuiltins(false),
      MockStandardLibrary(false),
      WriteDate(true),
      Strict(false),
      Verbose(false),
      Force(false),
      Quiet(false),
      WriteMain(true),
      UseColor(Config::UseColorType::AUTO)
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
