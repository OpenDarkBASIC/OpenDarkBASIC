#pragma once

#include "odb-util/config.h"
#include <cstdio>
#include <filesystem>

namespace odb {

class ODBUTIL_PUBLIC_API FileSystem
{
public:
    static FILE* dupFilePointer(FILE* file);

    static bool isDynamicLib(const std::filesystem::path& filename);

    static std::filesystem::path getPathToSelf();
};

}
