#include "odb-cli/Warnings.hpp"
#include "odb-sdk/Log.hpp"

bool warnTest(const std::vector<std::string>& args)
{
    odb::Log::info.print("warnTest: %s\n", args[0].c_str());
    return true;
}

ActionHandler warn(const std::vector<std::string>& args)
{
    // Check for "no-" prefix
    bool no = (args[0].rfind("no-", 0) == 0);

    // All warnings start with "warn-" but this part is ommitted when -W is
    // called. Have to add that back
    ActionHandler result = ActionHandler::fromFullOption(
        std::string("warn-") + (no ? args[0].substr(3) : args[0]), {no ? "0" : "1"});
    if (result.actionId == -1)
        odb::Log::info.print("Error: Unknown warning `%s'\n", args[0].c_str());

    return result;
}
