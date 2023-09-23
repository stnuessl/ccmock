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

#ifndef GMOCK_HPP_
#define GMOCK_HPP_

#include <llvm/ADT/DenseSet.h>

#include "OutputGenerator.hpp"

class GMock : public OutputGenerator {
public:
    GMock(std::shared_ptr<const Config> Config, clang::PrintingPolicy Policy);

    void run() override;

private:
    void initializeContextMap();

    void writeIncludeDirectives();

    void writeMockClass();
    void writeMockClass(const clang::DeclContext *Context,
                        unsigned int Indent = 0);
    void writeMockClass(const clang::DeclContext *Context,
                        llvm::StringRef Name,
                        unsigned int Indent = 0);

    void writePointerDefinitions();

    void writeFixture();
    void writeFixtureSetUpFunction();
    void writeFixtureVariables();
    void writeFixtureVariableAccess(const clang::DeclContext *Context);

    void writeMockFunctions();
    void writeMain();

    void writeMockMethod(const clang::DeclContext *Context,
                         unsigned int Indent = 0);
    void writeMockPointerInstance(const clang::DeclContext *Context,
                                  unsigned int Indent = 0);
    void writeMockPointerAccess(const clang::DeclContext *Context,
                                unsigned int Indent = 0);

    void writeQualifiedMockDeclarationName(const clang::DeclContext *Context);

    void writeFunction(const clang::FunctionDecl *Decl);
    void writeMockCall(const clang::FunctionDecl *Decl);
    void writeMockCallPointerAccess(const clang::FunctionDecl *Decl);
    void writeFunctionBody(const clang::FunctionDecl *Decl);
    void writeFunctionSpecifiers(const clang::FunctionDecl *Decl);
    void writeFunctionReferenceQualifiers(const clang::FunctionDecl *Decl);

    const Config::GMockSection &getConfig() const;
    void writeConfigPointerName();

    /* clang-format off */
    llvm::DenseMap<
        const clang::DeclContext *, 
        llvm::DenseSet<const clang::DeclContext *>
    > ContextMap_;
    /* clang-format on */
};

#endif /* GMOCK_HPP_ */
