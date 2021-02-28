#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"

#define NAME db_parser_goto

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odb;
using namespace ast;

TEST_F(NAME, simple_goto)
{
    ast = driver->parse("test",
        "goto label\n",
        matcher);
    ASSERT_THAT(ast, NotNull());
}
