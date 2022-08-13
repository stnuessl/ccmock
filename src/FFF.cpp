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

#include "FFF.hpp"

#include "util/Decl.hpp"
#include "util/commandline.hpp"

static void
createTypedefName(std::string &Name, const clang::NamedDecl *Decl, size_t index)
{
    llvm::raw_string_ostream OS(Name);

    OS << *Decl << "_fn" << index + 1 << "_t";
}

FFF::FFF(std::shared_ptr<const Config> Config, clang::PrintingPolicy Policy)
    : Generator(std::move(Config), Policy, "fff")
{
}

void FFF::run()
{
    writeSettings();
    writeIncludeDirectives();
    writeMacros();
    writeGlobalVariables();
    writeTypedefs();
    writeMocks();
}

void FFF::writeSettings()
{
    if (!Config_->FFF.GCCFunctionAttributes.empty()) {
        Out_ << "#define FFF_GCC_FUNCTION_ATTRIBUTES "
             << Config_->FFF.GCCFunctionAttributes << "\n";
    }

    if (Config_->FFF.ArgHistoryLen >= 0) {
        Out_ << "#define FFF_ARG_HISTORY_LEN (" << Config_->FFF.ArgHistoryLen
             << "u)\n";
    }

    if (Config_->FFF.CallHistoryLen >= 0) {
        Out_ << "#define FFF_CALL_HISTORY_LEN (" << Config_->FFF.CallHistoryLen
             << "u)\n";
    }
}

void FFF::writeIncludeDirectives()
{
    Out_ << R"(
#include <fff.h>

)";
}

void FFF::writeMacros()
{
    Out_ << "DEFINE_FFF_GLOBALS;\n"
            "\n"
            "#define FFF_FAKE_LIST(FAKE) \\\n";

    for (size_t i = 0, Size = Functions_.size(); i < Size; ++i) {
        Out_ << "   FAKE(" << *Functions_[i] << ")";
        if (i != Size - 1)
            Out_ << " \\\n";
        else
            Out_ << "\n";
    }

    Out_ << "\n";
}

void FFF::writeTypedefs()
{
    std::string Name;

    for (auto Decl : Functions_) {
        auto Parameters = Decl->parameters();

        for (size_t i = 0, Size = Parameters.size(); i < Size; ++i) {
            auto Type = Parameters[i]->getType();

            auto Pointee = Type->getPointeeType();
            if (Pointee.isNull() || !Pointee->isFunctionType())
                continue;

            if (Type->isTypedefNameType())
                continue;

            if (TypedefMap_.count(Type.getTypePtr()))
                continue;

            /*
             * Create a typedef and store it for later use when creating
             * the mocks with FFF macros.
             */
            Name.clear();
            createTypedefName(Name, Decl, i);

            auto &Context = Decl->getASTContext();
            auto VarDecl = util::decl::fakeVarDecl(Context, Type, Name);
            Out_ << "typedef ";
            VarDecl->print(Out_, PrintingPolicy_);
            Out_ << ";\n";

            auto [_, Ok] = TypedefMap_.insert({Type.getTypePtr(), VarDecl});
            if (!Ok) {
                llvm::errs() << util::cl::error()
                             << "failed to store new typedef information\n";
                std::exit(EXIT_FAILURE);
            }
        }
    }

    Out_ << "\n";
}

void FFF::writeMocks()
{
    Out_ << R"(
#ifdef __cplusplus
extern "C" {
#endif

)";

    for (auto &Decl : Functions_) {
        llvm::StringRef Suffix;

        if (Decl->isVariadic())
            Suffix = "_VARARG";

        auto Type = Decl->getReturnType();
        if (Type->isVoidType()) {
            Out_ << "FAKE_VOID_FUNC" << Suffix << "(";
        } else {
            Out_ << "FAKE_VALUE_FUNC" << Suffix << "(";
            writeType(Type);
            Out_ << ", ";
        }

        if (!Config_->FFF.CallingConvention.empty())
            Out_ << Config_->FFF.CallingConvention << ", ";

        Out_ << *Decl;

        auto Parameters = Decl->parameters();
        if (!Parameters.empty()) {
            for (auto ParmVarDecl : Parameters) {
                Out_ << ", ";

                auto Type = ParmVarDecl->getType();
                auto It = TypedefMap_.find(Type.getTypePtr());
                if (It != TypedefMap_.end()) {
                    Out_ << *It->second;
                    continue;
                }

                /*
                 * FFF's mocking macros create structs containing variables
                 * which get derived from function parameters.
                 * These structs therefore do not work well with const types,
                 * so we have to remove potential consts here.
                 */
                Type.removeLocalConst();
                writeType(Type);
            }
        }

        if (Decl->isVariadic())
            Out_ << ", ...";

        Out_ << ");\n";
    }

    Out_ << R"(

#ifdef __cplusplus
} /* extern "C" */
#endif
)";
}
