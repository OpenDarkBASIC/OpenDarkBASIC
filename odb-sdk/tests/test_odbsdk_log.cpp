#include <gmock/gmock.h>

extern "C" {
#include "odb-sdk/log.h"
}

#define NAME log

using namespace testing;

TEST(NAME, test)
{
    log_sdk_note("Hello\n");
}

