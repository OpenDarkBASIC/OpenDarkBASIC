#include "odb-compiler/commands/DBPCommandLoader.hpp"
#include "odb-compiler/commands/CommandIndex.hpp"
#include "odb-compiler/parsers/db/KeywordToken.hpp"
#include "odb-sdk/DynamicLibrary.hpp"
#include "odb-sdk/FileSystem.hpp"
#include "odb-sdk/Log.hpp"
#include "odb-sdk/Str.hpp"
#include <set>
#include <unordered_set>

namespace fs = std::filesystem;

namespace odb::cmd {

// ----------------------------------------------------------------------------
DBPCommandLoader::DBPCommandLoader(const fs::path& sdkRoot, const std::vector<fs::path>& pluginDirs)
    : CommandLoader(sdkRoot, pluginDirs)
{
}

// ----------------------------------------------------------------------------
bool DBPCommandLoader::populateIndex(CommandIndex* index)
{
#if defined(ODBCOMPILER_PLATFORM_WIN32)
    std::vector<fs::path> pluginsToLoad;

    if (!fs::is_directory(sdkRoot_))
    {
        log::sdk(log::ERROR, "SDK root directory `%s` does not exist\n", sdkRoot_.c_str());
        return false;
    }

    for (const auto& path : {sdkRoot_ / "plugins", sdkRoot_ / "plugins-licensed", sdkRoot_ / "plugins-user"})
    {
        if (fs::is_directory(path))
        {
            for (const auto& p : fs::recursive_directory_iterator(path))
                if (fileIsDynamicLib(p.path()))
                    pluginsToLoad.emplace_back(p.path());
        }
    }

    for (const auto& path : pluginDirs_)
    {
        if (!fs::is_directory(path))
        {
            Log::sdk(Log::WARNING, "`%s` does not exist. Skipping.\n", path.c_str());
            continue;
        }

        for (const auto& p : fs::recursive_directory_iterator(path))
            if (fileIsDynamicLib(p.path()))
                pluginsToLoad.emplace_back(p.path());
    }

    for (const auto& path : pluginsToLoad)
    {
        Reference<DynamicLibrary> lib = DynamicLibrary::open(path.string().c_str());
        if (lib == nullptr)
            continue;

        if (!populateIndexFromLibrary(index, lib))
            return false;
    }

    return true;
#else
    // Not implemented on other platforms... yet.
    Log::sdk(Log::ERROR, "DBP command loading not implemented on this platform\n");
    return false;
#endif
}

// ----------------------------------------------------------------------------
bool DBPCommandLoader::populateIndexFromLibrary(CommandIndex* index, DynamicLibrary* library)
{
#if defined(ODBCOMPILER_PLATFORM_WIN32)
    std::set<std::string> stringTable;

    // Load string table from library. We use a set here to deal with any duplicate entries in the library.
    for (auto& string : library->getStringTable())
    {
        stringTable.emplace(std::move(string));
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
        std::transform(commandName.begin(), commandName.end(), commandName.begin(),
                       [](char c) { return std::tolower(c); });

        // Skip built in commands.
        if (db::KeywordToken::lookup(commandName))
        {
            continue;
        }

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
    Log::sdk(Log::ERROR, "DBP command loading not implemented on this platform\n");
    return false;
#endif
}

} // namespace odb::cmd
