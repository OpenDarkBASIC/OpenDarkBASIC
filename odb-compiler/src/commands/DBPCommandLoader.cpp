#include "odb-compiler/commands/DBPCommandLoader.hpp"
#include "odb-compiler/commands/CommandIndex.hpp"
#include "odb-compiler/parsers/db/KeywordToken.hpp"
#include "odb-sdk/FileSystem.hpp"
#include "odb-sdk/Log.hpp"
#include "odb-sdk/Str.hpp"
#include <algorithm>
#include <odb-compiler/parsers/TargetLibParser.hpp>
#include <set>

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
    std::vector<fs::path> pluginsToLoad;

    if (!fs::is_directory(sdkRoot_))
    {
        Log::sdk(Log::ERROR, "SDK root directory `%s` does not exist\n", sdkRoot_.c_str());
        return false;
    }

    for (const auto& path : {sdkRoot_ / "plugins", sdkRoot_ / "plugins-licensed", sdkRoot_ / "plugins-user"})
    {
        if (fs::is_directory(path))
        {
            for (const auto& p : fs::recursive_directory_iterator(path))
                if (FileSystem::isDynamicLib(p.path()))
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
bool DBPCommandLoader::populateIndexFromLibrary(CommandIndex* index, TargetLibParser* library)
{
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
        str::split(&tokens, stringTableEntry, '%');

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
        auto& argumentTypeList = tokens[1];
        const auto& dllSymbol = tokens[2];
        Command::Type returnType = Command::Type::Void;
        std::vector<Command::Arg> args;

        // Extract return type.
        if (commandName.back() == '[')
        {
            commandName = commandName.substr(0, commandName.size() - 1);
            returnType = convertTypeChar(tokens[1][0]);
            argumentTypeList = argumentTypeList.substr(1);
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
            str::split(&argumentNames, tokens[3], ',');
        }

        // If a function has a single void type, represented by '0', it should be treated as having no arguments.
        if (argumentTypeList != "0")
        {
            for (std::size_t typeIdx = 0; typeIdx < argumentTypeList.size(); ++typeIdx)
            {
                Command::Arg arg;
                arg.type = convertTypeChar(argumentTypeList[typeIdx]);
                if (arg.type == Command::Type::Void)
                {
                    fprintf(stderr, "Void type '0' encountered in string table entry: %s\n", stringTableEntry.c_str());
                    return false;
                }
                if (typeIdx < argumentNames.size())
                {
                    arg.description = std::move(argumentNames[typeIdx]);
                }
                args.emplace_back(std::move(arg));
            }
        }

        index->addCommand(new Command(library, commandName, dllSymbol, returnType, args));
    }

    return true;
}

} // namespace odb::cmd
