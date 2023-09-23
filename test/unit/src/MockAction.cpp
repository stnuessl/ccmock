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

#include "output/OutputGenerator.hpp"
#include "output/CMocka.hpp"
#include "output/FFF.hpp"
#include "output/GMock.hpp"
#include "output/Raw.hpp"

OutputGenerator::OutputGenerator(std::shared_ptr<const Config> Config,
                                 clang::PrintingPolicy Policy,
                                 llvm::StringRef GeneratorName)
    : clang::ASTConsumer(),
      ASTContext_(nullptr),
      Config_(std::move(Config)),
      Writer_(Policy),
      FunctionDecls_(),
      VarDecls_(),
      Name_(GeneratorName),
      AnyVariadic_(false)
{
}

void OutputGenerator::HandleTranslationUnit(clang::ASTContext &Context)
{
    (void) Context;
}

GMock::GMock(std::shared_ptr<const Config> Config, clang::PrintingPolicy Policy)
    : OutputGenerator(std::move(Config), Policy, "GMock"),
      ContextMap_()
{
}

void GMock::run() {}

CMocka::CMocka(std::shared_ptr<const Config> Config, clang::PrintingPolicy Policy)
    : OutputGenerator(std::move(Config), Policy, "CMocka")
{
}

void CMocka::run() {}

FFF::FFF(std::shared_ptr<const Config> Config, clang::PrintingPolicy Policy)
    : OutputGenerator(std::move(Config), Policy, "FFF")
{
}

void FFF::run() {}

Raw::Raw(std::shared_ptr<const Config> Config, clang::PrintingPolicy Policy)
    : OutputGenerator(std::move(Config), Policy, "Raw"),
      CurrentAccess_(clang::AccessSpecifier::AS_none)
{
}

void Raw::run() {}

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
