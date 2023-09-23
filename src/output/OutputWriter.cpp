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

#include "OutputWriter.hpp"

#include "util/Decl.hpp"

void OutputWriter::writeFunctionParameterList(const clang::FunctionDecl *Decl,
                                              bool ParameterNames,
                                              bool VarArgList)
{
    auto Parameters = Decl->parameters();

    if (Parameters.empty()) {
        if (PrintingPolicy_.UseVoidForZeroParams)
            Out_ << "(void)";
        else
            Out_ << "()";

        return;
    }

    Out_ << "(";

    for (unsigned int i = 0, Size = Parameters.size(); i < Size; ++i) {
        const auto *Decl = Parameters[i];

        if (i != 0)
            Out_ << ", ";

        if (!ParameterNames) {
            writeType(Parameters[i]);
            continue;
        }

        /* Ensure that every parameter will have a name associated to it */
        if (!Parameters[i]->getName().empty()) {
            writeVarDecl(Parameters[i]);
            continue;
        }

        auto Type = Parameters[i]->getType();
        auto Pointee = Type->getPointeeType();

        if (Pointee.isNull() || !Pointee->isFunctionType()) {
            writeType(Type);

            /*
             * We need to be aware of something like "char *const ptr;".
             * Just checking for "isPointerType()" or "isReferenceType()" to
             * determine whether we need to append a space character or not is
             * not sufficient to correctly print such a type declaration.
             */
            if (auto C = Buffer_.back(); C != '&' && C != '*')
                Out_ << " ";

            Out_ << "arg" << i + 1;

            continue;
        }

        /*
         * Function pointers or references can be extremely tricky to print
         * as the name of the corresponding variable is surrounded by its own
         * type, e.g.:
         *      void *(*func)(void (*)(int, int))
         *             ^~~~
         * We therefore create a new VarDecl (which is obviously not part of
         * the parsed source code) with an appropriate name and then let
         * clang's "Decl::print" function do the heavy lifting.
         */
        std::string Name;
        llvm::raw_string_ostream OS(Name);

        OS << "arg" << i + 1;

        auto &Context = Decl->getASTContext();

        const auto *VarDecl = util::decl::fakeVarDecl(Context, Type, Name);
        writeVarDecl(VarDecl);
    }

    if (Decl->isVariadic()) {
        if (VarArgList)
            Out_ << ", va_list";
        else
            Out_ << ", ...";
    }

    Out_ << ")";
}

void OutputWriter::writeMockName(const clang::FunctionDecl *Decl)
{
    switch (Decl->getKind()) {
    case clang::Decl::CXXConstructor:
        Out_ << "constructor";
        return;
    case clang::Decl::CXXDestructor:
        Out_ << "destructor";
        return;
    case clang::Decl::CXXConversion:
        /* FIXME: implement conversion handling */
        return;
    default:
        break;
    }

    switch (Decl->getOverloadedOperator())  {
    case clang::OO_None:
        Out_ << Decl->getName();
        break;
    case clang::OO_New:
        Out_ << "op_new";
        break;
    case clang::OO_Delete:
        Out_ << "op_delete";
        break;
    case clang::OO_Array_New:
        Out_ << "op_array_new";
        break;
    case clang::OO_Array_Delete:
        Out_ << "op_array_delete";
        break;
    case clang::OO_Plus:
        Out_ << "op_plus";
        break;
    case clang::OO_Minus:
        Out_ << "op_minus";
        break;
    case clang::OO_Star:
        Out_ << "op_star";
        break;
    case clang::OO_Slash:
        Out_ << "op_slash";
        break;
    case clang::OO_Percent:
        Out_ << "op_percent";
        break;
    case clang::OO_Caret:
        Out_ << "op_caret";
        break;
    case clang::OO_Amp:
        Out_ << "op_amp";
        break;
    case clang::OO_Pipe:
        Out_ << "op_pipe";
        break;
    case clang::OO_Tilde:
        Out_ << "op_tilde";
        break;
    case clang::OO_Exclaim:
        Out_ << "op_exclaim";
        break;
    case clang::OO_Equal:
        Out_ << "op_equal";
        break;
    case clang::OO_Less:
        Out_ << "op_less";
        break;
    case clang::OO_Greater:
        Out_ << "op_greater";
        break;
    case clang::OO_PlusEqual:
        Out_ << "op_plus_equal";
        break;
    case clang::OO_MinusEqual:
        Out_ << "op_minus_equal";
        break;
    case clang::OO_StarEqual:
        Out_ << "op_star_equal";
        break;
    case clang::OO_SlashEqual:
        Out_ << "op_slash_equal";
        break;
    case clang::OO_PercentEqual:
        Out_ << "op_percent_equal";
        break;
    case clang::OO_CaretEqual:
        Out_ << "op_caret_equal";
        break;
    case clang::OO_AmpEqual:
        Out_ << "op_amp_equal";
        break;
    case clang::OO_PipeEqual:
        Out_ << "op_pipe_equal";
        break;
    case clang::OO_LessLess:
        Out_ << "op_less_less";
        break;
    case clang::OO_GreaterGreater:
        Out_ << "op_greater_greater";
        break;
    case clang::OO_LessLessEqual:
        Out_ << "op_less_less_equal";
        break;
    case clang::OO_GreaterGreaterEqual:
        Out_ << "op_greater_greater_equal";
        break;
    case clang::OO_EqualEqual:
        Out_ << "op_equal_equal";
        break;
    case clang::OO_ExclaimEqual:
        Out_ << "op_exclaim_equal";
        break;
    case clang::OO_LessEqual:
        Out_ << "op_less_equal";
        break;
    case clang::OO_GreaterEqual:
        Out_ << "op_greater_equal";
        break;
    case clang::OO_Spaceship:
        Out_ << "op_spaceship";
        break;
    case clang::OO_AmpAmp:
        Out_ << "op_amp_amp";
        break;
    case clang::OO_PipePipe:
        Out_ << "op_pipe_pipe";
        break;
    case clang::OO_PlusPlus:
        Out_ << "op_plus_plus";
        break;
    case clang::OO_MinusMinus:
        Out_ << "op_minus_minus";
        break;
    case clang::OO_Comma:
        Out_ << "op_comma";
        break;
    case clang::OO_ArrowStar:
        Out_ << "op_arrow_star";
        break;
    case clang::OO_Arrow:
        Out_ << "op_arrow";
        break;
    case clang::OO_Call:
        Out_ << "op_call";
        break;
    case clang::OO_Subscript:
        Out_ << "op_subscript";
        break;
    default:
        llvm_unreachable("unknown overloaded operator");
        break;
    }
}
