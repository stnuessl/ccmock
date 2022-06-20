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

#ifndef MOCK_ACTION_HPP_
#define MOCK_ACTION_HPP_

#include <clang/Frontend/FrontendAction.h>
#include <memory>

#include "Config.hpp"

class MockAction : public clang::ASTFrontendAction {
public:
    MockAction() = default;

    inline void setConfig(std::shared_ptr<const Config> Config);

protected:
    virtual std::unique_ptr<clang::ASTConsumer>
    CreateASTConsumer(clang::CompilerInstance &CI,
                      llvm::StringRef File) override;

    virtual bool PrepareToExecuteAction(clang::CompilerInstance &CI) override;
    virtual void EndSourceFileAction() override;

    std::shared_ptr<const Config> Config_;
};

inline void MockAction::setConfig(std::shared_ptr<const Config> Config)
{
    Config_ = std::move(Config);
}

#endif /* MOCK_ACTION_HPP_ */
