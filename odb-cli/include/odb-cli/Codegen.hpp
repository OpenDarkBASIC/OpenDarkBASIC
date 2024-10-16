#pragma once

#include <string>
#include <vector>

extern "C" {
#include "odb-compiler/codegen/target.h"
}

bool setArch(const std::vector<std::string>& args);
bool setPlatform(const std::vector<std::string>& args);
bool output(const std::vector<std::string>& args);
bool dumpIR(const std::vector<std::string>& args);
bool exec_output(const std::vector<std::string>& args);
bool set_optimization_level(const std::vector<std::string>& args);

enum target_arch getTargetArch(void);
enum target_platform getTargetPlatform(void);
