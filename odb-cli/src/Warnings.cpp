#include "odb-cli/Warnings.hpp"
#include "odb-sdk/Log.hpp"

bool warnTest(const std::vector<std::string>& args)
{
    odb::Log::info.print("warnTest: %s\n", args[0].c_str());
    return true;
}

MetaHandlerResult warn(const std::vector<std::string>& args)
{
    int no = (args[0].rfind("no-", 0) == 0);
    MetaHandlerResult result(std::string("W") + (no ? args[0].substr(3) : args[0]), {no ? "0" : "1"});
    if (result.actionId == -1)
        odb::Log::info.print("Error: Unknown warning `%s'\n", args[0].c_str());

    return result;
}
