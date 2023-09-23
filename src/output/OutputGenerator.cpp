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

#include "OutputGenerator.hpp"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/Support/GlobPattern.h>

#include "util/Decl.hpp"
#include "util/Glob.hpp"
#include "util/commandline.hpp"

namespace {

const llvm::StringMap<int> InternalBlacklist = {
    {"environ", 0},
    {"stdin", 0},
    {"stdout", 0},
    {"stderr", 0},
    {"std::cin", 0},
    {"std::wcin", 0},
    {"std::cout", 0},
    {"std::wcout", 0},
    {"std::cerr", 0},
    {"std::wcerr", 0},
};

class ASTVisitor : public clang::RecursiveASTVisitor<ASTVisitor> {
public:
    static void run(clang::ASTContext &Context, OutputGenerator &Generator);

    bool VisitCallExpr(clang::CallExpr *CallExpr);
    bool VisitCXXConstructExpr(clang::CXXConstructExpr *ConstructExpr);
    bool VisitDeclRefExpr(clang::DeclRefExpr *DeclRefExpr);

    bool shouldWalkTypesOfTypeLocs() const;

private:
    ASTVisitor(clang::ASTContext &Context, OutputGenerator *Generator);

    const Config &getConfig() const;

    void dispatch(const clang::Expr *Expr,
                  const clang::FunctionDecl *FunctionDecl);
    bool isVisited(const clang::DeclaratorDecl *Decl);

    void doVisitCallExpr(const clang::CallExpr *CallExpr);
    void doVisitCXXConstructExpr(const clang::CXXConstructExpr *ConstructExpr);
    void doVisitDeclRefExpr(const clang::DeclRefExpr *DeclRefExpr);

    OutputGenerator *Generator_;

    std::vector<std::pair<llvm::StringRef, llvm::GlobPattern>> GlobBlacklist_;
    llvm::DenseSet<int64_t> Visited_;
    llvm::StringMap<int> Blacklist_;
    clang::SourceManager *SourceManager_;

    std::string Buffer_;
};

void ASTVisitor::run(clang::ASTContext &Context, OutputGenerator &Generator)
{
    ASTVisitor Visitor(Context, &Generator);

    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
}

ASTVisitor::ASTVisitor(clang::ASTContext &Context, OutputGenerator *Generator)
    : Generator_(Generator),
      GlobBlacklist_(),
      Visited_(64),
      Blacklist_(32),
      SourceManager_(&Context.getSourceManager()),
      Buffer_()
{
    Buffer_.reserve(256);

    for (const auto &Name : getConfig().Mocking.Blacklist) {
        if (!util::glob::isPattern(Name)) {
            Blacklist_.insert({Name, 0});
            continue;
        }

        auto ExpectedGlob = llvm::GlobPattern::create(Name);
        if (!ExpectedGlob) {
            llvm::errs() << util::cl::error() << ExpectedGlob.takeError()
                         << "\n";

            std::exit(EXIT_FAILURE);
        }

        GlobBlacklist_.push_back({Name, std::move(*ExpectedGlob)});
    }
}

const Config &ASTVisitor::getConfig() const
{
    return Generator_->getConfig();
}

bool ASTVisitor::VisitCallExpr(clang::CallExpr *CallExpr)
{
    doVisitCallExpr(CallExpr);

    return true;
}

bool ASTVisitor::VisitCXXConstructExpr(clang::CXXConstructExpr *ConstructExpr)
{
    doVisitCXXConstructExpr(ConstructExpr);

    return true;
}

bool ASTVisitor::VisitDeclRefExpr(clang::DeclRefExpr *DeclRefExpr)
{
    doVisitDeclRefExpr(DeclRefExpr);

    return true;
}

bool ASTVisitor::shouldWalkTypesOfTypeLocs() const
{
    return false;
}

void ASTVisitor::dispatch(const clang::Expr *Expr,
                          const clang::FunctionDecl *Decl)
{
    llvm::raw_string_ostream OS(Buffer_);

    /*
     * Call expressions via function pointers don't have a function
     * declaration associated with them.
     */
    if (!Decl)
        return;

    if (!SourceManager_->isInMainFile(Expr->getExprLoc()))
        return;

    if (Decl->isDefined())
        return;

    /*
     * Mocking this function raises a lot of issues as it is heavily used
     * when dealing with system calls. This most likely will negatively
     * impact any unit test framework before even running any tests.
     */
    if (Decl->getIdentifier() && Decl->getName().equals("__errno_location"))
        return;

    /*
     * Deal with builtin functions which might refer to compiler builtins
     * like "__builtin_expect" or standard library functions.
     */
    const auto &Config = getConfig();

    if (Decl->getBuiltinID()) {
        auto Name = Decl->getName();

        /* Non standard library builtins */
        if (Name.startswith("__builtin_") && !Config.Mocking.MockBuiltins) {
            if (Config.General.Verbose) {
                llvm::errs() << util::cl::info() << "skipping builtin \""
                             << Name << "\"\n";
            }

            return;
        }

        /* C++ standard library functions */
        if (Decl->isInStdNamespace() && !Config.Mocking.MockCXXStdLib) {
            if (Config.General.Verbose) {
                llvm::errs() << util::cl::info()
                             << "skipping C++ standard library function \"";
                Decl->printQualifiedName(llvm::errs());
                llvm::errs() << "\"\n";
            }

            return;
        }

        /* C standard library functions */
        if (!Config.Mocking.MockCStdLib) {
            if (Config.General.Verbose) {
                llvm::errs() << util::cl::info()
                             << "skipping C standard library function \"";
                Decl->printQualifiedName(llvm::errs());
                llvm::errs() << "\"\n";
            }

            return;
        }
    }

    Buffer_.clear();
    Decl->printQualifiedName(OS);

    if (Blacklist_.count(Buffer_)) {
        if (Config.General.Verbose) {
            llvm::errs() << util::cl::info() << "skipping \"" << Buffer_
                         << "\" due to blacklist entry\n";
        }
        return;
    }

    for (const auto &[Pattern, Glob] : GlobBlacklist_) {
        if (Glob.match(Buffer_)) {
            if (Config.General.Verbose) {
                llvm::errs()
                    << util::cl::info() << "skipping \"" << Buffer_
                    << "\" due to blacklist entry \"" << Pattern << "\"\n";
            }

            return;
        }
    }

    if (isVisited(Decl))
        return;

    if (Decl->isVariadic()) {
        if (!Config.Mocking.MockVariadicFunctions)
            return;

        if (Decl->param_empty()) {
            llvm::errs() << util::cl::error()
                         << "unable to mock variadic function \"";
            Decl->printQualifiedName(llvm::errs());
            llvm::errs() << "\" with no parameters\n";

            std::exit(EXIT_FAILURE);
        }
    }

    Generator_->addDecl(Decl);
}

bool ASTVisitor::isVisited(const clang::DeclaratorDecl *Decl)
{
    auto [It, Ok] = Visited_.insert(Decl->getID());
    if (!Ok && It == Visited_.end()) {
        llvm::errs() << util::cl::error() << ": ";
        Decl->printQualifiedName(llvm::errs());
        llvm::errs() << ": failed to mark declaration as visited\n";

        std::exit(EXIT_FAILURE);
    }

    return !Ok;
}

void ASTVisitor::doVisitCallExpr(const clang::CallExpr *CallExpr)
{
    const auto *Decl = CallExpr->getDirectCallee();

    dispatch(CallExpr, Decl);
}

void ASTVisitor::doVisitCXXConstructExpr(
    const clang::CXXConstructExpr *ConstructExpr)
{
    const auto *Decl = ConstructExpr->getConstructor();

    dispatch(ConstructExpr, Decl);
    dispatch(ConstructExpr, Decl->getParent()->getDestructor());
}

void ASTVisitor::doVisitDeclRefExpr(const clang::DeclRefExpr *DeclRefExpr)
{
    llvm::raw_string_ostream OS(Buffer_);

    const auto *Decl = clang::dyn_cast<clang::VarDecl>(DeclRefExpr->getDecl());
    if (!Decl)
        return;

    if (!SourceManager_->isInMainFile(DeclRefExpr->getExprLoc()))
        return;

    if (Decl->getDefinition())
        return;

    Buffer_.clear();
    Decl->printQualifiedName(OS);

    if (InternalBlacklist.count(Buffer_)) {
        if (getConfig().General.Verbose) {
            llvm::errs() << util::cl::info() << "skipping \"" << Buffer_
                         << "\" due to internal blacklist entry\n";
        }
        return;
    }

    if (Blacklist_.count(Buffer_)) {
        if (getConfig().General.Verbose) {
            llvm::errs() << util::cl::info() << "skipping \"" << Buffer_
                         << "\" due to blacklist entry\n";
        }
        return;
    }

    if (isVisited(Decl))
        return;

    Generator_->addDecl(Decl);
}

} /* namespace */

OutputGenerator::OutputGenerator(std::shared_ptr<const Config> Config,
                                 clang::PrintingPolicy Policy,
                                 llvm::StringRef GeneratorName)
    : ASTConsumer(),
      ASTContext_(nullptr),
      Config_(std::move(Config)),
      Writer_(Policy),
      FunctionDecls_(),
      VarDecls_(),
      Name_(GeneratorName),
      AnyVariadic_(false)
{
    FunctionDecls_.reserve(32);
    VarDecls_.reserve(32);
}

void OutputGenerator::HandleTranslationUnit(clang::ASTContext &Context)
{
    ASTContext_ = &Context;

    /* Collect all undefined function and variable declarations */
    ASTVisitor::run(Context, *this);

    writeFileHeader();
    run();
    write();
}

clang::DiagnosticBuilder
OutputGenerator::diag(llvm::StringRef Description,
                      clang::DiagnosticIDs::Level Level)
{
    auto &Diags = ASTContext_->getDiagnostics();

    auto ID = Diags.getDiagnosticIDs()->getCustomDiagID(Level, Description);

    return Diags.Report(ID);
}

clang::DiagnosticBuilder
OutputGenerator::diag(clang::SourceLocation Loc,
                      llvm::StringRef Description,
                      clang::DiagnosticIDs::Level Level)
{
    auto &Diags = ASTContext_->getDiagnostics();

    auto ID = Diags.getDiagnosticIDs()->getCustomDiagID(Level, Description);

    return Diags.Report(Loc, ID);
}

void OutputGenerator::writeFileHeader()
{
    Writer_.write(
        "/*\n"
        " * Generated by ccmock " CCMOCK_VERSION_CORE " <" CCMOCK_WEBSITE ">\n"
        " *\n"
    );
    /* clang-format on */

    if (Config_->General.WriteDate) {
        /* Looks like I do not understand std::chrono... */
        constexpr std::size_t buf_size = 64;
        auto buf = std::array<char, buf_size>();
        struct tm tm = ::tm();

        auto Now = std::time(nullptr);
        (void) gmtime_r(&Now, &tm);

        auto size = std::strftime(buf.data(), std::size(buf), "%FT%T%z", &tm);

        Writer_.write(" *    Date        : ");
        Writer_.write(llvm::StringRef(buf.data(), size));
        Writer_.write("\n");
    }

    auto &Base = Config_->General.BaseDirectory;
    auto Input = std::filesystem::relative(Config_->General.Input, Base);
    auto Output = std::filesystem::relative(Config_->General.Output, Base);

    if (Output.empty())
        Output = "-";

    Writer_.write(" *    Backend     : ");
    Writer_.write(Name_);
    Writer_.write("\n");

    Writer_.write(" *    Directory   : ");
    Writer_.write(Base);
    Writer_.write("\n");

    Writer_.write(" *    Input       : ");
    Writer_.write(Input);
    Writer_.write("\n");

    Writer_.write(" *    Output      : ");
    Writer_.write(Output);
    Writer_.write("\n");

    Writer_.write(" */\n\n");
}

void OutputGenerator::writeMacroDefinitions()
{
    getWriter().write("#ifdef __cplusplus\n"
                      "#define CCMOCK_LINKAGE extern \"C\"\n"
                      "#else\n"
                      "#define CCMOCK_LINKAGE\n"
                      "#endif\n"
                      "\n");
}

void OutputGenerator::writeGlobalVariables()
{
    /* FIXME: make this disablable with a config value */
    for (const auto *Decl : getVarDecls()) {
        auto Type = Decl->getType();

        /*
         * These variable declarations here will all be declared with
         * "extern" so we have to remove it. Also some might have
         * gcc attributes assigned to them which also need to be removed.
         * To make this work with function pointer definitions, it is
         * the easiest to create a new VarDecl and let clang to the printing.
         */
        auto Pointee = Type->getPointeeType();
        if (Pointee.isNull() || !Pointee->isFunctionType()) {
            Writer_.writeType(Type);
            Writer_.write(" ");
            Writer_.write(Decl->getName());
            Writer_.write(";\n");
            continue;
        }

        auto &Context = Decl->getASTContext();
        auto *VarDecl = util::decl::fakeVarDecl(Context, Type, Decl->getName());
        Writer_.writeType(VarDecl);
        Writer_.write(";\n");
    }

    if (!getVarDecls().empty())
        Writer_.write("\n");
}

void OutputGenerator::write()
{
    auto &Path = getConfig().General.Output.native();
    std::error_code error;

    if (Path.empty()) {
        Writer_.flush(llvm::outs());
        return;
    }

    auto Out = llvm::raw_fd_ostream(Path, error);
    if (error) {
        llvm::errs() << util::cl::error() << "failed to open \""
                     << Config_->General.Output.native()
                     << "\": " << error.message() << "\n";
        std::exit(EXIT_FAILURE);
    }

    Writer_.flush(Out);
}
