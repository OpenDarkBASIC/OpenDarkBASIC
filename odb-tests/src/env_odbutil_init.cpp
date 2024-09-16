extern "C" {
#include "odb-util/init.h"
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
        ASSERT_THAT(odbutil_init(), Eq(0));
    }

    virtual void TearDown()
    {
        odbutil_deinit();
    }
};

const testing::Environment* const initODBUtilEnvironment =
        testing::AddGlobalTestEnvironment(new InitODBSDKEnvironment);
