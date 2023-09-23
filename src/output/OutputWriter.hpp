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

#ifndef OUTPUTWRITER_HPP_
#define OUTPUTWRITER_HPP_

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/PrettyPrinter.h>

class OutputWriter {
public:
    inline OutputWriter(clang::PrintingPolicy Policy);

    inline void indent(unsigned int N);
    inline void writeDeclName(const clang::DeclContext *Context);
    inline void writeFullyQualifiedName(const clang::NamedDecl *Decl);
    inline void writeFunctionDecl(const clang::FunctionDecl *Decl, unsigned int Indent = 0);
    void writeFunctionParameterList(const clang::FunctionDecl *Decl,
                                    bool ParameterNames = true,
                                    bool VarArgList = false);
    void writeMockName(const clang::FunctionDecl *Decl);
    inline void writeReturnType(const clang::FunctionDecl *Decl);
    inline void writeType(const clang::VarDecl *Decl);
    inline void writeType(clang::QualType Type);
    inline void writeVarDecl(const clang::VarDecl *Decl);
    
    template <typename T>
    void write(T Data);

    inline void flush(llvm::raw_ostream &OS);

    inline clang::PrintingPolicy &getPrintingPolicy();
    const inline clang::PrintingPolicy &getPrintingPolicy() const;
private:
    clang::PrintingPolicy PrintingPolicy_;
    std::string Buffer_;
    llvm::raw_string_ostream Out_;
};

OutputWriter::OutputWriter(clang::PrintingPolicy Policy) 
    : PrintingPolicy_(Policy), Buffer_(), Out_(Buffer_)
{
    Buffer_.reserve(BUFSIZ);
}
    
inline void OutputWriter::indent(unsigned int N)
{
    Out_.indent(N);
}

inline void OutputWriter::writeDeclName(const clang::DeclContext *Context)
{
    Out_ << clang::cast<clang::NamedDecl>(Context)->getName();
}

inline void OutputWriter::writeFullyQualifiedName(const clang::NamedDecl *Decl)
{
    Decl->printQualifiedName(Out_);
}

inline void OutputWriter::writeFunctionDecl(const clang::FunctionDecl *Decl, unsigned int Indent)
{
    Decl->print(Out_, PrintingPolicy_, Indent);
}

inline void OutputWriter::writeReturnType(const clang::FunctionDecl *Decl)
{
    auto Type = Decl->getReturnType();

    writeType(Type);
}

inline void OutputWriter::writeType(const clang::VarDecl *Decl)
{
    writeType(Decl->getType());
}

inline void OutputWriter::writeType(clang::QualType Type)
{
    Type.getCanonicalType().print(Out_, PrintingPolicy_);
}

inline void OutputWriter::writeVarDecl(const clang::VarDecl *Decl)
{
    Decl->print(Out_);
}

template <typename T>
void OutputWriter::write(T Data)
{
    Out_ << Data;
}

inline void OutputWriter::flush(llvm::raw_ostream &OS)
{
    OS << Buffer_ << "\n";
    Buffer_.clear();
}
    
inline clang::PrintingPolicy &OutputWriter::getPrintingPolicy()
{
    return PrintingPolicy_;
}

const inline clang::PrintingPolicy &OutputWriter::getPrintingPolicy() const
{
    return PrintingPolicy_;
}

#endif /* OUTPUTWRITER_HPP_ */
