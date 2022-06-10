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
        IO.mapOptional("UseColor", Config.UseColor);
        IO.mapOptional("ClangResourceDirectory", Config.ClangResourceDirectory);
        IO.mapOptional("MockStandardLibrary", Config.MockStandardLibrary);
        IO.mapOptional("MockType", Config.MockType, "StrictMock");
        IO.mapOptional("Output", Config.Output);
        IO.mapOptional("PrintTimestamp", Config.PrintTimestamp);
        IO.mapOptional("PrintMainFunction", Config.PrintMainFunction);
        IO.mapOptional("Verbose", Config.Verbose);
        IO.mapOptional("Force", Config.Force);
        IO.mapOptional("Quiet", Config.Quiet);
    }

    static std::string validate(llvm::yaml::IO &IO, Config &Config)
    {
        (void) IO;

//        if (Config.MockType == "") {
//            Config.MockType = "StrictMock";
//            return "";
//        }

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

void Config::read(llvm::StringRef Path)
{
    auto MemBuffer = llvm::MemoryBuffer::getFile(Path);
    if (!MemBuffer) {
        /* TODO: */
        llvm::errs() << "failed to open file \"" << Path << "\" - "
                     << MemBuffer.getError().message() << "\n";
        std::exit(EXIT_FAILURE);
    }

    llvm::yaml::Input Input(MemBuffer.get()->getBuffer());
    Input >> *this;

    if (Input.error()) {
        /* TODO: */
        llvm::errs() << "failed to parse file \"" << Path << "\".\n";
        std::exit(EXIT_FAILURE);
    }
}

void Config::write(llvm::StringRef Path)
{
    std::error_code Error;

    llvm::raw_fd_ostream OS(Path, Error);

    if (Error) {
        llvm::errs() << util::cl::Error()
                     << "failed to open \"" << Path << "\": "
                     << Error.message() << "\n";
        std::exit(EXIT_FAILURE);
    }

    write(OS);
}

void Config::write(llvm::raw_ostream &OS)
{
    auto Output = llvm::yaml::Output(OS);

    Output << *this;
}
