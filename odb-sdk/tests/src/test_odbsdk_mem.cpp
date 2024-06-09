extern "C" {
#include "odb-sdk/mem.h"
}

#include "gmock/gmock.h"

#define NAME memory

using namespace testing;

TEST(NAME, malloc_free)
{
    void* p = mem_alloc(16);
    EXPECT_THAT(p, NotNull());
    mem_free(p);
}

TEST(NAME, realloc_free)
{
    void* p = mem_realloc(NULL, 16);
    EXPECT_THAT(p, NotNull());
    mem_free(p);
}

TEST(NAME, realloc_realloc_free)
{
    void* p = mem_realloc(NULL, 2);
    EXPECT_THAT(p, NotNull());
    p = mem_realloc(p, 4);
    EXPECT_THAT(p, NotNull());
    mem_free(p);
}
