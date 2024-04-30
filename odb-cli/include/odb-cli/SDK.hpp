#pragma once

#include "odb-compiler/commands/SDKType.hpp"
#include <string>
#include <vector>
#include <filesystem>

bool setSDKRootDir(const std::vector<std::string>& args);
bool setSDKType(const std::vector<std::string>& args);
bool setAdditionalPluginsDir(const std::vector<std::string>& args);
bool printSDKRootDir(const std::vector<std::string>& args);
bool initSDK(const std::vector<std::string>& args);

odb::SDKType getSDKType();
const std::filesystem::path getSDKRootDir();
const std::vector<std::filesystem::path>& getAdditionalPluginDirs();
