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

#ifndef MOCK_ACTION_HPP_
#define MOCK_ACTION_HPP_

#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Tooling.h>
#include <memory>

#include "Config.hpp"

class MockActionFactory : public clang::tooling::FrontendActionFactory {
public:
    MockActionFactory() = default;

    inline void setConfig(std::shared_ptr<const Config> Config);

    std::unique_ptr<clang::FrontendAction> create() override;

private:
    std::shared_ptr<const Config> Config_;
};

inline void MockActionFactory::setConfig(std::shared_ptr<const Config> Config)
{
    Config_ = std::move(Config);
}

#endif /* MOCK_ACTION_HPP_ */
