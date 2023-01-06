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

#ifndef DECL_HPP_
#define DECL_HPP_

#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>

namespace util {
namespace decl {

inline bool isGlobalFunction(const clang::FunctionDecl *Decl)
{
    return !clang::isa<clang::CXXMethodDecl>(Decl) && Decl->isGlobal();
}

clang::VarDecl *fakeVarDecl(clang::ASTContext &Context,
                            clang::QualType Type,
                            llvm::StringRef Name);

} /* namespace decl */
} /* namespace util */

#endif /* DECL_HPP_ */
