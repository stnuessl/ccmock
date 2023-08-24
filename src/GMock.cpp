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
    : Generator(std::move(Config), Policy, "gmock")
{
    /* There is not much choice here as the gmock library is written in C++ */
    PrintingPolicy_.SuppressTagKeyword = true;
    PrintingPolicy_.Bool = true;
    PrintingPolicy_.UseVoidForZeroParams = false;
}

void GMock::run()
{
    // FIXME: clean up after major refactoring
    std::vector<const clang::FunctionDecl *> Vec;

    for (const auto &[_, DeclVec] : FuncDeclMap_)
        Vec.insert(Vec.end(), DeclVec.begin(), DeclVec.end());

    for (const auto *Decl : Vec) {
        const auto *Context = clang::cast<clang::DeclContext>(Decl);
        const auto *Parent = Decl->getParent();

        while (Parent) {
            ContextMap[Parent].insert(Context);

            Context = Parent;
            Parent = Parent->getParent();
        }
    }

    writeIncludeDirectives();
    writeMacroDefinitions();
    writeMockClass();
    writePointerDefinitions();
    writeFixture();

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

void GMock::writeMockClass()
{
    const clang::TranslationUnitDecl *TUDecl = nullptr;

    // FIXME: Cleanup after making astcontext available for the generators */
    if (!ContextMap.empty()) {
        const auto *Decl = clang::dyn_cast<clang::Decl>(ContextMap.begin()->first);
        TUDecl = Decl->getASTContext().getTranslationUnitDecl();

        writeMockClass(TUDecl);

    }
}

void GMock::writeMockClass(const clang::DeclContext *Context, unsigned int Indent)
{
    llvm::StringRef Name;

    switch (Context->getDeclKind()) {
    case clang::Decl::TranslationUnit:
        writeMockClass(Context, Config_->GMock.ClassName, Indent);
        break;
    case clang::Decl::Namespace:
    case clang::Decl::CXXRecord:
        Name = clang::dyn_cast<clang::NamedDecl>(Context)->getName();

        writeMockClass(Context, Name, Indent);
        break;
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
    const auto &Children = ContextMap[Context];

    Out_.indent(Indent);
    Out_ << "class";

    if (Context->isTranslationUnit())
        Out_ << " " << Name;
    else if (IsGlobalContext) 
        Out_ << " " << Name << "_";

    Out_ << " {\n";
    Out_.indent(Indent);
    Out_ << "public:\n";

    for (const auto &Child : Children)
        writeMockClass(Child, Indent + 4);

    if (requiresPointerVariable(Context, Children)) 
        writeMockPointerInstance(Context, Indent + 4);

    Out_.indent(Indent);
    Out_ << "}";

    if (!IsGlobalContext)
        Out_ << " " << Name;

    Out_ << ";\n\n";
}

void GMock::writePointerDefinitions()
{
    for (const auto &[Context, Children] : ContextMap) {
        if (!requiresPointerVariable(Context, Children))
            continue;

        Out_ << "thread_local testing::" << Config_->GMock.MockType << "<";
        writeQualifiedMockDeclarationName(Context);
        Out_ << "> *";
        writeQualifiedMockDeclarationName(Context);
        Out_ << "::";
        writeConfigPointerName();
        Out_ << " = nullptr;\n";
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

    Out_ << "\n"
            "class " 
         << Config_->GMock.TestFixtureName << " : public testing::Test {\n"
            "protected:\n";

    writeFixtureSetUpFunction();
    Out_ << "\n";
    writeFixtureVariables();

    /* Close class declaration */
    Out_ << "};\n";

    Out_ << "\n";
}
    
void GMock::writeFixtureSetUpFunction()
{
    constexpr unsigned int Indent = 8;

    Out_ << "void SetUp() override {\n";

    /* 
     * Generate assignments from class variables to the respective pointer 
     * variables.
     */
    for (const auto &[Context, Children] : ContextMap) {
        /*
         * Keys in 'ContextMap' can only be 'TranslationUnitDecls', 
         * 'NamespaceDecls' and 'CXXRecordDecls' but only the latter two
         * have a name.
         */

        if (!requiresPointerVariable(Context, Children))
            continue;

        writeMockPointerAccess(Context, Indent);
        
        Out_ << " = &";
        writeFixtureVariableAccess(Context);
        Out_ << ";\n";
    }

    Out_ << "    }\n";
}
    
void GMock::writeFixtureVariables()
{
    constexpr unsigned int Indent = 4;

    for (const auto &[Context, Children] : ContextMap) {
        if (!requiresPointerVariable(Context, Children))
            continue;

        Out_.indent(Indent);
        Out_ << "testing::" << Config_->GMock.MockType << "<";

        writeQualifiedMockDeclarationName(Context);
        Out_ << "> ";

        if (Context->isTranslationUnit())
            Out_ << "_";
        else
            Out_ << clang::dyn_cast<clang::NamedDecl>(Context)->getName();

        Out_ << ";\n";
    }
}

void GMock::writeFixtureVariableAccess(const clang::DeclContext *Context)
{
    Out_ << "(*this)";

    if (Context->isTranslationUnit()) {
        Out_ << ".";
        writeConfigGlobalNamespaceName();
        return;
    }

    constexpr size_t Size = 8;
    llvm::SmallVector<const clang::DeclContext *, Size> Vec;

    util::decl::collectAllContexts(Context, Vec);

    /* We don't need the 'TranslationUnitDecl' */
    Vec.pop_back();

    for (const auto *Context : llvm::reverse(Vec)) {
        const auto *Decl = clang::dyn_cast<clang::NamedDecl>(Context);
        
        Out_ << "." << Decl->getName();
    }
}



void GMock::writeMockFunctions()
{
    for (auto &[ContextDecl, FuncDeclVec] : FuncDeclMap_) {
        (void) ContextDecl;

        for (const auto *Decl : FuncDeclVec)
            writeFunction(Decl);
    }
}

void GMock::writeMain()
{
    if (!Config_->GMock.WriteMain)
        return;

    /* clang-format off */
    Out_ <<
R"(int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
)";
    /* clang-format on */
}


void GMock::writeMockMethod(const clang::DeclContext *Context, unsigned int Indent)
{
    const auto *Decl = clang::cast<clang::FunctionDecl>(Context);

    Out_.indent(Indent);
    Out_ << "MOCK_METHOD((";

    if (util::decl::hasReturnType(Decl))
        writeReturnType(Decl);
    else
        Out_ << "void";

    auto Name = Generator::getMockName(Decl);

    Out_ << "), " << Name << ", ";
    writeFunctionParameterList(Decl, /* ParameterNames */ false, /* VarArgList */ true);
    Out_ << ");\n";
}
    
void GMock::writeMockPointerInstance(const clang::DeclContext *Context, unsigned int Indent)
{
    Out_.indent(Indent);
    Out_ << "static thread_local testing::" 
         << Config_->GMock.MockType << "<";

    if (Context->isTranslationUnit())
        writeConfigClassName();
    else
        Out_ << clang::cast<clang::NamedDecl>(Context)->getName() << "_";

    Out_ << "> *";
    writeConfigPointerName();
    Out_ << ";\n";
}

void GMock::writeMockPointerAccess(const clang::DeclContext *Context, unsigned int Indent)
{
    constexpr size_t Size = 8;
    llvm::SmallVector<const clang::DeclContext *, Size> Vec;

    util::decl::collectAllContexts(Context, Vec);

    /* We don't need the 'TranslationUnitDecl' */
    Vec.pop_back();

    Out_.indent(Indent);
    writeConfigClassName();

    for (const auto *Item : llvm::reverse(Vec)) {
        const auto *Decl = clang::dyn_cast<clang::NamedDecl>(Item);

        Out_ << "::" << Decl->getName() << "_";
    }

    Out_ << "::";
    writeConfigPointerName();
}

void GMock::writeQualifiedMockDeclarationName(const clang::DeclContext *Context)
{
    constexpr size_t Size = 8;
    llvm::SmallVector<const clang::DeclContext *, Size> Vec;

    util::decl::collectAllContexts(Context, Vec);
    
    /* We don't need the 'TranslationUnitDecl' */
    Vec.pop_back();

    writeConfigClassName();

    for (const auto *Item : llvm::reverse(Vec)) {
        const auto *Decl = clang::cast<clang::NamedDecl>(Item);

        Out_ << "::" << Decl->getName() << "_";
    }
}

void GMock::writeFunction(const clang::FunctionDecl *Decl)
{
    if (Decl->isExternC())
        Out_ << "CCMOCK_DECL ";

    if (util::decl::hasReturnType(Decl)) {
        writeReturnType(Decl);
        Out_ << "\n";
    }

    writeQualifiedName(Decl);
    writeFunctionParameterList(Decl);
    writeFunctionSpecifiers(Decl);
    writeFunctionReferenceQualifiers(Decl);
    Out_ << "\n";
    writeFunctionBody(Decl);
    Out_ << "\n";
}

void GMock::writeMockCall(const clang::FunctionDecl *Decl)
{
    writeMockCallPointerAccess(Decl);

    Out_ << Generator::getMockName(Decl) << "(";

    /* Write out required function arguments. */
    auto Parameters = Decl->parameters();

    for (unsigned int i = 0, Size = Parameters.size(); i < Size; ++i) {
        if (i != 0)
            Out_ << ", ";

        bool useMove = Parameters[i]->getType()->isRValueReferenceType();

        if (useMove)
            Out_ << "std::move(";

        if (!Parameters[i]->getName().empty())
            Out_ << *Parameters[i];
        else
            Out_ << "arg" << i + 1;

        if (useMove)
            Out_ << ")";
    }

    if (Decl->isVariadic())
        Out_ << ", vargs_";

    Out_ << ");\n";
}
    
void GMock::writeMockCallPointerAccess(const clang::FunctionDecl *Decl)
{
    Out_ << "(*";
    writeConfigClassName();

    if (Decl->getParent()->isTranslationUnit()) {
        Out_ << "::";
        writeConfigPointerName();
        Out_ << ").";

        return;
    } 

    constexpr size_t Size = 8;
    llvm::SmallVector<const clang::DeclContext *, Size> Vec;

    util::decl::collectAllContexts(Decl->getParent(), Vec);
    
    /* We don't need the 'TranslationUnitDecl' */
    Vec.pop_back();
 
    for (const auto *Item : llvm::reverse(Vec)) {
        const auto *Decl = clang::dyn_cast<clang::NamedDecl>(Item);
        const auto Name = Decl->getName();

        if (util::decl::isGlobalContext(Item)) {
            Out_ << "::" << Name << "_::";
            writeConfigPointerName();
            Out_ << ")";

            continue;
        }

        Out_ << "." << Name;
    }

    Out_ << ".";
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
        Out_ << "{\n"
             << "    ";

        if (!Decl->getReturnType()->isVoidType())
            Out_ << "return ";

        writeMockCall(Decl);

        Out_ << "}\n";

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
    Out_ << "{\n"
            "    va_list vargs_;\n"
            "\n"
            "    va_start(vargs_, ";

    auto Parameters = Decl->parameters();

    auto *ParmVarDecl = Parameters.back();
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

void GMock::writeFunctionSpecifiers(const clang::FunctionDecl *Decl)
{
    const auto *MethodDecl = clang::dyn_cast<clang::CXXMethodDecl>(Decl);
    if (!MethodDecl)
        return;

    auto QualType = MethodDecl->getType();
    const auto *FuncProtoType = QualType->castAs<clang::FunctionProtoType>();

    if (FuncProtoType->isConst())
        Out_ << " const";

    /* 
     * Seems like destructors can get the noexcept attribute attached 
     * automatically which can cause a warning if we manually attach it
     * to the definition.
     */
    bool IsNoexcept = FuncProtoType->hasNoexceptExceptionSpec();
    if (IsNoexcept && Decl->getExceptionSpecSourceRange().isValid())
        Out_ << " noexcept";
}

void GMock::writeFunctionReferenceQualifiers(const clang::FunctionDecl *Decl)
{
    const auto *MethodDecl = clang::dyn_cast<clang::CXXMethodDecl>(Decl);
    if (!MethodDecl)
        return;

    switch (MethodDecl->getRefQualifier()) {
    case clang::RefQualifierKind::RQ_LValue:
        Out_ << " &";
        break;
    case clang::RefQualifierKind::RQ_RValue:
        Out_ << " &&";
        break;
    case clang::RefQualifierKind::RQ_None:
    default:
        break;
    }
}

const Config::GMockSection &GMock::getConfig() const
{
    return Config_->GMock;
}

void GMock::writeConfigClassName()
{
    Out_ << getConfig().ClassName;
}

void GMock::writeConfigPointerName()
{
    llvm::StringRef Name = getConfig().ClassName;

    Out_ << Name.rtrim('_')  << "_ptr_";
}

void GMock::writeConfigGlobalNamespaceName()
{
    Out_ << getConfig().GlobalNamespaceName;
}

