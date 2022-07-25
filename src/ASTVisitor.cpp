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

#include <llvm/Support/raw_ostream.h>

#include <clang/Basic/Builtins.h>
#include <clang/Basic/SourceManager.h>

#include "ASTVisitor.hpp"
#include "util/Glob.hpp"
#include "util/commandline.hpp"

ASTVisitor::ASTVisitor()
    : Buffer_(),
      Config_(),
      GlobBlacklist_(),
      FunctionDeclMap_(),
      Blacklist_(),
      SourceManager_()
{
    Buffer_.reserve(256);
    FunctionDeclMap_.reserve(256);
    Blacklist_.reserve(64);
}

void ASTVisitor::setConfig(std::shared_ptr<const Config> Config)
{
    Config_ = std::move(Config);

    /* TODO: shamelessly stolen from clang-rename */
#if 0
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
#endif

    for (const auto &Name : Config_->Mocking.Blacklist) {
        if (!util::glob::isPattern(Name)) {
            Blacklist_.insert(Name);
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

std::vector<const clang::FunctionDecl *> ASTVisitor::takeFunctionDecls()
{
    std::vector<const clang::FunctionDecl *> Vector;

    Vector.reserve(FunctionDeclMap_.size());

    for (auto &&Item : FunctionDeclMap_)
        Vector.push_back(std::move(Item.second));

    FunctionDeclMap_.clear();

    return Vector;
}

void ASTVisitor::dispatch(const clang::Expr *Expr,
                          const clang::FunctionDecl *Decl)
{
    llvm::raw_string_ostream OS(Buffer_);

    if (!SourceManager_->isInMainFile(Expr->getExprLoc()))
        return;

    if (!Decl)
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
    auto BuiltinID = Decl->getBuiltinID();
    if (BuiltinID) {
        auto Name = Decl->getName();

        /* Non standard library builtins */
        if (Name.startswith("__builtin_") && !Config_->Mocking.MockBuiltins) {
            if (Config_->General.Verbose) {
                llvm::errs() << util::cl::info() << "ignoring builtin \""
                             << Name << "\"\n";
            }

            return;
        }

        /* C++ standard library functions */
        if (Decl->isInStdNamespace() && !Config_->Mocking.MockCXXStdLib) {
            if (Config_->General.Verbose) {
                llvm::errs() << util::cl::info()
                             << "ignoring C++ standard library function \"";
                Decl->printQualifiedName(llvm::errs());
                llvm::errs() << "\"\n";
            }

            return;
        }

        /* C standard library functions */
        if (!Config_->Mocking.MockCStdLib) {
            if (Config_->General.Verbose) {
                llvm::errs() << util::cl::info()
                             << "ignoring C standard library function \"";
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
            llvm::errs() << util::cl::info() << "ignoring \"" << Buffer_
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
    }

    auto ID = Decl->getID();

    if (FunctionDeclMap_.count(ID))
        return;

    auto [it, ok] = FunctionDeclMap_.insert({ID, Decl});
    if (!ok) {
        llvm::errs() << util::cl::error() << ": ";
        Decl->printQualifiedName(llvm::errs());

        llvm::errs() << ": failed to save function for further processing\n";

        std::exit(EXIT_FAILURE);
    }
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
