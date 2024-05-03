#include <gmock/gmock.h>

extern "C" {
#include "odb-sdk/utf8_list.h"
}

#define NAME odbsdk_utf8_list

using namespace testing;

class NAME : public Test
{
public:
    void
    SetUp() override
    {
    }

    void
    TearDown() override
    {
        utf8_list_free(&list);
    }

    struct utf8_list list;
};

