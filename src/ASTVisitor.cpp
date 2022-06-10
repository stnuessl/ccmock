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
#include "util/commandline.hpp"

void ASTVisitor::setConfig(std::shared_ptr<const Config> Config)
{
    Config_ = Config;

    for (const auto &Name : Config_->Blacklist) {
        Blacklist_.insert(Name);
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

void ASTVisitor::dispatch(const clang::FunctionDecl *FunctionDecl)
{
    if (!FunctionDecl)
        return;

    if (FunctionDecl->hasBody())
        return;

    if (Blacklist_.count(FunctionDecl->getQualifiedNameAsString())) {
        llvm::errs() << "Ignoring: " << *FunctionDecl << "\n";
        return;
    }
    //
    //    if (FunctionDecl->willHaveBody())
    //       return;
    //
    //    if (FunctionDecl->isDefined())
    //        return;

    auto ID = FunctionDecl->getID();

    if (FunctionDeclMap_.count(ID) != 0)
        return;

    auto [it, ok] = FunctionDeclMap_.insert({ID, FunctionDecl});
    if (!ok) {
        FunctionDecl->printQualifiedName(llvm::errs());
        llvm::errs() << ": " << util::cl::Warning()
                     << "failed to mark for further processing\n";
    }
}

void ASTVisitor::doVisitCallExpr(const clang::CallExpr *CallExpr)
{
    auto Loc = CallExpr->getExprLoc();

    if (!SourceManager_->isInMainFile(Loc))
        return;

    auto FunctionDecl = CallExpr->getDirectCallee();

    if (Config_->MockStandardLibrary) {
        if (SourceManager_->isInSystemHeader(FunctionDecl->getLocation()))
            llvm::errs() << util::cl::Info()
                         << *FunctionDecl << ": is Systemheader\n";
    }

    dispatch(FunctionDecl);
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
