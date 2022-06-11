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

#include <time.h>
#include <iomanip>

#include <clang/Basic/SourceManager.h>
#include <clang/Basic/Version.h>
#include <llvm/Config/llvm-config.h>

#include "ASTVisitor.hpp"
#include "GMockGenerator.hpp"

#include "util/commandline.hpp"
#include "util/Type.hpp"


static const char IncludeHeaderString[] = {
    "#include <gtest/gtest.h>\n"
    "#include <gmock/gmock.h>\n"
    "\n"
};

static const char MainFunctionString[] = {
    "int main(int argc, char *argv[])\n"
    "{\n"
    "   testing::InitGoogleTest(&argc, argv);\n"
    "\n"
    "   return RUN_ALL_TESTS();\n"
    "}\n\n"
};

void GMockGenerator::HandleTranslationUnit(clang::ASTContext &Context)
{
    auto Visitor = ASTVisitor();
    Visitor.setConfig(Config_);
    Visitor.setSourceManager(&Context.getSourceManager());

    auto Printer = Context.getPrintingPolicy();
    Printer.UseVoidForZeroParams = 1;

    Context.setPrintingPolicy(Printer);

    (void) Visitor.TraverseDecl(Context.getTranslationUnitDecl());

    auto Vec = Visitor.takeFunctionDecls();

    auto CFuncDecls = std::vector<const clang::FunctionDecl *>();

    for (const auto Item : Vec) {
        if (!clang::isa<clang::CXXMethodDecl>(Item))
            CFuncDecls.push_back(Item);
    }

    writeFileHeader(Context);
    generateGlobalMocks(CFuncDecls);

    if (!Config_->Output.empty()) {
        std::error_code error;
        auto OS = llvm::raw_fd_stream(Config_->Output, error);
        if (error) {
            llvm::errs() << util::cl::Error() 
                         << "failed to open \"" << Config_->Output
                         << "\": " << error.message() << "\n";
            std::exit(EXIT_FAILURE);
        }
        dumpMocks(OS);
    } else {
        dumpMocks(llvm::outs());
    }
}

void GMockGenerator::dumpMocks(llvm::raw_ostream &os)
{
    os << main_out_.str() << "\n";
}

void GMockGenerator::writeFileHeader(const clang::ASTContext &Context)
{
    auto &SourceManager = Context.getSourceManager();
    auto FileID = SourceManager.getMainFileID();
    auto Entry = SourceManager.getFileEntryForID(FileID);

    main_out_ << "/*\n"
              << " * Automatically generated file\n"
              << " *\n"
              << " * ccmock     : " << CCMOCK_VERSION_CORE << "\n"
              << " * Source     : " << Entry->getName() << "\n"
              << " * Generator  : gmock\n";

    if (Config_->PrintTimestamp) {
        /* Looks like I do not understand std::chrono... */
        char buf[64];
        struct tm tm;

        auto Now = std::time(nullptr);
        gmtime_r(&Now, &tm);

        std::strftime(buf, std::size(buf), "%FT%T%z", &tm);

        main_out_ << " * Date       : " << buf << "\n";
    }

    main_out_ << " */\n\n";

    writeString(IncludeHeaderString);
}

void GMockGenerator::writeString(llvm::StringRef Str)
{
    main_out_ << Str;
}

void GMockGenerator::writeMockDeclStart()
{
    writeString("    MOCK_METHOD(");
}

void GMockGenerator::writeMockDeclEnd()
{
    writeString(");\n");
}

void GMockGenerator::writeType(clang::QualType QualType,
                               const clang::PrintingPolicy &Policy)
{
    QualType.print(main_out_, Policy);
}

void GMockGenerator::writeFunctionReturnType(const clang::FunctionDecl *Decl)
{
    auto &Policy = Decl->getASTContext().getPrintingPolicy();

    Decl->getReturnType().print(main_out_, Policy);
}

void GMockGenerator::writeDeclName(const clang::NamedDecl *Decl)
{
    main_out_ << *Decl;
}

void GMockGenerator::writeQualifiedName(const clang::NamedDecl *Decl)
{
    Decl->printQualifiedName(main_out_);
}

void GMockGenerator::writeFunctionParameters(const clang::FunctionDecl *Decl,
                                             bool ParameterNames)
{
    if (Decl->param_empty()) {
        switch (Language_) {
        case clang::Language::C:
            writeString("void");
            break;
        case clang::Language::CXX:
        default:
            break;
        }

        return;
    }

    auto &Policy = Decl->getASTContext().getPrintingPolicy();

    auto Size = Decl->getNumParams();
    auto Last = Size - 1;

    for (unsigned int i = 0; i < Size; ++i) {
        auto Parameter = Decl->getParamDecl(i);

        if (ParameterNames) {
            TypeBuffer_.clear();
            Parameter->getType().getAsStringInternal(TypeBuffer_, Policy);
            writeString(TypeBuffer_);

            /*
             * Checking if the type is a pointer or a reference does not work
             * here as for example
             *      ...(type *restrict name);
             *          ^~~~~~~~~~~~~^
             *
             * is a pointer type, but it does not tell us if we need to
             * append a space after we've written the type. Instead, we
             * check if we need to add a space after the type was written
             * to the output buffer.
             */
            if (TypeBuffer_.back() != '*' && TypeBuffer_.back() != '&')
                writeString(" ");

            main_out_ << *Parameter;
        } else {
            Parameter->getType().print(main_out_, Policy);
        }

        if (i < Last)
            writeString(", ");
    }
}

void GMockGenerator::writeFunctionQualifiers(const clang::FunctionDecl *Decl)
{
}

void GMockGenerator::writeFunctionBody(const clang::FunctionDecl *Decl)
{
    main_out_ << "{\n"
              << "    return mock." << *Decl << "(";

    auto Size = Decl->getNumParams();
    auto Last = Size - 1;

    for (unsigned int i = 0; i < Size; ++i) {
        main_out_ << *Decl->getParamDecl(i);

        if (i < Last)
            main_out_ << ", ";
    }

    main_out_ << ");\n"
              << "}\n";
}

void GMockGenerator::generateGlobalMocks(
    const std::vector<const clang::FunctionDecl *> &FuncDeclVec)
{
    if (FuncDeclVec.empty())
        return;

    main_out_ << "class mock {\n"
              << "public:\n";

    for (const auto FunctionDecl : FuncDeclVec) {
        if (FunctionDecl->isVariadic()) {
            llvm::errs() << *FunctionDecl << " is variadic!\n";
            continue;
        }
        /* Solution if multiple functions with same name in different namespaces
         * exists. */
        writeMockDeclStart();
        writeString("(");
        writeFunctionReturnType(FunctionDecl);
        writeString("), ");
        writeDeclName(FunctionDecl);
        writeString(", (");
        writeFunctionParameters(FunctionDecl, /* ParameterNames */ false);
        writeString(")");
        writeMockDeclEnd();
    }

    writeString("};\n\n");

    main_out_ << "static testing::" << Config_->MockType << "<mock> mock;\n\n";

    /*
     * TODO: extern C
     */

    for (const auto FunctionDecl : FuncDeclVec) {

        if (FunctionDecl->isExternC())
            writeString("#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n");

        writeFunctionReturnType(FunctionDecl);

        if (!util::type::isPointerOrReference(FunctionDecl->getReturnType()))
            writeString(" ");

        writeQualifiedName(FunctionDecl);
        writeString("(");
        writeFunctionParameters(FunctionDecl, /* ParameterNames */ true);
        writeString(")");
        main_out_ << "\n";
        writeFunctionBody(FunctionDecl);

        writeString("\n");

        if (FunctionDecl->isExternC())
            writeString("#ifdef __cplusplus\n}\n#endif\n\n");
    }

    writeString(MainFunctionString);
}
