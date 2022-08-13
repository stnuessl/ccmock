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

#include "MockAction.hpp"

#include <clang/Frontend/CompilerInstance.h>
#include <filesystem>

#include "CMocka.hpp"
#include "FFF.hpp"
#include "GMock.hpp"
#include "util/commandline.hpp"

namespace {

std::string DetectClangResourceDirectory()
{
    std::filesystem::path PathList[] = {
        "/usr/lib/clang",
        "/lib/clang",
    };

    for (const auto &Item : PathList) {
        if (!std::filesystem::is_directory(Item))
            continue;

        for (const auto &Entry : std::filesystem::directory_iterator(Item)) {
            const auto &Path = Entry.path();

            if (!std::isdigit(Path.native().back()))
                continue;

            if (std::filesystem::is_directory(Path / "include"))
                return Path.string();
        }
    }

    return std::string();
}

} // namespace

std::unique_ptr<clang::ASTConsumer>
MockAction::CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef File)
{
    (void) File;

    auto Policy = clang::PrintingPolicy(CI.getLangOpts());

    switch (Config_->Mocking.Backend) {
    case Config::BACKEND_GMOCK:
        return std::make_unique<GMock>(Config_, Policy);
    case Config::BACKEND_FFF:
        return std::make_unique<FFF>(Config_, Policy);
    case Config::BACKEND_CMOCKA:
        return std::make_unique<CMocka>(Config_, Policy);
    default:
        llvm_unreachable("invalid mocking backend");
        break;
    }

    return nullptr;
}

bool MockAction::PrepareToExecuteAction(clang::CompilerInstance &CI)
{
    /*
     * For some reason the clang libtooling applications never know about
     * the clang specific resource directory. This directory contains the
     * include directory to some important header files.
     * Try to automatically find the resource directory. The specific clang
     * version should not matter.
     */

    auto Path = Config_->Clang.ResourceDirectory.string();

    if (Path.empty()) {
        Path = DetectClangResourceDirectory();
        if (Path.empty()) {
            llvm::errs() << "failed to detect clang resource directory\n";
            return false;
        }
    }

    auto Size = Path.size();

    Path += "/include";

    auto Group = clang::frontend::IncludeDirGroup::System;

    CI.getHeaderSearchOpts().AddPath(Path, Group, false, false);

    /* Restore original resource directory path */
    Path.resize(Size);

    CI.getHeaderSearchOpts().ResourceDir = std::move(Path);

    return true;
}

void MockAction::EndSourceFileAction()
{
}
