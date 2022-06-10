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

#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <clang/Tooling/Tooling.h>

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>

#include "util/commandline.hpp"

#include "ActionFactory.hpp"
#include "Config.hpp"

#ifndef CCMOCK_VERSION_MAJOR
#error Preprocessor macro "CCMOCK_VERSION_MAJOR" not defined.
#endif

#ifndef CCMOCK_VERSION_MINOR
#error Preprocessor macro "CCMOCK_VERSION_MINOR" not defined.
#endif

#ifndef CCMOCK_VERSION_PATCH
#error Preprocessor macro "CCMOCK_VERSION_PATCH" not defined.
#endif

#ifndef CCMOCK_VERSION_CORE
#error Preprocessor macro "CCMOCK_VERSION_CORE" not defined.
#endif

/* clang-format off */

static llvm::cl::OptionCategory ToolCategory("Tool Options");

static llvm::cl::list<std::string> InputFiles(
    llvm::cl::desc("[<file> ...]"),
    llvm::cl::Positional,
    llvm::cl::ZeroOrMore,
    llvm::cl::PositionalEatsArgs,
    llvm::cl::cat(ToolCategory)
);

static llvm::cl::opt<std::string> CompileCommandsFile(
    "compile-commands",
    llvm::cl::desc(
        "Use file <file> as the compilation database when running ccmock.\n"
        "Useful if the compilation database is not located in any of the\n"
        "parent directories of the current working directory.\n"
    ),
    llvm::cl::value_desc("file"),
    llvm::cl::ValueOptional,
    llvm::cl::cat(ToolCategory)
);

static llvm::cl::opt<std::string> ConfigFile(
    "config",
    llvm::cl::desc(
        "Use file <file> as the local configuration when running ccmock.\n"
        "Useful if the configuration file is not located in any of the\n"
        "parent directories of the current working directory.\n"
    ),
    llvm::cl::value_desc("file"),
    llvm::cl::ValueOptional,
    llvm::cl::cat(ToolCategory)
);

static llvm::cl::opt<bool> DumpConfig(
    "dump-config",
    llvm::cl::desc(
        "Print the accumulated active configuration and exit. This option\n"
        "Can be combined with the \"-o\" option.\n"
    ),
    llvm::cl::init(false),
    llvm::cl::cat(ToolCategory)
);

static llvm::cl::opt<bool> Force(
    "force",
    llvm::cl::desc(
        ""
    ),
    llvm::cl::init(false),
    llvm::cl::cat(ToolCategory)
);

static llvm::cl::opt<bool> Verbose(
    "verbose",
    llvm::cl::desc(
        ""
    ),
    llvm::cl::init(false),
    llvm::cl::cat(ToolCategory)
);

static llvm::cl::opt<bool> Quiet(
    "quiet",
    llvm::cl::desc(
        ""
    ),
    llvm::cl::init(false),
    llvm::cl::cat(ToolCategory)
);

static llvm::cl::opt<std::string> OutputFile(
    "o",
    llvm::cl::desc(
        "Specifiy the name of the output file. If none is specified\n"
        "the generated output will be printed to standard output.\n"
    ),
    llvm::cl::value_desc("file"),
    llvm::cl::ValueRequired,
    llvm::cl::cat(ToolCategory)
);

/* clang-format on */

static std::unique_ptr<clang::tooling::CompilationDatabase>
loadCompileCommands(llvm::StringRef Path, std::string &Message)
{
    using namespace clang::tooling;

    if (Path.empty()) {
        llvm::SmallString<256> Buffer;

        auto Error = llvm::sys::fs::current_path(Buffer);

        if (Error) {
            llvm::errs() << util::cl::Warning()
                         << "failed to retrieve working directory: "
                         << Error.message() << "\n";
            Buffer = ".";
        }

        Path = Buffer.str();
        return CompilationDatabase::autoDetectFromDirectory(Path, Message);
    }

    auto Ext = llvm::sys::path::extension(Path);

    if (Ext.equals(".json")) {
        auto Syntax = JSONCommandLineSyntax::AutoDetect;
        return JSONCompilationDatabase::loadFromFile(Path, Message, Syntax);
    }

    if (Ext.equals(".txt"))
        return FixedCompilationDatabase::loadFromFile(Path, Message);

    llvm::errs() << util::cl::Error()
                 << "failed to load compile commands from \""
                 << Path
                 << "\": unsupported extension\n";

    std::exit(EXIT_FAILURE);
}


__attribute__((used)) static int ccmock_main(int argc, const char *argv[])
{
    llvm::StringRef Overview = "";
    std::string Message;

    const auto VersionPrinter = [](llvm::raw_ostream &OS) {
        OS << "ccmock " << CCMOCK_VERSION_CORE << "\n";
    };

    llvm::cl::SetVersionPrinter(VersionPrinter);
    llvm::cl::HideUnrelatedOptions(ToolCategory);

    bool Ok = llvm::cl::ParseCommandLineOptions(argc,
                                                argv,
                                                Overview,
                                                &llvm::errs(),
                                                nullptr,
                                                true);
    if (!Ok) {
        llvm::errs() << util::cl::Error()
                     << "failed to parse command-line arguments\n";
        std::exit(EXIT_FAILURE);
    }

    /* Print help message if no command-line arguments were provided */
    if (argc <= 1) {
        llvm::cl::PrintHelpMessage();
        std::exit(EXIT_SUCCESS);
    }

    /* Read in and merge all configuration files */
    auto Config = std::make_shared<::Config>();
    if (ConfigFile.empty())
        Config->read("ccmock.yaml");
    else
        Config->read(ConfigFile);

    if (Verbose.getNumOccurrences() != 0)
        Config->Verbose = Verbose;

    if (Force.getNumOccurrences() != 0)
        Config->Force = Force;

    if (Quiet.getNumOccurrences() != 0)
        Config->Quiet = Quiet;

    if (DumpConfig) {
        if (!OutputFile.empty())
            Config->write(OutputFile);
        else
            Config->write(llvm::outs());

        std::exit(EXIT_SUCCESS);
    }

    switch (Config->UseColor) {
    case Config::UseColorType::AUTO:
        break;
    case Config::UseColorType::NEVER:
        llvm::errs().enable_colors(false);
        break;
    case Config::UseColorType::ALWAYS:
        llvm::errs().enable_colors(true);
        break;
    default:
        break;
    }

    auto Factory = ActionFactory();
    Factory.setConfig(Config);

    auto Commands = loadCompileCommands(CompileCommandsFile, Message);
    if (!Commands) {
        llvm::errs() << util::cl::Error()
                     << Message << "\n";

        std::exit(EXIT_FAILURE);
    }

    if (InputFiles.empty()) {
        llvm::errs() << util::cl::Error()
                     << "no input source file specified.\n";
        std::exit(EXIT_FAILURE);
    }

    auto Tool = clang::tooling::ClangTool(*Commands, InputFiles);
    Tool.run(&Factory);

    return EXIT_SUCCESS;
}

#ifndef UNIT_TESTS_ENABLED
int main(int argc, const char *argv[])
{
    return ccmock_main(argc, argv);
}
#endif
