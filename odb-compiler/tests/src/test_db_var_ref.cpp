#include "gmock/gmock.h"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/keywords/Keyword.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"

#define NAME db_var_ref

using namespace testing;
using namespace odb;

class NAME : public ParserTestHarness
{
public:
};

TEST_F(NAME, variable_with_assignment_has_default_type_integer)
{
    /*using Annotation = ast::Symbol::Annotation;

    ast = driver->parseString("test", "");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;

    ast->accept(&v);*/
}
