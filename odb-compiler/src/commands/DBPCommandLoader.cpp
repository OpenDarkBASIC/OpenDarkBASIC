#include "odb-compiler/commands/DBPCommandLoader.hpp"
#include "odb-compiler/commands/CommandIndex.hpp"
#include "odb-sdk/DynamicLibrary.hpp"
#include "odb-sdk/Log.hpp"
#include "odb-sdk/FileSystem.hpp"
#include "odb-sdk/Str.hpp"
#include <unordered_set>
#include <set>

namespace fs = std::filesystem;

namespace odb {
namespace cmd {

// ----------------------------------------------------------------------------
DBPCommandLoader::DBPCommandLoader(const std::string& sdkRoot,
                                   const std::vector<std::string>& pluginDirs) :
    CommandLoader(sdkRoot, pluginDirs)
{
}

// ----------------------------------------------------------------------------
bool DBPCommandLoader::populateIndex(CommandIndex* index)
{
#if defined(ODBCOMPILER_PLATFORM_WIN32)
    std::unordered_set<std::string> pluginsToLoad;

    if (!fs::is_directory(sdkRoot_))
    {
        log::sdk(log::ERROR, "SDK root directory `%s` does not exist", sdkRoot_.c_str());
        return false;
    }

    fs::path sdkPluginsDir = sdkRoot_ / "plugins";
    if (!fs::is_directory(sdkPluginsDir))
    {
        log::sdk(log::WARNING, "`%s` does not exist. SDK Plugins will not be loaded", sdkPluginsDir.c_str());
    }
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
#else
    // Not implemented on other platforms... yet.
    return false;
#endif
}

// ----------------------------------------------------------------------------
bool DBPCommandLoader::populateIndexFromLibrary(CommandIndex* index, DynamicLibrary* library)
{
#if defined(ODBCOMPILER_PLATFORM_WIN32)
    std::set<std::string> stringTable;

    // Load string table from library. We use a set here to deal with any duplicate entries in the library.
    int stringTableSize = library->getStringTableSize();
    for (int i = 0; i < stringTableSize; ++i)
    {
        stringTable.emplace(library->getStringTableEntryAt(i));
    }

    // Parse string table.
    for (const auto& stringTableEntry : stringTable)
    {
        std::vector<std::string> tokens;
        str::split(stringTableEntry, tokens, '%');

        if (tokens.size() < 2)
        {
            fprintf(stderr, "Invalid string table entry: %s\n", stringTableEntry.c_str());
            continue;
        }

        auto convertTypeChar = [](char type) -> Command::Type
        {
            return static_cast<Command::Type>(type);
        };

        // Extract command name and return type.
        auto& commandName = tokens[0];
        auto& functionTypes = tokens[1];
        const auto& dllSymbol = tokens[2];
        Command::Type returnType = Command::Type::Void;
        std::vector<Command::Arg> args;

        // Extract return type.
        if (commandName.back() == '[')
        {
            commandName = commandName.substr(0, commandName.size() - 1);
            returnType = convertTypeChar(tokens[1][0]);
            functionTypes = functionTypes.substr(1);
        }
        std::transform(commandName.begin(), commandName.end(), commandName.begin(), [](char c) { return std::tolower(c); });

        // Extract arguments.
        std::vector<std::string> argumentNames;
        if (tokens.size() > 3)
        {
            str::split(tokens[3], argumentNames, ',');
        }
        for (int typeIdx = 0; typeIdx < functionTypes.size(); ++typeIdx)
        {
            Command::Arg arg;
            arg.type = convertTypeChar(functionTypes[typeIdx]);
            if (arg.type == Command::Type::Void)
            {
                continue;
            }
            if (typeIdx < argumentNames.size())
            {
                arg.description = std::move(argumentNames[typeIdx]);
            }
            args.emplace_back(std::move(arg));
        }

        index->addCommand(new Command(library, commandName, dllSymbol, returnType, args));
    }

    return true;
#else
    // Not implemented on other platforms... yet.
    return false;
#endif
}

}
}
