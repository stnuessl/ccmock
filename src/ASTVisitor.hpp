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

#ifndef AST_VISITOR_HPP_
#define AST_VISITOR_HPP_

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <clang/AST/RecursiveASTVisitor.h>
#include <llvm/Support/GlobPattern.h>

#include "Config.hpp"

class ASTVisitor : public clang::RecursiveASTVisitor<ASTVisitor> {
public:
    ASTVisitor();

    inline void setSourceManager(clang::SourceManager *SourceManager);
    void setConfig(std::shared_ptr<const Config> Config);

    bool VisitCallExpr(clang::CallExpr *CallExpr);
    bool VisitCXXConstructExpr(clang::CXXConstructExpr *ConstructExpr);

    inline bool shouldWalkTypesOfTypeLocs() const;

    std::vector<const clang::FunctionDecl *> takeFunctionDecls();

private:
    void dispatch(const clang::FunctionDecl *FunctionDecl);

    void doVisitCallExpr(const clang::CallExpr *CallExpr);
    void doVisitCXXConstructExpr(const clang::CXXConstructExpr *ConstructExpr);

    std::string Buffer_;
    std::shared_ptr<const Config> Config_;

    std::vector<std::pair<llvm::StringRef, llvm::GlobPattern>> GlobBlacklist_;
    std::unordered_map<int64_t, const clang::FunctionDecl *> FunctionDeclMap_;
    std::unordered_set<std::string> Blacklist_;
    clang::SourceManager *SourceManager_;
};

inline void ASTVisitor::setSourceManager(clang::SourceManager *SourceManager)
{
    SourceManager_ = SourceManager;
}

inline bool ASTVisitor::shouldWalkTypesOfTypeLocs() const
{
    return false;
}

#endif /* AST_VISITOR_HPP_ */
