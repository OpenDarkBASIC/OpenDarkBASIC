#include "odb-cli/BuildInfo.hpp"
#include "odb-sdk/Log.hpp"

extern "C" {
#include "odb-compiler/build_info.h"
}

// ----------------------------------------------------------------------------
bool printCommitHash(const std::vector<std::string>& args)
{
    odb::Log::info.print("Commit hash: %s\n", build_info_commit_hash());
    return true;
}

// ----------------------------------------------------------------------------
bool printVersion(const std::vector<std::string>& args)
{
    odb::Log::info.print("Version: %s\n", build_info_version());
    return true;
}
