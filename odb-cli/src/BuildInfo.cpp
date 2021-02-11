#include "odb-cli/BuildInfo.hpp"
#include "odb-compiler/BuildInfo.hpp"
#include "odb-sdk/Log.hpp"

bool printCommitHash(const std::vector<std::string>& args)
{
    odb::Log::info.print("Commit hash: %s\n", odb::BuildInfo::commitHash());
    return true;
}
