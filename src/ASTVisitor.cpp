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

    for (const auto &Name : Config_->Mocking.Blacklist) {
        if (!util::glob::isPattern(Name)) {
            Blacklist_.insert(Name);
            continue;
        }

        auto ExpectedGlob = llvm::GlobPattern::create(Name);
        if (!ExpectedGlob) {
            llvm::errs() << util::cl::error() 
                         << ExpectedGlob.takeError() 
                         << "\n";

            std::exit(EXIT_FAILURE);
        }

        GlobBlacklist_.push_back({ Name, std::move(*ExpectedGlob) });
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

void ASTVisitor::dispatch(const clang::FunctionDecl *Decl)
{
    llvm::raw_string_ostream OS(Buffer_);

    if (!Decl)
        return;

    if (Decl->isStatic())
        return;

    if (Decl->isInlined())
        return;

    if (Decl->hasBody())
        return;

    if (Decl->isInStdNamespace())
        return;

    if (Decl->getBuiltinID() && !Config_->Mocking.MockBuiltins)
        return;

    Buffer_.clear();
    Decl->printQualifiedName(OS);

    if (Blacklist_.count(Buffer_)) {
        if (Config_->General.Verbose) {
            llvm::errs() << util::cl::info() << Buffer_
                         << ": skipping function due to blacklist entry\n";
        }
        return;
    }

    for (const auto &[Pattern, Glob] : GlobBlacklist_) {
        if (Glob.match(Buffer_)) {
            if (Config_->General.Verbose) {
                llvm::errs() << util::cl::info() << Buffer_
                             << ": skipping function due to blacklist entry \""
                             << Pattern << "\"\n";
            }

            return;
        }
    }

    if (Decl->isVariadic()) {
        llvm::errs() << util::cl::error()
                     << "cannot process variadic function \"" << *Decl
                     << "\" - use a blacklist entry to avoid this error\n";
        std::exit(EXIT_FAILURE);
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
    auto Loc = CallExpr->getExprLoc();

    if (!SourceManager_->isInMainFile(Loc))
        return;

    auto Decl = CallExpr->getDirectCallee();

    dispatch(Decl);
}

void ASTVisitor::doVisitCXXConstructExpr(
    const clang::CXXConstructExpr *ConstructExpr)
{
    auto Loc = ConstructExpr->getExprLoc();

    if (!SourceManager_->isInMainFile(Loc))
        return;

    auto ConstructerDecl = ConstructExpr->getConstructor();

    dispatch(ConstructerDecl);
}
