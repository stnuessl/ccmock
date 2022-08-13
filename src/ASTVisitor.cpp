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

#include "ASTVisitor.hpp"

#include <clang/Basic/Builtins.h>
#include <clang/Basic/SourceManager.h>
#include <llvm/Support/raw_ostream.h>

#include "util/Glob.hpp"
#include "util/commandline.hpp"

static const llvm::StringMap<int> InternalBlacklist = {
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

ASTVisitor::ASTVisitor(std::shared_ptr<const Config> Config,
                       clang::ASTContext &Context)
    : Config_(std::move(Config)),
      Buffer_(),
      GlobBlacklist_(),
      Visited_(64),
      Result_(),
      Blacklist_(32),
      SourceManager_(&Context.getSourceManager())
{
    Buffer_.reserve(256);
    Result_.Decls.reserve(64);

    for (const auto &Name : Config_->Mocking.Blacklist) {
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

#if 0
void ASTVisitor::setConfig(std::shared_ptr<const Config> Config)
{
    Config_ = std::move(Config);

    /* TODO: shamelessly stolen from clang-rename */
      // Check if NewNames is a valid identifier in C++17.
  LangOptions Options;
  Options.CPlusPlus = true;
  Options.CPlusPlus17 = true;
  IdentifierTable Table(Options);
  for (const auto &NewName : NewNames) {
    auto NewNameTokKind = Table.get(NewName).getTokenID();
    if (!tok::isAnyIdentifier(NewNameTokKind)) {
      errs() << "ERROR: new name is not a valid identifier in C++17.\n\n";
      return 1;
    }
  }

}
#endif

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

    if (Decl->isStatic())
        return;

    if (Decl->isInlined())
        return;

    if (Decl->hasBody())
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
    if (Decl->getBuiltinID()) {
        auto Name = Decl->getName();

        /* Non standard library builtins */
        if (Name.startswith("__builtin_") && !Config_->Mocking.MockBuiltins) {
            if (Config_->General.Verbose) {
                llvm::errs() << util::cl::info() << "skipping builtin \""
                             << Name << "\"\n";
            }

            return;
        }

        /* C++ standard library functions */
        if (Decl->isInStdNamespace() && !Config_->Mocking.MockCXXStdLib) {
            if (Config_->General.Verbose) {
                llvm::errs() << util::cl::info()
                             << "skipping C++ standard library function \"";
                Decl->printQualifiedName(llvm::errs());
                llvm::errs() << "\"\n";
            }

            return;
        }

        /* C standard library functions */
        if (!Config_->Mocking.MockCStdLib) {
            if (Config_->General.Verbose) {
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
        if (Config_->General.Verbose) {
            llvm::errs() << util::cl::info() << "skipping \"" << Buffer_
                         << "\" due to blacklist entry\n";
        }
        return;
    }

    for (const auto &[Pattern, Glob] : GlobBlacklist_) {
        if (Glob.match(Buffer_)) {
            if (Config_->General.Verbose) {
                llvm::errs()
                    << util::cl::info() << "skipping \"" << Buffer_
                    << "\" due to blacklist entry \"" << Pattern << "\"\n";
            }

            return;
        }
    }

    if (Decl->isVariadic()) {
        if (!Config_->Mocking.MockVariadicFunctions)
            return;

        if (Decl->param_empty()) {
            llvm::errs() << util::cl::error()
                         << "unable to mock variadic function \"";
            Decl->printQualifiedName(llvm::errs());
            llvm::errs() << "\" with no parameters\n";

            std::exit(EXIT_FAILURE);
        }

        Result_.AnyVariadic = true;
    }

    add(Decl);
}

void ASTVisitor::add(const clang::DeclaratorDecl *Decl)
{
    auto ID = Decl->getID();

    if (Visited_.count(ID))
        return;

    auto [_, ok] = Visited_.insert(ID);
    if (!ok) {
        llvm::errs() << util::cl::error() << ": ";
        Decl->printQualifiedName(llvm::errs());
        llvm::errs() << ": failed to save function for further processing\n";

        std::exit(EXIT_FAILURE);
    }

    Result_.Decls.push_back(Decl);
}

void ASTVisitor::doVisitCallExpr(const clang::CallExpr *CallExpr)
{
    auto Decl = CallExpr->getDirectCallee();

    dispatch(CallExpr, Decl);
}

void ASTVisitor::doVisitCXXConstructExpr(
    const clang::CXXConstructExpr *ConstructExpr)
{
    auto Decl = ConstructExpr->getConstructor();

    dispatch(ConstructExpr, Decl);
}

void ASTVisitor::doVisitDeclRefExpr(const clang::DeclRefExpr *DeclRefExpr)
{
    llvm::raw_string_ostream OS(Buffer_);

    auto Decl = clang::dyn_cast<clang::VarDecl>(DeclRefExpr->getDecl());
    if (!Decl)
        return;

    if (!SourceManager_->isInMainFile(DeclRefExpr->getExprLoc()))
        return;

    if (Decl->getDefinition())
        return;

    Buffer_.clear();
    Decl->printQualifiedName(OS);

    if (InternalBlacklist.count(Buffer_)) {
        if (Config_->General.Verbose) {
            llvm::errs() << util::cl::info() << "skipping \"" << Buffer_
                         << "\" due to internal blacklist entry\n";
        }
        return;
    }

    if (Blacklist_.count(Buffer_)) {
        if (Config_->General.Verbose) {
            llvm::errs() << util::cl::info() << "skipping \"" << Buffer_
                         << "\" due to blacklist entry\n";
        }
        return;
    }

    add(Decl);
}
