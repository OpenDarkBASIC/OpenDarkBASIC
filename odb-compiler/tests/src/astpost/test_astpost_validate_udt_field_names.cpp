#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/astpost/ValidateUDTFieldNames.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"

#define NAME astpost_validate_udt_field_names

using namespace testing;
using namespace odb;

class NAME : public ParserTestHarness
{
public:
};

TEST_F(NAME, invalid_1)
{
    ast = driver->parseString("test",
        "result = foo#.bar.baz");
    ASSERT_THAT(ast, NotNull());

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsFalse());
}

TEST_F(NAME, invalid_2)
{
    ast = driver->parseString("test",
        "result = foo.bar#.baz");
    ASSERT_THAT(ast, NotNull());

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsFalse());
}

TEST_F(NAME, valid_1)
{
    ast = driver->parseString("test",
        "result = foo.bar.baz#");
    ASSERT_THAT(ast, NotNull());

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsTrue());
}
