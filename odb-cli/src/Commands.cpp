#include "odb-cli/Commands.hpp"
#include "odb-cli/SDK.hpp"
#include "odb-compiler/commands/CommandIndex.hpp"
#include "odb-compiler/commands/ODBCommandLoader.hpp"
#include "odb-compiler/commands/DBPCommandLoader.hpp"
#include "odb-sdk/Log.hpp"
#include <algorithm>

using namespace odb;

static cmd::CommandIndex cmdIndex_;

// ----------------------------------------------------------------------------
bool loadCommands(const std::vector<std::string>& args)
{
    std::unique_ptr<odb::cmd::CommandLoader> loader;
    switch (getSDKType())
    {
        case odb::SDKType::ODB :
            loader = std::make_unique<cmd::ODBCommandLoader>(getSDKRootDir(), getAdditionalPluginDirs());
            break;
        case odb::SDKType::DarkBASIC :
            loader = std::make_unique<cmd::DBPCommandLoader>(getSDKRootDir(), getAdditionalPluginDirs());
            break;
    }

    if (loader->populateIndex(&cmdIndex_) == false)
        return false;

    if (cmdIndex_.findConflicts())
        return false;

    Log::cmd(Log::INFO, "Loaded %d commands\n", cmdIndex_.commands().size());

    return true;
}

// ----------------------------------------------------------------------------
bool dumpCommandsJSON(const std::vector<std::string>& args)
{
    std::vector<Reference<cmd::Command>> commands = cmdIndex_.commands();
    std::sort(commands.begin(), commands.end(), [](const cmd::Command* a, const cmd::Command* b) { return a->dbSymbol() < b->dbSymbol(); });

#if 0
    Log::data.log("{\n");
    for (const auto& command : commands)
    {
        Log::data.log("  \"%s\": {\n", command->dbSymbol().c_str());
        Log::data.log("    \"help\": \"%s\",\n", command->helpFile().c_str());
        Log::data.log("    \"overloads\": [\n");
        for (auto overload = command->overloads.begin(); overload != command->overloads.end(); ++overload)
        {
            Log::data.log("      {\n");
            Log::data.log("        \"returnType\": \"%s\",\n", command->returnType.has_value() ? std::string{(char)command->returnType.value()}.c_str() : "void");
            Log::data.log("        \"args\": [");
            auto arg = overload->arglist.begin();
            if (arg != overload->arglist.end())
                Log::data.log("\"%s\"", (*arg++).name.c_str());
            while (arg != overload->arglist.end())
                Log::data.log(", \"%s\"", (*arg++).name.c_str());
            Log::data.log("]\n");
            Log::data.log("      }%s\n", overload + 1 != command->overloads.end() ? ", " : "");
        }
        Log::data.log("    ]\n");
        Log::data.log("  }%s\n", command + 1 != commands.end() ? "," : "");
    }
    Log::data.log("}\n");

    Log::cmdParser(Log::INFO, "Commands dumped\n");
#endif
    return true;
}

// ----------------------------------------------------------------------------
bool dumpCommandsINI(const std::vector<std::string> &args)
{
#if 0
    auto commands = cmdIndex_.commandsAsList();
    std::sort(commands.begin(), commands.end(), [](const Command &a, const Command &b) { return a.name < b.name; });

    Log::data.log("[LINKS]\n");
    for (const auto& command : commands)
    {
        Log::data.log("%s=%s=", command.name.c_str(), command.helpFile.c_str());
        for (const auto &overload : command.overloads)
        {
            if (command.overloads.size() > 1)
                Log::data.log("[");
            if (command.returnType.has_value())
                Log::data.log("(");

            auto arg = overload.arglist.begin();
            if (arg != overload.arglist.end())
                Log::data.log("%s", (*arg++).name.c_str());
            else
                Log::data.log("*no parameters*");
            while (arg != overload.arglist.end())
                Log::data.log(", %s", (*arg++).name.c_str());

            if (command.returnType)
                Log::data.log(")");
            if (command.overloads.size() > 1)
                Log::data.log("]");
        }
        Log::data.log("\n");
    }

    Log::cmdParser(Log::INFO, "Commands dumped\n");
#endif
    return true;
}

// ----------------------------------------------------------------------------
bool dumpCommandNames(const std::vector<std::string>& args)
{
    auto commands = cmdIndex_.commandNamesAsList();
    std::sort(commands.begin(), commands.end(), [](const std::string& a,const  std::string& b) { return a < b; });
    for (const auto& command : commands)
    {
        Log::data.log("%s\n", command.c_str());
    }

    Log::cmd(Log::INFO, "Commands dumped\n");
    return true;
}

// ----------------------------------------------------------------------------
const cmd::CommandIndex* getCommandIndex()
{
    return &cmdIndex_;
}
