extern "C" {
#include "odb-sdk/init.h"
}

#include "gmock/gmock.h"

using namespace testing;

class InitODBSDKEnvironment : public testing::Environment
{
public:
    virtual ~InitODBSDKEnvironment() {}

    virtual void SetUp()
    {
        testing::FLAGS_gtest_death_test_style = "threadsafe";
        ASSERT_THAT(odbsdk_threadlocal_init(), Eq(0));
        ASSERT_THAT(odbsdk_init(), Eq(0));
    }

    virtual void TearDown()
    {
        odbsdk_deinit();
        odbsdk_threadlocal_deinit();
    }
};

const testing::Environment* const initODBSDKEnvironment =
        testing::AddGlobalTestEnvironment(new InitODBSDKEnvironment);
