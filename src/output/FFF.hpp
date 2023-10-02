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

#ifndef FFF_HPP_
#define FFF_HPP_

#include "OutputGenerator.hpp"

class FFF : public OutputGenerator {
public:
    FFF(std::shared_ptr<const Config> Config, clang::PrintingPolicy Policy);

    void run() override;

private:
    void writeSettings();
    void writeIncludeDirectives();
    void writeMacros();
    void writeTypedefs();
    void writeMocks();

    const Config::FFFSection &getConfig() const;

    llvm::DenseMap<const clang::Type *, const clang::VarDecl *> TypedefMap_;
};

#endif /* FFF_HPP_ */
