#include <gmock/gmock.h>
#include "odbc/parsers/db/Driver.hpp"
#include "odbc/ast/Node.hpp"
#include "odbc/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME db_symbol_to_keyword_hack

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odbc;

TEST_F(NAME, keyword_overstepping_buffer_boundary)
{
    // Flex loads the file in 8192 byte chunks. Fill up a file close to that
    // amount so we can put a keyword that oversteps the buffer boundaries
    FILE* fp = fopen("db_symbol_to_keyword_hack.dba", "w");
    for (int i = 0; i != 817; ++i)
        fprintf(fp, "abcdefg=1:");
    fprintf(fp, "abcdefg=1\n");
    // There are now 8180 bytes in the file. This will overstep 8192
    fprintf(fp, "make object sphere 1, 10\n");
    fclose(fp);

    // Make sure parser knows about the keyword
    db.addKeyword({"make object sphere", "", {}});
    matcher.updateFromDB(&db);

    ASSERT_THAT(driver->parseFile("db_symbol_to_keyword_hack.dba"), IsTrue());
}
