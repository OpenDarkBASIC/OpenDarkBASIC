#include "odb-compiler/commands/ODBCommandLoader.hpp"
#include "odb-compiler/commands/CommandIndex.hpp"
#include "odb-compiler/commands/Command.hpp"
#include "odb-sdk/DynamicLibrary.hpp"
#include "odb-sdk/Log.hpp"
#include "odb-sdk/FileSystem.hpp"
#include "odb-sdk/Reference.hpp"
#include <filesystem>
#include <unordered_set>

namespace fs = std::filesystem;

namespace odb {
namespace cmd {

// ----------------------------------------------------------------------------
static bool typeExists (Command::Type t)
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
        log::sdk(log::ERROR, "Unknown type '%c' found in typeinfo string `%s`", *t, typeinfo);
        return false;
    }

    t++;
    if (*t != '(')
    {
        log::sdk(log::ERROR, "Unexpected `%c` while parsing typeinfo string `%s`. Expected `(`", *t, typeinfo);
        return false;
    }

    args->clear();
    for (t++; *t && *t != ')'; t++)
    {
        args->emplace({});
        args->back().type = static_cast<Command::Type>(*t);
        if (!typeExists(args->back().type))
        {
            log::sdk(log::ERROR, "Unknown type '%c' found in typeinfo string `%s`", *t, typeinfo);
            return false;
        }
    }

    if (*t != ')')
    {
        if (*t)
            log::sdk(log::ERROR, "Unexpected `%c` while parsing typeinfo string `%s`. Expected `)`", *t, typeinfo);
        else
            log::sdk(log::ERROR, "Missing `)` while parsing typeinfo string `%s`.", typeinfo);
        return false;
    }

    return true;
}

// ----------------------------------------------------------------------------
ODBCommandLoader::ODBCommandLoader(const std::string& sdkRoot,
                                   const std::vector<std::string>& pluginDirs) :
    CommandLoader(sdkRoot, pluginDirs)
{
}

// ----------------------------------------------------------------------------
bool ODBCommandLoader::populateIndex(CommandIndex* index)
{
    std::unordered_set<std::string> pluginsToLoad;

    if (!fs::is_directory(sdkRoot_))
    {
        log::sdk(log::ERROR, "SDK root directory `%s` does not exist", sdkRoot_.c_str());
        return false;
    }

    fs::path sdkPluginsDir = sdkRoot_ / "plugins";
    if (!fs::is_directory(sdkPluginsDir))
        log::sdk(log::WARNING, "`%s` does not exist. SDK Plugins will not be loaded", sdkPluginsDir.c_str());
    else
    {
        for (const auto& p : fs::recursive_directory_iterator(sdkPluginsDir))
            if (fileIsDynamicLib(p.path()))
                pluginsToLoad.emplace(p.path().string());
    }

    for (const auto& path : pluginDirs_)
    {
        if (!fs::is_directory(path))
        {
            log::sdk(log::WARNING, "`%s` does not exist. Skipping.", path.c_str());
            continue;
        }

        for (const auto& p : fs::recursive_directory_iterator(sdkPluginsDir))
            if (fileIsDynamicLib(p.path()))
                pluginsToLoad.emplace(p.path().string());
    }

    for (const auto& path : pluginsToLoad)
    {
        Reference<DynamicLibrary> lib = DynamicLibrary::open(path.c_str());
        if (lib == nullptr)
            continue;

        if (!populateIndexFromLibrary(index, lib))
            return false;
    }

    return true;
}

// ----------------------------------------------------------------------------
bool ODBCommandLoader::populateIndexFromLibrary(CommandIndex* index, DynamicLibrary* library)
{
    auto lookupString = [&library](std::string sym) -> std::string {
        const char** addr = reinterpret_cast<const char**>(
            library->lookupSymbolAddress(sym.c_str()));
        return addr ? *addr : "";
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
            log::sdk(log::NOTICE, "Error occurred while loading command `%s` from `%s`", dbSymbol.c_str(), library->getFilename());
            return false;
        }

        index->addCommand(new Command(library, dbSymbol, cppSymbol, retType, args, helpfile));
    }

    return true;
}

}
}
