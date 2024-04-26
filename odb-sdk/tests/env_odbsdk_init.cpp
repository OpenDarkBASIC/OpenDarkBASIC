extern "C" {
#include "odb-sdk/init.h"
}

#include "gmock/gmock.h"

using namespace testing;

class LibraryInitEnvironment : public testing::Environment
{
public:
    virtual ~LibraryInitEnvironment() {}

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

const testing::Environment* const libraryInitEnvironment =
        testing::AddGlobalTestEnvironment(new LibraryInitEnvironment);
