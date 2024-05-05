#pragma once

#include "odb-compiler/commands/SDKType.hpp"
#include <string>
#include <vector>
#include <filesystem>

extern "C" {
#include "odb-sdk/ospath.h"
}

bool setSDKRootDir(const std::vector<std::string>& args);
bool setSDKType(const std::vector<std::string>& args);
bool setAdditionalPluginsDir(const std::vector<std::string>& args);
bool printSDKRootDir(const std::vector<std::string>& args);
bool initSDK(const std::vector<std::string>& args);

odb::SDKType getSDKType();
struct ospath_view getSDKRootDir();
const std::vector<std::filesystem::path>& getAdditionalPluginDirs();
