#pragma once

#include <cstdio>
#include "odb-sdk/config.hpp"

namespace odb {

ODBSDK_PUBLIC_API FILE* dupFilePointer(FILE* file);

}
