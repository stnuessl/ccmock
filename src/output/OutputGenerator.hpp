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

#ifndef OUTPUTGENERATOR_HPP_
#define OUTPUTGENERATOR_HPP_

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/Decl.h>
#include <memory>
#include <vector>

#include "Config.hpp"
#include "OutputWriter.hpp"

class OutputGenerator : public clang::ASTConsumer {
public:
    OutputGenerator(std::shared_ptr<const Config> Config,
                    clang::PrintingPolicy Policy,
                    llvm::StringRef GeneratorName);

    virtual void run() = 0;
    void HandleTranslationUnit(clang::ASTContext &Context) override;

    inline void addDecl(const clang::FunctionDecl *Decl);
    inline void addDecl(const clang::VarDecl *Decl);

    inline const Config &getConfig() const;

    clang::DiagnosticBuilder
    diag(llvm::StringRef Description,
         clang::DiagnosticIDs::Level Level = clang::DiagnosticIDs::Warning);

    clang::DiagnosticBuilder
    diag(clang::SourceLocation Loc,
         llvm::StringRef Description,
         clang::DiagnosticIDs::Level Level = clang::DiagnosticIDs::Warning);

protected:
    inline const clang::ASTContext &getASTContext() const;
    inline OutputWriter &getWriter();
    inline llvm::ArrayRef<const clang::FunctionDecl *> getFunctionDecls() const;
    inline llvm::ArrayRef<const clang::VarDecl *> getVarDecls() const;
    inline bool anyVariadic() const;

    /* clang-format off */
    inline llvm::DenseMap<
        const clang::DeclContext *,
        llvm::DenseSet<const clang::DeclContext *>
    > createContextMap();
    /* clang-format on */

    void writeFileHeader();
    void writeMacroDefinitions();
    void writeGlobalVariables();

private:
    void write();

    const clang::ASTContext *ASTContext_;
    std::shared_ptr<const Config> Config_;
    OutputWriter Writer_;
    std::vector<const clang::FunctionDecl *> FunctionDecls_;
    std::vector<const clang::VarDecl *> VarDecls_;
    llvm::StringRef Name_;

    bool AnyVariadic_;
};

inline const clang::ASTContext &OutputGenerator::getASTContext() const
{
    return *ASTContext_;
}

inline OutputWriter &OutputGenerator::getWriter()
{
    return Writer_;
}

inline void OutputGenerator::addDecl(const clang::FunctionDecl *Decl)
{
    if (Decl->isVariadic() && !AnyVariadic_)
        AnyVariadic_ = true;

    FunctionDecls_.push_back(Decl);
}

inline void OutputGenerator::addDecl(const clang::VarDecl *Decl)
{
    VarDecls_.push_back(Decl);
}

inline const Config &OutputGenerator::getConfig() const
{
    return *Config_;
}

inline llvm::ArrayRef<const clang::FunctionDecl *>
OutputGenerator::getFunctionDecls() const
{
    return llvm::ArrayRef(FunctionDecls_);
}

inline llvm::ArrayRef<const clang::VarDecl *>
OutputGenerator::getVarDecls() const
{
    return llvm::ArrayRef(VarDecls_);
}

inline bool OutputGenerator::anyVariadic() const
{
    return AnyVariadic_;
}

inline llvm::DenseMap<const clang::DeclContext *,
                      llvm::DenseSet<const clang::DeclContext *>>
OutputGenerator::createContextMap()
{
    auto Size = getFunctionDecls().size();

    /* clang-format off */
    auto Map = llvm::DenseMap<
        const clang::DeclContext *,
        llvm::DenseSet<const clang::DeclContext *>
    >(Size);
    /* clang-format on */

    /*
     * Create a mapping of all used declaration contexts to their respective
     * child contexts.
     */
    for (const auto *Decl : getFunctionDecls()) {
        const auto *Context = clang::cast<clang::DeclContext>(Decl);
        const auto *Parent = Context->getParent();

        while (Parent) {
            Map[Parent].insert(Context);

            Context = Parent;
            Parent = Parent->getParent();
        }
    }

    return Map;
}


#endif /* OUTPUTGENERATOR_HPP_ */
