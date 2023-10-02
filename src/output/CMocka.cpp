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
    : OutputGenerator(std::move(Config), Policy, "cmocka")
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
    getWriter().write("#include <string.h>\n"
                      "#include <stdarg.h>\n"
                      "#include <stddef.h>\n"
                      "#include <setjmp.h>\n"
                      "\n"
                      "#include <cmocka.h>\n"
                      "\n");
}

void CMocka::writeMockFunctions()
{
    auto &Writer = getWriter();

    for (const auto *Decl : getFunctionDecls()) {
        if (Decl->isExternC())
            Writer.write("CCMOCK_LINKAGE ");

        Writer.writeReturnType(Decl);
        Writer.write("\n");
        Writer.writeFullyQualifiedName(Decl);
        Writer.writeFunctionParameterList(Decl);
        Writer.write("\n");
        writeFunctionBody(Decl);
        Writer.write("\n");
    }
}

void CMocka::writeFunctionBody(const clang::FunctionDecl *Decl)
{
    auto Parameters = Decl->parameters();
    auto &Writer = getWriter();

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

    Writer.write("{\n");

    if (getConfig().OutputParameters && llvm::any_of(Parameters, IsOutParam)) {
        Writer.write("    const void *mock_ptr;\n"
                     "\n");
    }

    if (getConfig().StrictMocks)
        Writer.write("    function_called();\n");

    if (!Parameters.empty()) {
        Writer.write("\n");

        for (unsigned int i = 0, N = Parameters.size(); i < N; ++i) {
            if (Parameters[i]->getType()->isPointerType())
                Writer.write("    check_expected_ptr(");
            else
                Writer.write("    check_expected(");

            if (!Parameters[i]->getName().empty()) {
                Writer.write(Parameters[i]->getName());
            } else {
                Writer.write("arg");
                Writer.write(i + 1);
            }

            Writer.write(");\n");
        }
    }

    if (getConfig().OutputParameters) {
        Writer.write("\n");

        /* Treat non-const pointer as output parameters */
        for (unsigned int i = 0, N = Parameters.size(); i < N; ++i) {
            auto Type = Parameters[i]->getType();

            if (!IsOutParam(Parameters[i]))
                continue;

            Writer.write("    mock_ptr = mock_ptr_type(");
            Writer.writeType(Type);
            Writer.write(");\n"
                         "    if (mock_ptr)\n"
                         "        memmove(");

            if (!Parameters[i]->getName().empty()) {
                Writer.write(Parameters[i]->getName());
            } else {
                Writer.write("arg");
                Writer.write(i + 1);
            }

            Writer.write(", mock_ptr, sizeof(*");

            if (!Parameters[i]->getName().empty()) {
                Writer.write(Parameters[i]->getName());
            } else {
                Writer.write("arg");
                Writer.write(i + 1);
            }

            Writer.write("));\n"
                         "\n");
        }
    }

    auto Type = Decl->getReturnType();
    if (!Type->isVoidType()) {
        Writer.write("\n"
                     "    return ");

        if (Type->isPointerType())
            Writer.write("mock_ptr_type(");
        else
            Writer.write("mock_type(");

        Writer.writeType(Type);
        Writer.write(");\n");
    }

    Writer.write("}\n");
}

const Config::CMockaSection &CMocka::getConfig() const
{
    return OutputGenerator::getConfig().CMocka;
}
