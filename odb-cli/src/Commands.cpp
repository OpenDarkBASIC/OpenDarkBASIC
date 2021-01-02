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

    log::cmd(log::INFO, "Loaded %d commands\n", cmdIndex_.commands().size());

    return true;
}

// ----------------------------------------------------------------------------
bool dumpCommandsJSON(const std::vector<std::string>& args)
{
    std::vector<Reference<cmd::Command>> commands = cmdIndex_.commands();
    std::sort(commands.begin(), commands.end(), [](const cmd::Command* a, const cmd::Command* b) { return a->dbSymbol() < b->dbSymbol(); });

#if 0
    log::data("{\n");
    for (const auto& command : commands)
    {
        log::data("  \"%s\": {\n", command->dbSymbol().c_str());
        log::data("    \"help\": \"%s\",\n", command->helpFile().c_str());
        log::data("    \"overloads\": [\n");
        for (auto overload = command->overloads.begin(); overload != command->overloads.end(); ++overload)
        {
            log::data("      {\n");
            log::data("        \"returnType\": \"%s\",\n", command->returnType.has_value() ? std::string{(char)command->returnType.value()}.c_str() : "void");
            log::data("        \"args\": [");
            auto arg = overload->arglist.begin();
            if (arg != overload->arglist.end())
                log::data("\"%s\"", (*arg++).name.c_str());
            while (arg != overload->arglist.end())
                log::data(", \"%s\"", (*arg++).name.c_str());
            log::data("]\n");
            log::data("      }%s\n", overload + 1 != command->overloads.end() ? ", " : "");
        }
        log::data("    ]\n");
        log::data("  }%s\n", command + 1 != commands.end() ? "," : "");
    }
    log::data("}\n");

    log::cmdParser(log::INFO, "Commands dumped\n");
#endif
    return true;
}

// ----------------------------------------------------------------------------
bool dumpCommandsINI(const std::vector<std::string> &args)
{
#if 0
    auto commands = cmdIndex_.commandsAsList();
    std::sort(commands.begin(), commands.end(), [](const Command &a, const Command &b) { return a.name < b.name; });

    log::data("[LINKS]\n");
    for (const auto& command : commands)
    {
        log::data("%s=%s=", command.name.c_str(), command.helpFile.c_str());
        for (const auto &overload : command.overloads)
        {
            if (command.overloads.size() > 1)
                log::data("[");
            if (command.returnType.has_value())
                log::data("(");

            auto arg = overload.arglist.begin();
            if (arg != overload.arglist.end())
                log::data("%s", (*arg++).name.c_str());
            else
                log::data("*no parameters*");
            while (arg != overload.arglist.end())
                log::data(", %s", (*arg++).name.c_str());

            if (command.returnType)
                log::data(")");
            if (command.overloads.size() > 1)
                log::data("]");
        }
        log::data("\n");
    }

    log::cmdParser(log::INFO, "Commands dumped\n");
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
        log::data("%s\n", command.c_str());
    }

    log::cmd(log::INFO, "Commands dumped\n");
    return true;
}

// ----------------------------------------------------------------------------
const cmd::CommandIndex* getCommandIndex()
{
    return &cmdIndex_;
}
