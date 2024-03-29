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

#ifndef CMOCKA_HPP_
#define CMOCKA_HPP_

#include "OutputGenerator.hpp"

class CMocka : public OutputGenerator {
public:
    CMocka(std::shared_ptr<const Config> Config, clang::PrintingPolicy Policy);

    void run() override;

private:
    void writeIncludeDirectives();
    void writeMockFunctions();

    void writeFunctionBody(const clang::FunctionDecl *Decl);
    const Config::CMockaSection &getConfig() const;
};

#endif /* CMOCKA_HPP_ */
