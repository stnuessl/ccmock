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

#ifndef GMOCK_GENERATOR_HPP_
#define GMOCK_GENERATOR_HPP_

#include <memory>
#include <string>

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/PrettyPrinter.h>
#include <clang/AST/Type.h>

#include <llvm/ADT/StringRef.h>

#include "Config.hpp"

class GMockGenerator : public clang::ASTConsumer {
public:
    GMockGenerator(std::shared_ptr<const Config> Config,
                   clang::PrintingPolicy Policy);

    void setOutputFile(llvm::StringRef File);

    virtual void HandleTranslationUnit(clang::ASTContext &Context) override;

    void dumpMocks();

private:
    void writeCommentHeader();
    void writeIncludeStatements();
    void writeGlobalFunctionMocks(
        const std::vector<const clang::FunctionDecl *> &Vec);
    void writeClassMethodMocks();
    void writeMainFunctionDefinition();

    void writeFunctionParameterList(const clang::FunctionDecl *Decl);
    void writeMockParameterList(const clang::FunctionDecl *Decl);
    void writeMockCall(const clang::FunctionDecl *Decl);
    void writeFunctionBody(const clang::FunctionDecl *Decl);
    void writeFunctionSpecifiers(const clang::FunctionDecl *Decl);

    void
    generateGlobalMocks(const std::vector<const clang::FunctionDecl *> &Vec);
    void generateCXXMocks(const std::vector<const clang::FunctionDecl *> &Vec,
                          const clang::PrintingPolicy);

    std::shared_ptr<const Config> Config_;

    std::string Buffer_;
    llvm::raw_string_ostream Out_;
    clang::PrintingPolicy PrintingPolicy_;
};

#endif /* GMOCK_GENERATOR_HPP_ */
