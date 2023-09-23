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
    : OutputGenerator(std::move(Config), Policy, "fff")
{
}

void FFF::run()
{
    writeSettings();
    writeIncludeDirectives();
    writeMacros();
    writeTypedefs();
    writeMocks();
    writeGlobalVariables();
}

void FFF::writeSettings()
{
    auto &Writer = getWriter();
    if (!getConfig().GCCFunctionAttributes.empty()) {
        Writer.write("#define FFF_GCC_FUNCTION_ATTRIBUTES ");
        Writer.write(getConfig().GCCFunctionAttributes);
        Writer.write("\n");
    }

    if (getConfig().ArgHistoryLen >= 0) {
        Writer.write("#define FFF_ARG_HISTORY_LEN (");
        Writer.write(getConfig().ArgHistoryLen);
        Writer.write("u)\n");
    }

    if (getConfig().CallHistoryLen >= 0) {
        Writer.write("#define FFF_CALL_HISTORY_LEN (");
        Writer.write(getConfig().CallHistoryLen);
        Writer.write("u)\n");
    }
}

void FFF::writeIncludeDirectives()
{
    llvm::StringRef Data = R"(
#include <fff.h>

)";

    getWriter().write(Data);
}

void FFF::writeMacros()
{
    llvm::StringRef Data = "DEFINE_FFF_GLOBALS\n"
                           "\n"
                           "#define FFF_FAKE_LIST(FAKE) \\\n";

    getWriter().write(Data);

    for (const auto *Decl : getFunctionDecls()) {
        getWriter().write("    FAKE(");
        getWriter().write(Decl->getName());
        getWriter().write(") \\\n");
    }

    getWriter().write("\n\n");
}

void FFF::writeTypedefs()
{
    auto &Writer = getWriter();
    std::string Name;

    for (const auto *Decl : getFunctionDecls()) {
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
            auto *VarDecl = util::decl::fakeVarDecl(Context, Type, Name);

            Writer.write("typedef ");
            Writer.writeVarDecl(VarDecl);
            Writer.write(";\n");

            auto [_, Ok] = TypedefMap_.insert({Type.getTypePtr(), VarDecl});
            if (!Ok) {
                /* FIXME: use diagnostic engine */
                llvm::errs() << util::cl::error()
                             << "failed to store new typedef information\n";
                std::exit(EXIT_FAILURE);
            }
        }
    }

    if (!TypedefMap_.empty())
        Writer.write("\n");
}

void FFF::writeMocks()
{
    auto &Writer = getWriter();
    Writer.write("#ifdef __cplusplus\n"
                 "extern \"C\" {\n"
                 "#endif\n\n");

    for (const auto *Decl : getFunctionDecls()) {
        llvm::StringRef Suffix;

        if (Decl->isVariadic())
            Suffix = "_VARARG";

        auto Type = Decl->getReturnType();
        if (Type->isVoidType()) {
            Writer.write("FAKE_VOID_FUNC");
            Writer.write(Suffix);
            Writer.write("(");
        } else {
            Writer.write("FAKE_VALUE_FUNC");
            Writer.write(Suffix);
            Writer.write("(");
            Writer.writeType(Type);
            Writer.write(", ");
        }

        if (!getConfig().CallingConvention.empty()) {
            Writer.write(getConfig().CallingConvention);
            Writer.write(", ");
        }

        Writer.write(Decl->getName());

        auto Parameters = Decl->parameters();
        if (!Parameters.empty()) {
            for (const auto *ParmVarDecl : Parameters) {
                Writer.write(", ");

                auto Type = ParmVarDecl->getType();
                auto It = TypedefMap_.find(Type.getTypePtr());
                if (It != TypedefMap_.end()) {
                    Writer.write(It->second->getName());
                    continue;
                }

                /*
                 * FFF's mocking macros create structs containing variables
                 * which get derived from function parameters.
                 * These structs therefore do not work well with const
                 * types, so we have to remove potential consts here.
                 */
                Type.removeLocalConst();
                Writer.writeType(Type);
            }
        }

        if (Decl->isVariadic())
            Writer.write(", ...");

        Writer.write(");\n");
    }

    Writer.write("\n"
                 "#ifdef __cplusplus\n"
                 "} /* extern \"C\" */\n"
                 "#endif\n\n");
}

const Config::FFFSection &FFF::getConfig() const
{
    return OutputGenerator::getConfig().FFF;
}
