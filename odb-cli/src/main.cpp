#include "odb-cli/Actions.hpp"
#include "odb-sdk/Log.hpp"

class Log
{
public:
    Log() { odb::log::init(); }
    ~Log() { odb::log::deinit(); }
};

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    Log log;

    if (parseCommandLine(argc, argv) == false)
        return -1;

    return 0;
}
