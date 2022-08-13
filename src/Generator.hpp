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

#ifndef GENERATOR_HPP_
#define GENERATOR_HPP_

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/PrettyPrinter.h>
#include <clang/AST/Type.h>

#include "Config.hpp"

class Generator : public clang::ASTConsumer {
public:
    Generator(std::shared_ptr<const Config> Config,
              clang::PrintingPolicy Policy,
              llvm::StringRef Backend);

    virtual void HandleTranslationUnit(clang::ASTContext &Context) override;
    virtual void run() = 0;

protected:
    void writeMacroDefinitions();
    void writeGlobalVariables();
    inline void writeType(clang::QualType Type);
    inline void writeReturnType(const clang::FunctionDecl *Decl);
    inline void writeQualifiedName(const clang::FunctionDecl *Decl);
    void writeFunctionParameterList(const clang::FunctionDecl *Decl);

    std::shared_ptr<const Config> Config_;
    clang::PrintingPolicy PrintingPolicy_;
    std::string Buffer_;
    llvm::raw_string_ostream Out_;
    std::vector<const clang::FunctionDecl *> Functions_;
    std::vector<const clang::CXXMethodDecl *> Methods_;
    std::vector<const clang::DeclaratorDecl *> Variables_;

    bool AnyVariadic_;

private:
    void writeFileHeader();
    void write() const;

    llvm::StringRef Backend_;
};

inline void Generator::writeType(clang::QualType Type)
{
    Type.print(Out_, PrintingPolicy_);
}

inline void Generator::writeReturnType(const clang::FunctionDecl *Decl)
{
    writeType(Decl->getReturnType());
}

inline void Generator::writeQualifiedName(const clang::FunctionDecl *Decl)
{
    Decl->printQualifiedName(Out_, PrintingPolicy_);
}

#endif /* GENERATOR_HPP_ */
