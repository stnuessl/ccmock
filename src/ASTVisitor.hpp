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

#ifndef AST_VISITOR_HPP_
#define AST_VISITOR_HPP_

#include <clang/AST/RecursiveASTVisitor.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/Support/GlobPattern.h>
#include <memory>
#include <vector>

#include "Config.hpp"

class ASTVisitor : public clang::RecursiveASTVisitor<ASTVisitor> {
public:
    struct Result {
    public:
        Result() = default;

        llvm::DenseMap<
            const clang::DeclContext *, 
            std::vector<const clang::FunctionDecl *>
        > FuncDeclMap;
        std::vector<const clang::VarDecl *> VarDeclVec;
        bool AnyVariadic;
    };

    static inline Result run(std::shared_ptr<const Config> Config,
                             clang::ASTContext &Context);
                      
    bool VisitCallExpr(clang::CallExpr *CallExpr);
    bool VisitCXXConstructExpr(clang::CXXConstructExpr *ConstructExpr);
    bool VisitDeclRefExpr(clang::DeclRefExpr *DeclRefExpr);

    inline bool shouldWalkTypesOfTypeLocs() const;
private:
    ASTVisitor(std::shared_ptr<const Config> Config,
               clang::ASTContext &Context);

    void dispatch(const clang::Expr *Expr,
                  const clang::FunctionDecl *FunctionDecl);
    bool isVisited(const clang::DeclaratorDecl *Decl);

    void doVisitCallExpr(const clang::CallExpr *CallExpr);
    void doVisitCXXConstructExpr(const clang::CXXConstructExpr *ConstructExpr);
    void doVisitDeclRefExpr(const clang::DeclRefExpr *DeclRefExpr);


    std::shared_ptr<const Config> Config_;
    std::string Buffer_;

    std::vector<std::pair<llvm::StringRef, llvm::GlobPattern>> GlobBlacklist_;
    llvm::DenseSet<int64_t> Visited_;
    Result Result_;
    llvm::StringMap<int> Blacklist_;
    clang::SourceManager *SourceManager_;
};

inline bool ASTVisitor::shouldWalkTypesOfTypeLocs() const
{
    return false;
}

inline ASTVisitor::Result ASTVisitor::run(std::shared_ptr<const Config> Config,
                                           clang::ASTContext &Context)
{
    ASTVisitor Visitor(std::move(Config), Context);

    Visitor.TraverseDecl(Context.getTranslationUnitDecl());

    return std::move(Visitor.Result_);
}

#endif /* AST_VISITOR_HPP_ */
