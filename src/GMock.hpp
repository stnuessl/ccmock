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

#ifndef GMOCK_HPP_
#define GMOCK_HPP_

#include "Generator.hpp"

class GMock : public Generator {
public:
    GMock(std::shared_ptr<const Config> Config, clang::PrintingPolicy Policy);

    virtual void run() override;

private:
    void writeIncludeDirectives();
    void writeGlobalMocks();
    void writeMockClasses();
    void writeMockInstances();
    void writeMockFunctions();
    void writeMain();

    void writeFunctionParameterList(const clang::FunctionDecl *Decl);
    void writeMockParameterList(const clang::FunctionDecl *Decl);
    void writeMockCall(const clang::FunctionDecl *Decl);
    void writeFunctionBody(const clang::FunctionDecl *Decl);
    void writeFunctionSpecifiers(const clang::FunctionDecl *Decl);
};

#endif /* GMOCK_HPP_ */
