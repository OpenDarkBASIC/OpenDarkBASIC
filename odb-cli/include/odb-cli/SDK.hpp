#pragma once

#include <string>
#include <vector>

extern "C" {
#include "odb-compiler/sdk/sdk.h"
#include "odb-sdk/ospath.h"
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
