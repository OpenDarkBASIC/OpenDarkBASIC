#pragma once

#include "odb-compiler/config.hpp"

namespace odb {

class ODBCOMPILER_PUBLIC_API BuildInfo
{
public:
    static const char* authors();
    static const char* version();
    static const char* url();
    static const char* host();
    static const char* time();
    static const char* commitInfo();
    static const char* commitHash();
    static const char* compiler();
    static const char* cmake();

    static const char* all();
};

}
