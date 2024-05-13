#include "odb-cli/BuildInfo.hpp"

extern "C" {
#include "odb-compiler/build_info.h"
#include "odb-sdk/log.h"
}

// ----------------------------------------------------------------------------
bool printCommitHash(const std::vector<std::string>& args)
{
    log_raw("%s\n", build_info_commit_hash());
    return true;
}

// ----------------------------------------------------------------------------
bool printVersion(const std::vector<std::string>& args)
{
    log_raw("%s\n", build_info_version());
    return true;
}
