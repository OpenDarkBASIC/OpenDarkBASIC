#include <gmock/gmock.h>

extern "C" {
#include "odb-sdk/utf8.h"
}

#define NAME odbsdk_utf8

using namespace testing;

namespace {
class NAME : public Test
{
    void
    SetUp() override
    {
    }
    void
    TearDown() override
    {
        utf8_deinit(str);
    }

    struct utf8 str = utf8();
};
}

TEST_F(NAME, test)
{
}

