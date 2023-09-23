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

#include "Raw.hpp"

#include "clang/AST/DeclCXX.h"

Raw::Raw(std::shared_ptr<const Config> Config, clang::PrintingPolicy Policy)
    : OutputGenerator(std::move(Config), Policy, "Raw"), 
      CurrentAccess_(clang::AccessSpecifier::AS_none)
{
}

void Raw::run()
{
    ContextMap_ = createContextMap();

    visit(getASTContext().getTranslationUnitDecl());
}

void Raw::visit(const clang::DeclContext *Context, unsigned int Indent)
{
    switch (Context->getDeclKind()) {
    case clang::Decl::TranslationUnit:
        for (const auto &Child : ContextMap_[Context])
            visit(Child, Indent);

        break;
    case clang::Decl::Namespace:
        getWriter().indent(Indent);
        getWriter().write("namespace ");
        getWriter().writeDeclName(Context);
        getWriter().write(" {\n\n");

        for (const auto &Child : ContextMap_[Context])
            visit(Child, Indent);

        getWriter().write("\n} /* namespace ");
        getWriter().writeDeclName(Context);
        getWriter().write(" */\n\n");
        break;
    case clang::Decl::CXXRecord:
        getWriter().indent(Indent);

        switch (clang::cast<clang::CXXRecordDecl>(Context)->getTagKind()) {
        case clang::TagTypeKind::TTK_Class:
            getWriter().write("class ");
            break;
        case clang::TagTypeKind::TTK_Struct:
            getWriter().write("struct ");
            break;
        case clang::TagTypeKind::TTK_Union:
            /* FIXME: Test needed! */
            getWriter().write("union ");
            break;
        default:
            break;
        }

        getWriter().writeDeclName(Context);
        getWriter().write(" {\n");

        for (const auto &Child : ContextMap_[Context])
            visit(Child, Indent + 4);

        getWriter().write("};\n\n");

        CurrentAccess_ = clang::AccessSpecifier::AS_none;
        break;
    case clang::Decl::Function:
    case clang::Decl::CXXConstructor:
    case clang::Decl::CXXDestructor:
    case clang::Decl::CXXConversion:
    case clang::Decl::CXXMethod: {
        const auto *Decl = clang::cast<clang::FunctionDecl>(Context);

        auto Access = Decl->getAccess();
        if (Access != CurrentAccess_) {
            switch (Access) {
            case clang::AccessSpecifier::AS_public:
                getWriter().write("public:\n");
                break;
            case clang::AccessSpecifier::AS_protected:
                getWriter().write("protected:\n");
                break;
            case clang::AccessSpecifier::AS_private:
                getWriter().write("private:\n");
                break;
            default:
                break;
            }

            CurrentAccess_ = Access;
        }

        getWriter().indent(Indent);
        getWriter().writeFunctionDecl(Decl, Indent);
        getWriter().write(";\n");
    }
    default:
        break;
    };
}
