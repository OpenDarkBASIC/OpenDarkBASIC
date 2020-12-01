#include "gmock/gmock.h"
#include "odb-util/Log.hpp"

int main(int argc, char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    odb::log::init();
    return RUN_ALL_TESTS();
}
