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

#include "CMocka.hpp"

CMocka::CMocka(std::shared_ptr<const Config> Config,
               clang::PrintingPolicy Policy)
    : Generator(std::move(Config), Policy, "cmocka")
{
}

void CMocka::run()
{
    writeIncludeDirectives();
    writeMacroDefinitions();
    writeGlobalVariables();
    writeMockFunctions();
}

void CMocka::writeIncludeDirectives()
{
    Out_ << R"(
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include <cmocka.h>
)";
}

void CMocka::writeMockFunctions()
{
    for (auto &Decl : Functions_) {
        if (Decl->isExternC())
            Out_ << "CCMOCK_DECL ";

        writeReturnType(Decl);
        Out_ << "\n";
        writeQualifiedName(Decl);
        writeFunctionParameterList(Decl);
        Out_ << "\n";
        writeFunctionBody(Decl);
        Out_ << "\n";
    }
}

void CMocka::writeFunctionBody(const clang::FunctionDecl *Decl)
{
    auto Parameters = Decl->parameters();
    bool OutParams = Config_->CMocka.OutputParameters;

    auto IsOutParam = [](const clang::ParmVarDecl *Decl) {
        auto Type = Decl->getType();
        auto Pointee = Type->getPointeeType();

        if (Pointee.isNull() || Pointee->isVoidType())
            return false;

        if (Pointee->isFunctionType())
            return false;

        if (Pointee.isConstQualified())
            return false;

        return true;
    };

    Out_ << "{\n";

    if (OutParams && llvm::any_of(Parameters, IsOutParam)) {
        Out_ << "    const void *mock_ptr;\n"
             << "\n";
    }

    if (Config_->CMocka.StrictMocks)
        Out_ << "    function_called();\n";

    if (!Parameters.empty()) {
        Out_ << "\n";

        for (int i = 0, N = Parameters.size(); i < N; ++i) {
            if (Parameters[i]->getType()->isPointerType())
                Out_ << "    check_expected_ptr(";
            else
                Out_ << "    check_expected(";

            if (!Parameters[i]->getName().empty())
                Out_ << *Parameters[i];
            else
                Out_ << "arg" << i + 1;

            Out_ << ");\n";
        }
    }

    if (OutParams) {
        Out_ << "\n";

        /* Treat non-const pointer as output parameters */
        for (int i = 0, N = Parameters.size(); i < N; ++i) {
            auto Type = Parameters[i]->getType();

            if (!IsOutParam(Parameters[i]))
                continue;

            Out_ << "    mock_ptr = mock_ptr_type(";
            writeType(Type);
            Out_ << ");\n"
                    "    if (mock_ptr)\n"
                    "        memmove(";

            if (!Parameters[i]->getName().empty())
                Out_ << *Parameters[i];
            else
                Out_ << "arg" << i + 1;

            Out_ << ", mock_ptr, sizeof(*";

            if (!Parameters[i]->getName().empty())
                Out_ << *Parameters[i];
            else
                Out_ << "arg" << i + 1;

            Out_ << "));\n"
                 << "\n";
        }
    }

    auto Type = Decl->getReturnType();
    if (!Type->isVoidType()) {
        Out_ << "\n"
                "    return ";

        if (Type->isPointerType())
            Out_ << "mock_ptr_type(";
        else
            Out_ << "mock_type(";

        writeType(Type);
        Out_ << ");\n";
    }

    Out_ << "}\n";
}
