#include <gmock/gmock.h>

extern "C" {
#include "odb-sdk/log.h"
#include "odb-sdk/utf8.h"
}

#define NAME odbsdk_log

using namespace testing;

TEST(NAME, test)
{
    log_sdk_note("Hello\n");
}

TEST(NAME, file_line_column)
{
    const char* source
        = "for a = 1 to 10\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";
    struct utf8_span loc = {23, 5}; /* a = 5 */
    log_flc(
        "{e:error: }",
        "some/file.dba",
        source,
        loc,
        "Assignment is bad %s\n",
        "for some reason");
    // EXPECT_THAT(output, StrEq("some/file.dba:2:8: error: Assignment is bad
    // for some reason\n"))
}

TEST(NAME, excerpt1)
{
    const char* source
        = "for a = 1 to 10\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";
    struct utf8_span loc = {34, 15}; /* a = 7 ... print */
    log_excerpt("some/file.dba", source, loc, "");
}
