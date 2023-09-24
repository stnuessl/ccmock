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

#include <filesystem>

#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>

#include "util/commandline.hpp"

#include "MockAction.hpp"
#include "CompilationDatabase.hpp"
#include "Config.hpp"

#ifndef CCMOCK_VERSION_CORE
#error Preprocessor macro "CCMOCK_VERSION_CORE" not defined.
#endif

/* clang-format off */

/* 
 * NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
 * These variables cannot be const-qualified as the interface requirements
 * for the command-line parsing utilities are expecting them to be mutable.
 */
static llvm::cl::OptionCategory ToolCategory("Tool Options");


static llvm::cl::opt<std::string> Input(
    llvm::cl::desc("<file>"),
    llvm::cl::value_desc("file"),
    llvm::cl::ValueRequired,
    llvm::cl::Positional,
    llvm::cl::Optional,
    llvm::cl::cat(ToolCategory)
);

static llvm::cl::opt<std::string> BaseDirectory(
    "base-directory",
    llvm::cl::desc(
        "Use directory <base-directory> when emitting relative paths.\n"
        "Useful as the compiler frontend changes directories according\n"
        "to the entry in the respective compilation database. Default\n"
        "value is the current working directory.\n"
    ),
    llvm::cl::value_desc("directory"),
    llvm::cl::ValueOptional,
    llvm::cl::cat(ToolCategory)
);

static llvm::cl::list<std::string> Blacklist(
    "blacklist",
    llvm::cl::desc(
        "Do not generate mock functions for entities with a full qualified\n"
        "name matching <pattern>.\n"
    ),
    llvm::cl::value_desc("pattern"),
    llvm::cl::ValueRequired,
    llvm::cl::cat(ToolCategory)
);

static llvm::cl::opt<Config::Backend> Backend(
    "backend",
    llvm::cl::desc(
        "Select the backend to use for generating mock functions.\n"
        "The default backend is GMock as it works with C and C++.\n"
    ),
    llvm::cl::values(
        clEnumValN(
            Config::BACKEND_GMOCK, 
            "gmock", 
            "Use the GMock backend."
        ),
        clEnumValN(
            Config::BACKEND_FFF,
            "fff",
            "Use the Fake Function Framework backend."
        ),
        clEnumValN(
            Config::BACKEND_CMOCKA,
            "cmocka",
            "Use the CMocka backend."
        ),
        clEnumValN(
            Config::BACKEND_RAW,
            "raw",
            "Use the raw backend."
        )
    ),
    llvm::cl::value_desc("backend"),
    llvm::cl::init(Config::BACKEND_GMOCK),
    llvm::cl::ValueRequired,
    llvm::cl::cat(ToolCategory)
);

static llvm::cl::opt<std::string> CompileCommands(
    "compile-commands",
    llvm::cl::desc(
        "Use file <file> as the compilation database when running ccmock.\n"
        "Useful if the compilation database is not located in any of the\n"
        "parent directories of the current working directory.\n"
    ),
    llvm::cl::value_desc("file"),
    llvm::cl::ValueRequired,
    llvm::cl::cat(ToolCategory)
);

static llvm::cl::opt<size_t> CompileCommandIndex(
    "cc-index",
    llvm::cl::desc(
        "If there are multiple compile commands for the passed input file\n"
        "in the compilation database, use the command at <index> specific\n"
        "to this file.\n"
    ),
    llvm::cl::value_desc("index"),
    llvm::cl::init(0),
    llvm::cl::ValueRequired,
    llvm::cl::cat(ToolCategory)
);

static llvm::cl::list<std::string> ConfigFile(
    "config",
    llvm::cl::desc(
        "Use file <file> as the local configuration when running ccmock.\n"
        "Useful if the configuration file is not located in any of the\n"
        "parent directories of the current working directory.\n"
    ),
    llvm::cl::value_desc("file"),
    llvm::cl::ValueRequired,
    llvm::cl::CommaSeparated,
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

static llvm::cl::list<std::string> ExtraArguments(
    "extra-args",
    llvm::cl::desc(
        "Append additional command-line arguments to the invocation of\n"
        "the internally used clang compiler.\n"
    ),
    llvm::cl::value_desc("arg"),
    llvm::cl::ValueRequired,
    llvm::cl::CommaSeparated,
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

static llvm::cl::alias ForceAlias(
    "f",
    llvm::cl::desc("Same as --force"),
    llvm::cl::aliasopt(Force),
    llvm::cl::NotHidden
);

static llvm::cl::opt<bool> Verbose(
    "verbose",
    llvm::cl::desc(
        "Turn on verbose output. Writes informational messages to standard "
        "output."
    ),
    llvm::cl::init(false),
    llvm::cl::cat(ToolCategory)
);

static llvm::cl::alias VerboseAlias(
    "v",
    llvm::cl::desc("Same as --verbose"),
    llvm::cl::aliasopt(Verbose)
);

static llvm::cl::opt<bool> Strict(
    "strict",
    llvm::cl::desc("Treat warnings as errors."),
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

static llvm::cl::alias QuietAlias(
    "q",
    llvm::cl::desc("Same as --quiet"),
    llvm::cl::aliasopt(Quiet),
    llvm::cl::NotHidden
);

static llvm::cl::opt<std::string> Output(
    "o",
    llvm::cl::desc(
        "Specifiy the name of the output file. If none is specified\n"
        "the generated output will be printed to standard output.\n"
    ),
    llvm::cl::value_desc("file"),
    llvm::cl::ZeroOrMore,
    llvm::cl::ValueRequired,
    llvm::cl::cat(ToolCategory)
);

static llvm::cl::list<std::string> RemoveArguments(
    "remove-args",
    llvm::cl::desc(
        "Remove command-line arguments from the invocation of\n"
        "the internally used clang compiler. Be aware that this option\n"
        "only removes direct matches and is not aware of any value\n"
        "dependant arguments\n"
    ),
    llvm::cl::value_desc("arg"),
    llvm::cl::ValueRequired,
    llvm::cl::cat(ToolCategory)
);

static llvm::cl::opt<std::string> ResourceDirectory(
    "resource-directory",
    llvm::cl::desc(
        "Path to the clang resource directory. Required by clang tools to\n"
        "include clang specific header files. The program tries to\n"
        "automatically detect the appropriate directory.\n"
    ),
    llvm::cl::value_desc("path"),
    llvm::cl::ValueRequired,
    llvm::cl::cat(ToolCategory)
);

/* NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables) */
/* clang-format on */

class ExtraArgumentsAdjuster {
public:
    explicit ExtraArgumentsAdjuster(std::vector<std::string> &&ExtraArgs)
        : ExtraArgs_(std::move(ExtraArgs))
    {
    }

    clang::tooling::CommandLineArguments
    operator()(const clang::tooling::CommandLineArguments &Args,
               llvm::StringRef File)
    {
        /*
         * We need to work on copies as this adjuster might be used multiple
         * times by the ClangTool.
         */
        auto AdjustedArgs = Args;

        (void) File;

        auto It = std::end(AdjustedArgs);
        auto Begin = std::begin(ExtraArgs_);
        auto End = std::end(ExtraArgs_);

        AdjustedArgs.insert(It, Begin, End);

        return AdjustedArgs;
    }

private:
    clang::tooling::CommandLineArguments ExtraArgs_;
};

class RemoveArgumentsAdjuster {
public:
    explicit RemoveArgumentsAdjuster(std::vector<std::string> &&RemoveArgs)
        : RemoveArgs_(RemoveArgs.size())
    {
        for (auto &&Arg : RemoveArgs)
            RemoveArgs_.insert({std::move(Arg), 0});
    }

    clang::tooling::CommandLineArguments
    operator()(const clang::tooling::CommandLineArguments &Args,
               llvm::StringRef File)
    {
        (void) File;

        auto AdjustedArgs = clang::tooling::CommandLineArguments();
        AdjustedArgs.reserve(Args.size() - RemoveArgs_.size());

        for (const auto &Arg : Args) {
            if (!RemoveArgs_.count(Arg))
                AdjustedArgs.push_back(Arg);
        }

        return AdjustedArgs;
    }

private:
    llvm::StringMap<int> RemoveArgs_;
};

/* NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays) */
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
        llvm::errs() << util::cl::error()
                     << "failed to parse command-line arguments\n";
        std::exit(EXIT_FAILURE);
    }

    /*
     * Print help message if no command-line arguments were provided.
     * Looks like this can only be done after having invoked
     * "llvm::cl::ParseCommandLineOptions".
     */
    if (argc <= 1) {
        llvm::cl::PrintHelpMessage();
        std::exit(EXIT_SUCCESS);
    }

    /*
     * Read in and merge all configuration files.
     * Precedence order is from highest to lowest:
     *      1. Command-line arguments
     *      2. Configuration specified by environment variable
     *      3. 1st Configuration
     *      4. 2nd Configuration
     *      ...
     */
    auto Config = std::make_shared<::Config>();
    for (const auto &File : ConfigFile)
        Config->read(File);

    if (llvm::StringRef File = ::getenv("CCMOCK_CONFIG"); !File.empty())
        Config->read(File);

    if (!ExtraArguments.empty())
        Config->Clang.ExtraArguments = std::move(ExtraArguments);

    if (!RemoveArguments.empty())
        Config->Clang.RemoveArguments = std::move(RemoveArguments);

    if (!ResourceDirectory.empty())
        Config->Clang.ResourceDirectory = std::move(ResourceDirectory);

    if (!BaseDirectory.empty())
        Config->General.BaseDirectory = std::move(BaseDirectory);

    if (Backend.getNumOccurrences() != 0)
        Config->Mocking.Backend = Backend;

    if (CompileCommandIndex.getNumOccurrences() != 0)
        Config->Clang.CompileCommandIndex = CompileCommandIndex;

    if (!Blacklist.empty())
        Config->Mocking.Blacklist = std::move(Blacklist);

    if (CompileCommands.getNumOccurrences() != 0)
        Config->Clang.CompileCommands = std::move(CompileCommands);

    if (Verbose.getNumOccurrences() != 0)
        Config->General.Verbose = Verbose;

    if (Quiet.getNumOccurrences() != 0)
        Config->General.Quiet = Quiet;

    if (!Input.empty())
        Config->General.Input = std::move(Input);

    if (!Output.empty())
        Config->General.Output = std::move(Output);

    /*
     * Dump the config now before adjusting it for program internal reasons
     * so the users can see their effective configuration settings.
     */
    if (DumpConfig) {
        if (!Config->General.Output.empty())
            Config->write(Config->General.Output.native());
        else
            Config->write(llvm::outs());

        std::exit(EXIT_SUCCESS);
    }

    if (Config->General.BaseDirectory.empty())
        Config->General.BaseDirectory = std::filesystem::current_path();

    auto &Path = Config->General.Output;
    if (!Path.empty() && Path.is_relative()) {
        /*
         * The ClangTool might change the working directory during its
         * invocation. We therefore use the absolute path for the output file
         * to make sure it will be created were the user expects it.
         */
        Path = std::filesystem::absolute(Path);
    }

    /* Enable or disable colored output */
    switch (Config->General.ColorMode) {
    case Config::COLORMODE_AUTO:
        break;
    case Config::COLORMODE_NEVER:
        llvm::errs().enable_colors(false);
        break;
    case Config::COLORMODE_ALWAYS:
        llvm::errs().enable_colors(true);
        break;
    default:
        break;
    }

    /* Prepare inputs for the invocation of the ClangTool */
    auto Commands = CompilationDatabase();
    Commands.setIndex(Config->Clang.CompileCommandIndex);

    if (!Config->Clang.CompileCommands.empty())
        Commands.load(Config->Clang.CompileCommands, Message);
    else
        Commands.detect(Config->General.Input, Message);

    if (!Commands) {
        llvm::errs() << util::cl::error() << Message << "\n";
        std::exit(EXIT_FAILURE);
    }

    if (Config->General.Input.empty()) {
        llvm::errs() << util::cl::error()
                     << "no input source file specified.\n";
        std::exit(EXIT_FAILURE);
    }

    auto Factory = MockActionFactory();
    Factory.setConfig(Config);

    /* Perform the ClangTool invocation */
    const auto &Input = Config->General.Input.native();
    auto Tool = clang::tooling::ClangTool(Commands, Input);

    auto &ExtraArgs = Config->Clang.ExtraArguments;
    if (!ExtraArgs.empty()) {
        auto Adjuster = ExtraArgumentsAdjuster(std::move(ExtraArgs));
        Tool.appendArgumentsAdjuster(std::move(Adjuster));
    }

    auto &RemoveArgs = Config->Clang.RemoveArguments;
    if (!RemoveArgs.empty()) {
        auto Adjuster = RemoveArgumentsAdjuster(std::move(RemoveArgs));
        Tool.appendArgumentsAdjuster(std::move(Adjuster));
    }

    return Tool.run(&Factory);
}

#ifndef UNIT_TESTS_ENABLED
int main(int argc, const char *argv[])
{
    return ccmock_main(argc, argv);
}
#endif
