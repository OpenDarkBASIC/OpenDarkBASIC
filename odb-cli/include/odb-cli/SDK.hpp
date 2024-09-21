#pragma once

#include <string>
#include <vector>

extern "C" {
#include "odb-compiler/sdk/sdk_type.h"
#include "odb-util/ospath.h"
}

void initSDK(void);
void deinitSDK(void);

bool setSDKRootDir(const std::vector<std::string>& args);
bool setSDKType(const std::vector<std::string>& args);
bool setAdditionalPluginsDir(const std::vector<std::string>& args);
bool printSDKRootDir(const std::vector<std::string>& args);
bool setupSDK(const std::vector<std::string>& args);

enum sdk_type getSDKType();
struct ospathc getSDKRootDir();
struct ospath_list* getSDKPluginDirs();
