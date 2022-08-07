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

#include "Decl.hpp"

#include <clang/AST/ASTContext.h>

namespace util {
namespace decl {

clang::VarDecl *fakeVarDecl(clang::ASTContext &Context,
                            clang::QualType Type,
                            llvm::StringRef Name)
{
    auto TranslationUnitDecl = Context.getTranslationUnitDecl();
    auto Loc = clang::SourceLocation();
    auto &IdInfo = Context.Idents.getOwn(Name);

    return clang::VarDecl::Create(Context,
                                  TranslationUnitDecl,
                                  Loc,
                                  Loc,
                                  &IdInfo,
                                  Type,
                                  nullptr,
                                  clang::StorageClass::SC_None);
} /* namespace decl */
} // namespace decl

} // namespace util
