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

#ifndef ACTION_FACTORY_HPP_
#define ACTION_FACTORY_HPP_

#include <memory>

#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Tooling.h>

#include "Config.hpp"

class ActionFactory : public clang::tooling::FrontendActionFactory {
public:
    ActionFactory() = default;

    inline void setConfig(std::shared_ptr<const Config> Config);

    virtual std::unique_ptr<clang::FrontendAction> create() override;

private:
    std::shared_ptr<const Config> Config_;
};

inline void ActionFactory::setConfig(std::shared_ptr<const Config> Config)
{
    Config_ = std::move(Config);
}

#endif /* ACTION_FACTORY_HPP_ */
