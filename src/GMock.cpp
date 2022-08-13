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

#include "GMock.hpp"

#include <clang/AST/ASTContext.h>

#include "util/Decl.hpp"

GMock::GMock(std::shared_ptr<const Config> Config, clang::PrintingPolicy Policy)
    : Generator(std::move(Config), Policy, "gmock")
{
    /* There is not much choice here as the gmock library is written in C++ */
    PrintingPolicy_.adjustForCPlusPlus();
}

void GMock::run()
{
    writeIncludeDirectives();
    writeMacroDefinitions();
    writeMockClasses();
    writeMockInstances();
    writeMockFunctions();
    writeGlobalVariables();
    writeMain();
}

void GMock::writeIncludeDirectives()
{

    if (AnyVariadic_) {
        Out_ << R"(
#include <cstdarg>
)";
    }

    Out_ << R"(
#include <gmock/gmock.h>
#include <gtest/gtest.h>
)";
}

void GMock::writeMockClasses()
{
    Out_ << "class " << Config_->GMock.MockName
         << " {\n"
            "public:\n";

    for (const auto Decl : Functions_) {
        /*
         * TODO: Solution if multiple functions with same name in different
         * namespaces exists.
         */

        /*
         * Example output:
         *      MOCK_METHOD((ReturnType), FunctionName, ((Parameters));
         */
        Out_ << "    MOCK_METHOD((";
        writeReturnType(Decl);
        Out_ << "), " << *Decl << ", ";
        writeMockParameterList(Decl);
        Out_ << ");\n";
    }

    Out_ << "};\n\n";
}

void GMock::writeMockInstances()
{
    auto MockType = llvm::StringRef("StrictMock");
    if (!Config_->GMock.MockType.empty())
        MockType = Config_->GMock.MockType;

    /*
     * Example output:
     *      static testing::StrictMock<MockName> MockName;
     */
    Out_ << "static testing::" << MockType << "<" << Config_->GMock.MockName
         << "> " << Config_->GMock.MockName << ";\n\n";
}

void GMock::writeMockFunctions()
{
    for (const auto Decl : Functions_) {
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

void GMock::writeMain()
{
    if (!Config_->GMock.WriteMain)
        return;

    /* clang-format off */
    Out_ <<
R"(
int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
)";
    /* clang-format on */
}

void GMock::writeMockParameterList(const clang::FunctionDecl *Decl)
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
        if (i != 0)
            Out_ << ", ";

        Parameters[i]->getType().print(Out_, PrintingPolicy_);
    }

    if (Decl->isVariadic())
        Out_ << ", va_list";

    Out_ << ")";
}

void GMock::writeMockCall(const clang::FunctionDecl *Decl)
{
    auto Parameters = Decl->parameters();

    Out_ << Config_->GMock.MockName << "." << *Decl << "(";

    for (unsigned int i = 0, Size = Parameters.size(); i < Size; ++i) {
        if (i != 0)
            Out_ << ", ";

        if (!Parameters[i]->getName().empty())
            Out_ << *Parameters[i];
        else
            Out_ << "arg" << i + 1;
    }

    if (Decl->isVariadic())
        Out_ << ", vargs_";

    Out_ << ");\n";
}

void GMock::writeFunctionBody(const clang::FunctionDecl *Decl)
{
    auto Parameters = Decl->parameters();

    /*
     * Example output:
     *  {
     *      return mock.func(arg1, arg2);
     *  }
     */
    if (!Decl->isVariadic()) {
        Out_ << "{\n"
             << "    ";

        if (!Decl->getReturnType()->isVoidType())
            Out_ << "return ";

        writeMockCall(Decl);

        Out_ << "}\n";

        return;
    }

    /* Example output for variadic mock:
     *  {
     *      va_list vargs_;
     *
     *      va_start(vargs_, arg);
     *      auto value_ = mock.func(arg, vargs_);
     *      va_end(vargs_);
     *
     *      return value_;
     *  }
     */

    Out_ << "{\n"
            "    va_list vargs_;\n"
            "\n"
            "    va_start(vargs_, ";

    auto ParmVarDecl = Parameters.back();
    if (!ParmVarDecl->getName().empty())
        Out_ << *ParmVarDecl;
    else
        Out_ << "arg" << Parameters.size();

    Out_ << ");\n"
         << "    ";

    if (!Decl->getReturnType()->isVoidType())
        Out_ << "auto mock_val = ";

    writeMockCall(Decl);

    Out_ << "    va_end(vargs_);\n";

    if (!Decl->getReturnType()->isVoidType()) {
        Out_ << "\n"
                "    return mock_val;\n";
    }

    Out_ << "}\n";
}
