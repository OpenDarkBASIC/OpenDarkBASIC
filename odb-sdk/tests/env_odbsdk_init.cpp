#include "gmock/gmock.h"
#include "vh/init.h"
#include "vh/db.h"

using namespace testing;

class LibraryInitEnvironment : public testing::Environment
{
public:
    virtual ~LibraryInitEnvironment() {}

    virtual void SetUp()
    {
        testing::FLAGS_gtest_death_test_style = "threadsafe";
        ASSERT_THAT(vh_threadlocal_init(), Eq(0));
        ASSERT_THAT(vh_init(), Eq(0));
    }

    virtual void TearDown()
    {
        vh_deinit();
        vh_threadlocal_deinit();
    }
};

const testing::Environment* const libraryInitEnvironment =
        testing::AddGlobalTestEnvironment(new LibraryInitEnvironment);
