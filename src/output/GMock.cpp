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

#include "GMock.hpp"
#include "util/Decl.hpp"

#include <clang/AST/ASTContext.h>
#include <clang/AST/DeclLookups.h>

namespace {

inline bool requiresPointerVariable(const clang::DeclContext *Context, 
                                const llvm::DenseSet<const clang::DeclContext *> &Children)
{
    if (!util::decl::isGlobalContext(Context))
        return false;

    if (!Context->isTranslationUnit())
        return true;

    return util::decl::containsFunctionDecls(Children);
}

} /* namespace */

GMock::GMock(std::shared_ptr<const Config> Config, clang::PrintingPolicy Policy)
    : OutputGenerator(std::move(Config), Policy, "gmock"),
      ContextMap_()
{
    /* There is not much choice here as the gmock library is written in C++ */
    getWriter().getPrintingPolicy().SuppressTagKeyword = true;
    getWriter().getPrintingPolicy().Bool = true;
    getWriter().getPrintingPolicy().UseVoidForZeroParams = false;
}

void GMock::run()
{
    initializeContextMap();

    writeIncludeDirectives();
    writeMacroDefinitions();
    writeMockClass();
    writePointerDefinitions();
    writeFixture();

    writeMockFunctions();
    writeGlobalVariables();
    writeMain();
}

void GMock::initializeContextMap()
{
    ContextMap_ = createContextMap();
}

void GMock::writeIncludeDirectives()
{

    if (anyVariadic()) {
        getWriter().write("\n"
                          "#include <cstdarg>\n"
                          "\n");
    }


    getWriter().write("\n"
                      "#include <gmock/gmock.h>\n"
                      "#include <gtest/gtest.h>\n"
                      "\n");
}

void GMock::writeMockClass()
{
    writeMockClass(getASTContext().getTranslationUnitDecl());
}

void GMock::writeMockClass(const clang::DeclContext *Context, unsigned int Indent)
{
    llvm::StringRef Name;

    switch (Context->getDeclKind()) {
    case clang::Decl::TranslationUnit:
        writeMockClass(Context, getConfig().ClassName, Indent);
        break;
    case clang::Decl::Namespace:
    case clang::Decl::CXXRecord:
        Name = clang::cast<clang::NamedDecl>(Context)->getName();

        writeMockClass(Context, Name, Indent);
        break;
    case clang::Decl::Function:
    case clang::Decl::CXXConstructor:
    case clang::Decl::CXXDestructor:
    case clang::Decl::CXXConversion:
    case clang::Decl::CXXMethod:
        writeMockMethod(Context, Indent);

        break;
    default:
        break;
    }
}

void GMock::writeMockClass(const clang::DeclContext *Context, llvm::StringRef Name, unsigned int Indent)
{
    bool IsGlobalContext = util::decl::isGlobalContext(Context);
    const auto &Children = ContextMap_[Context];

    getWriter().indent(Indent);
    getWriter().write("class");

    if (Context->isTranslationUnit()) {
        getWriter().write(" ");
        getWriter().write(Name);
    } else if (IsGlobalContext) {
        getWriter().write(" ");
        getWriter().write(Name);
        getWriter().write("_");
    }

    getWriter().write(" {\n");
    getWriter().indent(Indent);
    getWriter().write("public:\n");

    for (const auto &Child : Children)
        writeMockClass(Child, Indent + 4);

    if (requiresPointerVariable(Context, Children)) 
        writeMockPointerInstance(Context, Indent + 4);

    getWriter().indent(Indent);
    getWriter().write("}");

    if (!IsGlobalContext) {
        getWriter().write(" ");
        getWriter().write(Name);
    }

    getWriter().write(";\n\n");
}

void GMock::writePointerDefinitions()
{
    /* 
     * Example:
     *      thread_local 
     *      testing::StrictMock<DeclName> *DeclName::ccmock_ptr = nullptr;
     */

    for (const auto &[Context, Children] : ContextMap_) {
        if (!requiresPointerVariable(Context, Children))
            continue;

        getWriter().write("thread_local testing::");
        getWriter().write(getConfig().MockType);
        getWriter().write("<");
        writeQualifiedMockDeclarationName(Context);
        getWriter().write("> *");
        writeQualifiedMockDeclarationName(Context);
        getWriter().write("::");
        writeConfigPointerName();
        getWriter().write(" = nullptr;\n");
    }
}

void GMock::writeFixture()
{
    /*
     * Example:
     *      class TestFixture : public testing::Test {
     *      protected:
     *          void SetUp() override {
     *              ccmock_::ns_::c_::ptr_ = &(*this).ns.c;
     *          }
     *
     *          testing::StrictMock<ccmock::mock_> mock;
     *      };
     */
    getWriter().write("\n"
                 "class ");
    getWriter().write(getConfig().TestFixtureName);
    getWriter().write(" : public testing::Test {\n"
                 "protected:\n");
    writeFixtureSetUpFunction();
    getWriter().write("\n");
    writeFixtureVariables();

    /* Close class declaration */
    getWriter().write("};\n\n");
}
    
void GMock::writeFixtureSetUpFunction()
{
    constexpr unsigned int Indent = 8;

    getWriter().write("    void SetUp() override {\n");

    /* 
     * Generate assignments from class variables to the respective pointer 
     * variables.
     */

    for (const auto &[Context, Children] : ContextMap_) {
        /*
         * Keys in 'ContextMap' can only be 'TranslationUnitDecls', 
         * 'NamespaceDecls' and 'CXXRecordDecls' but only the latter two
         * have a name.
         */

        if (!requiresPointerVariable(Context, Children))
            continue;

        writeMockPointerAccess(Context, Indent);
    
        getWriter().write(" = &");
        writeFixtureVariableAccess(Context);
        getWriter().write(";\n");
    }

    getWriter().write("    }\n");
}
    
void GMock::writeFixtureVariables()
{
    constexpr unsigned int Indent = 4;

    for (const auto &[Context, Children] : ContextMap_) {
        if (!requiresPointerVariable(Context, Children))
            continue;

        getWriter().indent(Indent);
        getWriter().write("testing::");
        getWriter().write(getConfig().MockType);
        getWriter().write("<");
        writeQualifiedMockDeclarationName(Context);
        getWriter().write("> ");

        if (Context->isTranslationUnit())
            getWriter().write("_");
        else
            getWriter().write(clang::cast<clang::NamedDecl>(Context)->getName());

        getWriter().write(";\n");
    }
}

void GMock::writeFixtureVariableAccess(const clang::DeclContext *Context)
{
    getWriter().write("(*this)");

    if (Context->isTranslationUnit()) {
        getWriter().write(".");
        getWriter().write(getConfig().GlobalNamespaceName);
        return;
    }

    constexpr size_t Size = 8;
    llvm::SmallVector<const clang::DeclContext *, Size> Vec;

    util::decl::collectAllContexts(Context, Vec);

    /* We don't need the 'TranslationUnitDecl' */
    Vec.pop_back();

    for (const auto *Context : llvm::reverse(Vec)) {
        const auto *Decl = clang::cast<clang::NamedDecl>(Context);
        
        getWriter().write(".");
        getWriter().write(Decl->getName());
    }
}



void GMock::writeMockFunctions()
{
    for (const auto &Decl : getFunctionDecls())
        writeFunction(Decl);
}

void GMock::writeMain()
{
    if (!getConfig().WriteMain)
        return;

    getWriter().write("int main(int argc, char *argv[])\n"
                      "{\n"
                      "    testing::InitGoogleTest(&argc, argv);\n"
                      "\n"
                      "    return RUN_ALL_TESTS();\n"
                      "}\n\n");
}


void GMock::writeMockMethod(const clang::DeclContext *Context, unsigned int Indent)
{
    const auto *Decl = clang::cast<clang::FunctionDecl>(Context);

    getWriter().indent(Indent);
    getWriter().write("MOCK_METHOD((");

    if (util::decl::hasReturnType(Decl))
        getWriter().writeReturnType(Decl);
    else
        getWriter().write("void");

    getWriter().write("), ");
    getWriter().writeMockName(Decl);
    getWriter().write(", ");
    getWriter().writeFunctionParameterList(Decl,
                                           /* ParameterNames */ false,
                                           /* VarArgList */ true);
    getWriter().write(");\n");
}
    
void GMock::writeMockPointerInstance(const clang::DeclContext *Context, unsigned int Indent)
{
    /* 
     * Example:
     *      static thread_local 
     *      testing::StrictMock<ConfigClassName> *ccmock_ptr;
     */
    getWriter().indent(Indent);
    getWriter().write("static thread_local testing::");
    getWriter().write(getConfig().MockType);
    getWriter().write("<");

    if (Context->isTranslationUnit()) {
        getWriter().write(getConfig().ClassName);
    } else {
        getWriter().write(clang::cast<clang::NamedDecl>(Context)->getName());
        getWriter().write("_");
    }

    getWriter().write("> *");
    writeConfigPointerName();
    getWriter().write(";\n");
}

void GMock::writeMockPointerAccess(const clang::DeclContext *Context, unsigned int Indent)
{
    constexpr size_t Size = 8;
    llvm::SmallVector<const clang::DeclContext *, Size> Vec;

    util::decl::collectAllContexts(Context, Vec);

    /* We don't need the 'TranslationUnitDecl' */
    Vec.pop_back();

    getWriter().indent(Indent);
    getWriter().write(getConfig().ClassName);

    for (const auto *Item : llvm::reverse(Vec)) {
        const auto *Decl = clang::cast<clang::NamedDecl>(Item);

        /* FIXME: duplicate code */
        getWriter().write("::");
        getWriter().write(Decl->getName());
        getWriter().write("_");
    }

    getWriter().write("::");
    writeConfigPointerName();
}

void GMock::writeQualifiedMockDeclarationName(const clang::DeclContext *Context)
{
    constexpr size_t Size = 8;
    llvm::SmallVector<const clang::DeclContext *, Size> Vec;

    util::decl::collectAllContexts(Context, Vec);
    
    /* We don't need the 'TranslationUnitDecl' */
    Vec.pop_back();

    getWriter().write(getConfig().ClassName);

    for (const auto *Item : llvm::reverse(Vec)) {
        const auto *Decl = clang::cast<clang::NamedDecl>(Item);

        /* FIXME: duplicate code */
        getWriter().write("::");
        getWriter().write(Decl->getName());
        getWriter().write("_");
    }
}

void GMock::writeFunction(const clang::FunctionDecl *Decl)
{
    if (Decl->isExternC())
        getWriter().write("CCMOCK_LINKAGE ");

    if (util::decl::hasReturnType(Decl)) {
        getWriter().writeReturnType(Decl);
        getWriter().write("\n");
    }

    getWriter().writeFullyQualifiedName(Decl);
    getWriter().writeFunctionParameterList(Decl);
    writeFunctionSpecifiers(Decl);
    writeFunctionReferenceQualifiers(Decl);
    getWriter().write("\n");
    writeFunctionBody(Decl);
    getWriter().write("\n");
}

void GMock::writeMockCall(const clang::FunctionDecl *Decl)
{
    writeMockCallPointerAccess(Decl);

    getWriter().writeMockName(Decl);
    getWriter().write("(");

    /* Write out required function arguments. */
    auto Parameters = Decl->parameters();

    for (unsigned int i = 0, Size = Parameters.size(); i < Size; ++i) {
        if (i != 0)
            getWriter().write(", ");

        bool useMove = Parameters[i]->getType()->isRValueReferenceType();

        if (useMove)
            getWriter().write("std::move(");

        if (!Parameters[i]->getName().empty()) {
            getWriter().write(Parameters[i]->getName());
        } else {
            getWriter().write("arg");
            getWriter().write(i + 1);
        }

        if (useMove)
            getWriter().write(")");
    }

    if (Decl->isVariadic())
        getWriter().write(", vargs_");

    getWriter().write(");\n");
}
    
void GMock::writeMockCallPointerAccess(const clang::FunctionDecl *Decl)
{
    /*
     * Example for translation unit:
     *      (*ccmock_::ccmock_ptr).
     * Examples for namespaces and classes:
     *      (*ccmock_::Namespace_::Class::ccmock_ptr).
     *      (*ccmock_::Class1_::Class2::ccmock_ptr).
     */
    getWriter().write("(*");
    getWriter().write(getConfig().ClassName);

    if (Decl->getParent()->isTranslationUnit()) {
        getWriter().write("::");
        writeConfigPointerName();
        getWriter().write(").");

        return;
    } 

    constexpr size_t Size = 8;
    llvm::SmallVector<const clang::DeclContext *, Size> Vec;

    util::decl::collectAllContexts(Decl->getParent(), Vec);
    
    /* We don't need the 'TranslationUnitDecl' */
    Vec.pop_back();
 
    for (const auto *Item : llvm::reverse(Vec)) {
        const auto *Decl = clang::cast<clang::NamedDecl>(Item);
        const auto Name = Decl->getName();

        if (util::decl::isGlobalContext(Item)) {
            getWriter().write("::");
            getWriter().write(Name);
            getWriter().write("_::");

            writeConfigPointerName();
            getWriter().write(")");

            continue;
        }

        getWriter().write(".");
        getWriter().write(Name);
    }

    getWriter().write(".");
}

void GMock::writeFunctionBody(const clang::FunctionDecl *Decl)
{
    /*
     * Example output for the non-variadic scenario:
     *  {
     *      return mock.func(arg1, arg2);
     *  }
     */
    if (!Decl->isVariadic()) {
        getWriter().write("{\n"
                          "    ");
        if (!Decl->getReturnType()->isVoidType())
            getWriter().write("return ");

        writeMockCall(Decl);

        getWriter().write("}\n");

        return;
    }

    /* Example output for the variadic scenario:
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
    getWriter().write("{\n"
              "    va_list vargs_;\n"
              "\n"
              "    va_start(vargs_, ");

    auto Parameters = Decl->parameters();

    const auto *ParmVarDecl = Parameters.back();

    if (!ParmVarDecl->getName().empty())
        getWriter().write(ParmVarDecl->getName());
    else {
        getWriter().write("arg");
        getWriter().write(Parameters.size());
    }

    getWriter().write(");\n"
                      "    ");
    if (!Decl->getReturnType()->isVoidType())
        getWriter().write("auto ccmock_val_ = ");

    writeMockCall(Decl);

    getWriter().write("    va_end(vargs_);\n");

    if (!Decl->getReturnType()->isVoidType()) {
        getWriter().write("\n"
                          "    return ccmock_val_;\n");
    }

    /* Close the function body */
    getWriter().write("}\n");
}

void GMock::writeFunctionSpecifiers(const clang::FunctionDecl *Decl)
{
    /* FIXME: move to OutputWriter */
    const auto *MethodDecl = clang::dyn_cast<clang::CXXMethodDecl>(Decl);
    if (!MethodDecl)
        return;

    auto QualType = MethodDecl->getType();
    const auto *FuncProtoType = QualType->castAs<clang::FunctionProtoType>();

    if (FuncProtoType->isConst())
        getWriter().write(" const");

    /* 
     * Seems like destructors can get the noexcept attribute attached 
     * automatically which can cause a warning if we manually attach it
     * to the definition.
     */
    bool IsNoexcept = FuncProtoType->hasNoexceptExceptionSpec();
    if (IsNoexcept && Decl->getExceptionSpecSourceRange().isValid())
        getWriter().write(" noexcept");
}

void GMock::writeFunctionReferenceQualifiers(const clang::FunctionDecl *Decl)
{
    /* FIXME: move to OutputWriter */
    const auto *MethodDecl = clang::dyn_cast<clang::CXXMethodDecl>(Decl);
    if (!MethodDecl)
        return;

    switch (MethodDecl->getRefQualifier()) {
    case clang::RefQualifierKind::RQ_LValue:
        getWriter().write(" &");
        break;
    case clang::RefQualifierKind::RQ_RValue:
        getWriter().write(" &&");
        break;
    case clang::RefQualifierKind::RQ_None:
    default:
        break;
    }
}

const Config::GMockSection &GMock::getConfig() const
{
    return OutputGenerator::getConfig().GMock;
}

void GMock::writeConfigPointerName()
{
    llvm::StringRef Name = getConfig().ClassName;

    getWriter().write(Name.rtrim('_'));
    getWriter().write("_ptr_");
}

