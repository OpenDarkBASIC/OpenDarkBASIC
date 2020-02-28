#include "gmock/gmock.h"
#include "odbc/util/Log.hpp"

int main(int argc, char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    odbc::log::init();
    return RUN_ALL_TESTS();
}
