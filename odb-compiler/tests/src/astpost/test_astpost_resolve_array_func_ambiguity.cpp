#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/astpost/ResolveArrayFuncAmbiguity.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"

#define NAME astpost_resolve_array_func_ambiguity

using namespace testing;
using namespace odb;

class NAME : public ParserTestHarness
{
public:
};

TEST_F(NAME, resolve_func_call)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parseString("test",
        "result = foo(5)\n"
        "function foo(x)\n"
        "endfunction\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;

    astpost::ResolveArrayFuncAmbiguity post;
    ASSERT_THAT(post.execute(ast), IsTrue());
}

TEST_F(NAME, resolve_array_ref)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parseString("test",
        "dim foo(6)\n"
        "result = foo(5)\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;

    astpost::ResolveArrayFuncAmbiguity post;
    ASSERT_THAT(post.execute(ast), IsTrue());
}

TEST_F(NAME, array_and_function_with_same_name)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parseString("test",
        "dim foo(6)\n"
        "result = foo(5)\n"
        "function foo(x)\n"
        "endfunction\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;

    astpost::ResolveArrayFuncAmbiguity post;
    ASSERT_THAT(post.execute(ast), IsFalse());
}
