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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <MockAction.hpp>
#include <ActionFactory.hpp>

std::unique_ptr<clang::ASTConsumer>
MockAction::CreateASTConsumer(clang::CompilerInstance &CI,
                      llvm::StringRef File)
{
    (void) CI;
    (void) File;

    return nullptr;
}

bool MockAction::PrepareToExecuteAction(clang::CompilerInstance &CI)
{
    (void) CI;

    return true;
}

void MockAction::EndSourceFileAction()
{

}

TEST(ActionFactory, Create)
{
    auto Factory = ActionFactory();

    auto Action = Factory.create();

    ASSERT_TRUE(Action);
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

