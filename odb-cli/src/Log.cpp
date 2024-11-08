#include "odb-cli/Log.hpp"

extern "C" {
#include "odb-util/log.h"
}

static void
write_stderr(const char* fmt, va_list ap)
{
    vfprintf(stderr, fmt, ap);
    fflush(stderr);
}

// ----------------------------------------------------------------------------
bool disableColor(const std::vector<std::string>& args)
{
    struct log_interface iface = {write_stderr, 0};
    log_configure(iface);
    return true;
}

// ----------------------------------------------------------------------------
bool enableColor(const std::vector<std::string>& args)
{
    struct log_interface iface = {write_stderr, 1};
    log_configure(iface);
    return true;
}
