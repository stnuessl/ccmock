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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <MockAction.hpp>

#include "Generator.hpp"
#include "CMocka.hpp"
#include "FFF.hpp"
#include "GMock.hpp"

Generator::Generator(std::shared_ptr<const Config> Config,
                     clang::PrintingPolicy Policy,
                     llvm::StringRef Backend)
    : clang::ASTConsumer(),
      Config_(std::move(Config)),
      PrintingPolicy_(Policy),
      Buffer_(),
      Out_(Buffer_),
      FuncDeclVec_(),
      VarDeclVec_(),
      AnyVariadic_(false),
      Backend_(Backend)
{
}

void Generator::HandleTranslationUnit(clang::ASTContext &Context)
{
    (void) Context;
}

GMock::GMock(std::shared_ptr<const Config> Config, clang::PrintingPolicy Policy)
    : Generator(std::move(Config), Policy, "GMock")
{
}

void GMock::run() {}

CMocka::CMocka(std::shared_ptr<const Config> Config, clang::PrintingPolicy Policy)
    : Generator(std::move(Config), Policy, "CMocka")
{
}

void CMocka::run() {}

FFF::FFF(std::shared_ptr<const Config> Config, clang::PrintingPolicy Policy)
    : Generator(std::move(Config), Policy, "FFF")
{
}

void FFF::run() {}

TEST(ActionFactory, Create)
{
    auto Factory = MockActionFactory();

    auto Action = Factory.create();

    ASSERT_TRUE(Action);
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
