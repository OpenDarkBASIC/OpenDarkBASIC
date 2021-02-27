#include "odb-compiler/commands/ODBCommandLoader.hpp"
#include "odb-compiler/commands/CommandIndex.hpp"
#include "odb-compiler/commands/Command.hpp"
#include "odb-compiler/parsers/TargetLibParser.hpp"
#include "odb-sdk/Log.hpp"
#include "odb-sdk/FileSystem.hpp"
#include "odb-sdk/Reference.hpp"
#include <filesystem>
#include <unordered_set>

namespace fs = std::filesystem;

namespace odb {
namespace cmd {

// ----------------------------------------------------------------------------
static bool typeExists(Command::Type t)
{
    using T = Command::Type;
    switch (t)
    {
        case T::Integer:
        case T::Float:
        case T::String:
        case T::Double:
        case T::Long:
        case T::Dword:
        case T::Void:
            return true;
        default:
            return false;
    }
}

// ----------------------------------------------------------------------------
static bool parseTypeinfoString(Command::Type* retType, std::vector<Command::Arg>* args, const char* typeinfo)
{
    const char* t = typeinfo;
    *retType = static_cast<Command::Type>(*t);
    if (!typeExists(*retType))
    {
        Log::sdk(Log::ERROR, "Unknown type '%c' found in typeinfo string `%s`", *t, typeinfo);
        return false;
    }

    t++;
    if (*t != '(')
    {
        Log::sdk(Log::ERROR, "Unexpected `%c` while parsing typeinfo string `%s`. Expected `(`", *t, typeinfo);
        return false;
    }

    args->clear();
    for (t++; *t && *t != ')'; t++)
    {
        args->emplace({});
        args->back().type = static_cast<Command::Type>(*t);
        if (!typeExists(args->back().type))
        {
            Log::sdk(Log::ERROR, "Unknown type '%c' found in typeinfo string `%s`", *t, typeinfo);
            return false;
        }
    }

    if (*t != ')')
    {
        if (*t)
            Log::sdk(Log::ERROR, "Unexpected `%c` while parsing typeinfo string `%s`. Expected `)`", *t, typeinfo);
        else
            Log::sdk(Log::ERROR, "Missing `)` while parsing typeinfo string `%s`.", typeinfo);
        return false;
    }

    return true;
}

// ----------------------------------------------------------------------------
ODBCommandLoader::ODBCommandLoader(const fs::path& sdkRoot,
                                   const std::vector<fs::path>& pluginDirs) :
    CommandLoader(sdkRoot, pluginDirs)
{
}

// ----------------------------------------------------------------------------
bool ODBCommandLoader::populateIndex(CommandIndex* index)
{
    std::vector<fs::path> pluginsToLoad;

    if (!fs::is_directory(sdkRoot_))
    {
        Log::sdk(Log::ERROR, "SDK root directory `%s` does not exist\n", sdkRoot_.c_str());
        return false;
    }

    fs::path sdkPluginsDir = sdkRoot_ / "plugins";
    if (!fs::is_directory(sdkPluginsDir))
        Log::sdk(Log::WARNING, "`%s` does not exist. SDK Plugins will not be loaded\n", sdkPluginsDir.c_str());
    else
    {
        for (const auto& p : fs::recursive_directory_iterator(sdkPluginsDir))
            if (FileSystem::isDynamicLib(p.path()))
                pluginsToLoad.emplace_back(p.path());
    }

    for (const auto& path : pluginDirs_)
    {
        if (!fs::is_directory(path))
        {
            Log::sdk(Log::WARNING, "`%s` does not exist. Skipping.\n", path.c_str());
            continue;
        }

        for (const auto& p : fs::recursive_directory_iterator(sdkPluginsDir))
            if (FileSystem::isDynamicLib(p.path()))
                pluginsToLoad.emplace_back(p.path());
    }

    for (const auto& path : pluginsToLoad)
    {
        Reference<TargetLibParser> lib = TargetLibParser::open(path.string());
        if (lib == nullptr)
            continue;

        if (!populateIndexFromLibrary(index, lib))
            return false;
    }

    return true;
}

// ----------------------------------------------------------------------------
bool ODBCommandLoader::populateIndexFromLibrary(CommandIndex* index, TargetLibParser* library)
{
    auto lookupString = [&library](std::string sym) -> std::string {
        //        const char** addr = reinterpret_cast<const char**>(
        //            library->lookupSymbolAddress(sym.c_str()));
        //        return addr ? *addr : "";
        return "";
    };

    for (int i = 0; i != library->getSymbolCount(); ++i)
    {
        std::string cppSymbol = library->getSymbolAt(i);

        std::string dbSymbol = lookupString(cppSymbol + "_name");
        if (dbSymbol == "")
            continue;
        std::string typeinfo = lookupString(cppSymbol + "_typeinfo");
        if (typeinfo == "")
            continue;
        std::string helpfile = lookupString(cppSymbol + "_helpfile");  // optional symbol

        Command::Type retType;
        std::vector<Command::Arg> args;
        if (!parseTypeinfoString(&retType, &args, typeinfo.c_str()))
        {
            Log::sdk(Log::NOTICE, "Error occurred while loading command `%s` from `%s`", dbSymbol.c_str(), library->getFilename());
            return false;
        }

        index->addCommand(new Command(library, dbSymbol, cppSymbol, retType, args, helpfile));
    }

    return true;
}

}
}
