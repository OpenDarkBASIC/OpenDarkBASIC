#include "gmock/gmock.h"
#include "odb-sdk/Log.hpp"

int main(int argc, char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
