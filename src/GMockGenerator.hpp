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

#include "util/string_ostream.hpp"
#include "Config.hpp"

class GMockGenerator : public clang::ASTConsumer {
public:
    GMockGenerator() = default;

    inline void setConfig(std::shared_ptr<const Config> Config);
    inline void setLanguage(clang::Language Language);
    virtual void HandleTranslationUnit(clang::ASTContext &Context) override;

    void dumpMocks(llvm::raw_ostream &os);

private:
    void writeFileHeader(const clang::ASTContext &Context);
    void writeString(llvm::StringRef Str);
    void writeMockDeclStart();
    void writeMockDeclEnd();
    void writeType(const clang::QualType QualType,
                   const clang::PrintingPolicy &Policy);
    void writeFunctionReturnType(const clang::FunctionDecl *Decl);
    void writeDeclName(const clang::NamedDecl *Decl);
    void writeQualifiedName(const clang::NamedDecl *Decl);
    void writeFunctionParameters(const clang::FunctionDecl *Decl,
                                 bool ParameterNames = false);
    void writeFunctionQualifiers(const clang::FunctionDecl *Decl);

    void writeFunctionBody(const clang::FunctionDecl *Decl);

    void
    generateGlobalMocks(const std::vector<const clang::FunctionDecl *> &Vec);
    void generateCXXMocks(const std::vector<const clang::FunctionDecl *> &Vec,
                          const clang::PrintingPolicy);

    std::shared_ptr<const Config> Config_;

    util::string_ostream main_out_;
    std::string TypeBuffer_;
    clang::Language Language_;
    bool UseExternC;
};

inline void GMockGenerator::setConfig(std::shared_ptr<const Config> Config)
{
    Config_ = Config;
}

inline void GMockGenerator::setLanguage(clang::Language Language)
{
    Language_ = Language;
}

#endif /* GMOCK_GENERATOR_HPP_ */
