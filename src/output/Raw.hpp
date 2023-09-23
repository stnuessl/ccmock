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

#ifndef RAW_HPP_
#define RAW_HPP_

#include "OutputGenerator.hpp"

class Raw : public OutputGenerator {
public:
    Raw(std::shared_ptr<const Config> Config, clang::PrintingPolicy Policy);

    void run() override;
private:
    void visit(const clang::DeclContext *Context, unsigned int Indent = 0);
    
    /* clang-format off */
    llvm::DenseMap<
        const clang::DeclContext *, 
        llvm::DenseSet<const clang::DeclContext *>
    > ContextMap_;
    /* clang-format on */

    clang::AccessSpecifier CurrentAccess_;
};

#endif /* RAW_HPP_ */
