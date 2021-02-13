#include "odb-cli/Log.hpp"
#include "odb-sdk/Log.hpp"

// ----------------------------------------------------------------------------
bool enableColor(const std::vector<std::string>& args)
{
    odb::Log::info.enableColor();
    return true;
}

// ----------------------------------------------------------------------------
bool disableColor(const std::vector<std::string>& args)
{
    odb::Log::info.disableColor();
    return true;
}
