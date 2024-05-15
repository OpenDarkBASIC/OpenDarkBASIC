#pragma once

#include <string>
#include <vector>

extern "C" {
#include "odb-compiler/codegen/codegen.h"
}

bool setOutputType(const std::vector<std::string>& args);
bool setArch(const std::vector<std::string>& args);
bool setPlatform(const std::vector<std::string>& args);
bool output(const std::vector<std::string>& args);

enum odb_codegen_platform getTargetPlatform(void);

