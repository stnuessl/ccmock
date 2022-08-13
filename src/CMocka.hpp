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

#ifndef CMOCKA_HPP_
#define CMOCKA_HPP_

#include "Generator.hpp"

class CMocka : public Generator {
public:
    CMocka(std::shared_ptr<const Config> Config, clang::PrintingPolicy Policy);

    virtual void run() override;

private:
    void writeIncludeDirectives();
    void writeMockFunctions();

    void writeFunctionBody(const clang::FunctionDecl *Decl);
};

#endif /* CMOCKA_HPP_ */
