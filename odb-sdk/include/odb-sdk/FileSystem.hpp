#pragma once

#include "odb-sdk/config.hpp"
#include <cstdio>
#include <filesystem>

namespace odb {

ODBSDK_PUBLIC_API FILE* dupFilePointer(FILE* file);

ODBSDK_PUBLIC_API bool fileIsDynamicLib(const std::filesystem::path& filename);

}
