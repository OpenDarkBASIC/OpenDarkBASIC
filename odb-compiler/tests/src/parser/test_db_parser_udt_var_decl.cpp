#include "odb-compiler/ast/Annotation.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/Scope.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/matchers/ArgListCountEq.hpp"
#include "odb-compiler/tests/matchers/BlockStmntCountEq.hpp"
#include "odb-compiler/tests/matchers/InitializerListCountEq.hpp"
#include "odb-compiler/tests/matchers/LiteralEq.hpp"
#include "odb-compiler/tests/matchers/ScopedAnnotatedSymbolEq.hpp"
#include "odb-compiler/tests/matchers/SymbolEq.hpp"
#include "odb-compiler/tests/matchers/UDTRefEq.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"

#define NAME db_parser_udt_var_decl

using namespace testing;
using namespace odb;
using namespace ast;

class NAME : public ParserTestHarness
{
public:
};

/*
 * All possible valid udt body declarations:
 *
 *     type udt
 *         var as double integer
 *         var as integer
 *         var as dword
 *         var as word
 *         var as byte
 *         var as boolean
 *         var as double float
 *         var as float
 *         var# as double float
 *         var# as float
 *         var as string
 *         var$ as string
 *         var as nestedudt
 *
 *         dim var(...) as double integer
 *         dim var(...) as integer
 *         dim var(...) as dword
 *         dim var(...) as word
 *         dim var(...) as byte
 *         dim var(...) as boolean
 *         dim var(...) as double float
 *         dim var(...) as float
 *         dim var#(...) as double float
 *         dim var#(...) as float
 *         dim var(...) as string
 *         dim var$(...) as string
 *         dim var(...) as nestedudt
 *     endtype
 *
 * All possible valid udt type declarations:
 *
 *     type udt
 *         ...
 *     endtype
 *
 * Invalid udt body declarations:
 *
 *     type udt
 *         local var as double integer
 *         local var as integer
 *         local var as dword
 *         local var as word
 *         local var as byte
 *         local var as boolean
 *         local var as double float
 *         local var as float
 *         local var# as double float
 *         local var# as float
 *         local var as string
 *         local var$ as string
 *         local var as nestedudt
 *
 *         local dim var(...) as double integer
 *         local dim var(...) as integer
 *         local dim var(...) as dword
 *         local dim var(...) as word
 *         local dim var(...) as byte
 *         local dim var(...) as boolean
 *         local dim var(...) as double float
 *         local dim var(...) as float
 *         local dim var#(...) as double float
 *         local dim var#(...) as float
 *         local dim var(...) as string
 *         local dim var$(...) as string
 *         local dim var(...) as nestedudt
 *
 *         global var as double integer
 *         global var as integer
 *         global var as dword
 *         global var as word
 *         global var as byte
 *         global var as boolean
 *         global var as double float
 *         global var as float
 *         global var# as double float
 *         global var# as float
 *         global var as string
 *         global var$ as string
 *         global var as nestedudt
 *
 *         global dim var(...) as double integer
 *         global dim var(...) as integer
 *         global dim var(...) as dword
 *         global dim var(...) as word
 *         global dim var(...) as byte
 *         global dim var(...) as boolean
 *         global dim var(...) as double float
 *         global dim var(...) as float
 *         global dim var#(...) as double float
 *         global dim var#(...) as float
 *         global dim var(...) as string
 *         global dim var$(...) as string
 *         global dim var(...) as nestedudt
 *
 *         var# as double integer
 *         var# as integer
 *         var# as dword
 *         var# as word
 *         var# as byte
 *         var# as boolean
 *         var# as string
 *         var# as nestedudt
 *         var as nestedudt#
 *
 *         var$ as double integer
 *         var$ as integer
 *         var$ as dword
 *         var$ as word
 *         var$ as byte
 *         var$ as boolean
 *         var$ as double float
 *         var$ as float
 *         var$ as nestedudt
 *         var as nestedudt$
 *
 *         dim var#(...) as double integer
 *         dim var#(...) as integer
 *         dim var#(...) as dword
 *         dim var#(...) as word
 *         dim var#(...) as byte
 *         dim var#(...) as boolean
 *         dim var#(...) as string
 *         dim var#(...) as nestedudt
 *         dim var(...) as nestedudt#
 *
 *         dim var$(...) as double integer
 *         dim var$(...) as integer
 *         dim var$(...) as dword
 *         dim var$(...) as word
 *         dim var$(...) as byte
 *         dim var$(...) as boolean
 *         dim var$(...) as double float
 *         dim var$(...) as float
 *         dim var$(...) as nestedudt
 *         dim var(...) as nestedudt$
 *
 *         dim var() as double integer
 *         dim var() as integer
 *         dim var() as dword
 *         dim var() as word
 *         dim var() as byte
 *         dim var() as boolean
 *         dim var() as double float
 *         dim var() as float
 *         dim var#() as double float
 *         dim var#() as float
 *         dim var() as string
 *         dim var$() as string
 *         dim var() as nestedudt
 *     endtype
 *
 * Invalid udt declarations:
 *
 *     type udt#
 *         ...
 *     endtype
 *
 *     type udt$
 *         ...
 *     endtype
 */

TEST_F(NAME, local_var_decl_invalid_double_integer)
{
    ast = driver->parse("test",
        "type udt\n"
        "    local var as double integer\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_var_decl_invalid_integer)
{
    ast = driver->parse("test",
        "type udt\n"
        "    local var as integer\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_var_decl_invalid_dword)
{
    ast = driver->parse("test",
        "type udt\n"
        "    local var as dword\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_var_decl_invalid_word)
{
    ast = driver->parse("test",
        "type udt\n"
        "    local var as word\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_var_decl_invalid_byte)
{
    ast = driver->parse("test",
        "type udt\n"
        "    local var as byte\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_var_decl_invalid_boolean)
{
    ast = driver->parse("test",
        "type udt\n"
        "    local var as boolean\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_var_decl_invalid_double_float)
{
    ast = driver->parse("test",
        "type udt\n"
        "    local var as double float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_var_decl_invalid_float)
{
    ast = driver->parse("test",
        "type udt\n"
        "    local var as float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_float_var_decl_invalid_double_float)
{
    ast = driver->parse("test",
        "type udt\n"
        "    local var# as double float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_float_var_decl_invalid_float)
{
    ast = driver->parse("test",
        "type udt\n"
        "    local var# as float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_var_decl_invalid_string)
{
    ast = driver->parse("test",
        "type udt\n"
        "    local var as string\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_string_var_decl_invalid_string)
{
    ast = driver->parse("test",
        "type udt\n"
        "    local var$ as string\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_var_decl_invalid_nested_udt)
{
    ast = driver->parse("test",
        "type udt\n"
        "    local var as nestedudt\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_array_decl_invalid_double_integer)
{
    ast = driver->parse("test",
        "type udt\n"
        "    local dim arr(2, 3) as double integer\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_array_decl_invalid_integer)
{
    ast = driver->parse("test",
        "type udt\n"
        "    local dim arr(2, 3) as integer\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_array_decl_invalid_dword)
{
    ast = driver->parse("test",
        "type udt\n"
        "    local dim arr(2, 3) as dword\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_array_decl_invalid_word)
{
    ast = driver->parse("test",
        "type udt\n"
        "    local dim arr(2, 3) as word\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_array_decl_invalid_byte)
{
    ast = driver->parse("test",
        "type udt\n"
        "    local dim arr(2, 3) as byte\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_array_decl_invalid_boolean)
{
    ast = driver->parse("test",
        "type udt\n"
        "    local dim arr(2, 3) as boolean\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_array_decl_invalid_double_float)
{
    ast = driver->parse("test",
        "type udt\n"
        "    local dim arr(2, 3) as double float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_array_decl_invalid_float)
{
    ast = driver->parse("test",
        "type udt\n"
        "    local dim arr(2, 3) as float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_float_array_decl_invalid_double_float)
{
    ast = driver->parse("test",
        "type udt\n"
        "    local dim arr#(2, 3) as double float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_float_array_decl_invalid_float)
{
    ast = driver->parse("test",
        "type udt\n"
        "    local dim arr#(2, 3) as float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_array_decl_invalid_string)
{
    ast = driver->parse("test",
        "type udt\n"
        "    local dim arr(2, 3) as string\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_string_array_decl_invalid_string)
{
    ast = driver->parse("test",
        "type udt\n"
        "    local dim arr$(2, 3) as string\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_array_decl_invalid_nested_udt)
{
    ast = driver->parse("test",
        "type udt\n"
        "    local dim arr(2, 3) as nestedudt\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_var_decl_invalid_double_integer)
{
    ast = driver->parse("test",
        "type udt\n"
        "    global var as double integer\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_var_decl_invalid_integer)
{
    ast = driver->parse("test",
        "type udt\n"
        "    global var as integer\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_var_decl_invalid_dword)
{
    ast = driver->parse("test",
        "type udt\n"
        "    global var as dword\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_var_decl_invalid_word)
{
    ast = driver->parse("test",
        "type udt\n"
        "    global var as word\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_var_decl_invalid_byte)
{
    ast = driver->parse("test",
        "type udt\n"
        "    global var as byte\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_var_decl_invalid_boolean)
{
    ast = driver->parse("test",
        "type udt\n"
        "    global var as boolean\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_var_decl_invalid_double_float)
{
    ast = driver->parse("test",
        "type udt\n"
        "    global var as double float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_var_decl_invalid_float)
{
    ast = driver->parse("test",
        "type udt\n"
        "    global var as float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_float_var_decl_invalid_double_float)
{
    ast = driver->parse("test",
        "type udt\n"
        "    global var# as double float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_float_var_decl_invalid_float)
{
    ast = driver->parse("test",
        "type udt\n"
        "    global var# as float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_var_decl_invalid_string)
{
    ast = driver->parse("test",
        "type udt\n"
        "    global var as string\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_string_var_decl_invalid_string)
{
    ast = driver->parse("test",
        "type udt\n"
        "    global var$ as string\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_var_decl_invalid_nested_udt)
{
    ast = driver->parse("test",
        "type udt\n"
        "    global var as nestedudt\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_array_decl_invalid_double_integer)
{
    ast = driver->parse("test",
        "type udt\n"
        "    global dim arr(2, 3) as double integer\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_array_decl_invalid_integer)
{
    ast = driver->parse("test",
        "type udt\n"
        "    global dim arr(2, 3) as integer\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_array_decl_invalid_dword)
{
    ast = driver->parse("test",
        "type udt\n"
        "    global dim arr(2, 3) as dword\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_array_decl_invalid_word)
{
    ast = driver->parse("test",
        "type udt\n"
        "    global dim arr(2, 3) as word\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_array_decl_invalid_byte)
{
    ast = driver->parse("test",
        "type udt\n"
        "    global dim arr(2, 3) as byte\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_array_decl_invalid_boolean)
{
    ast = driver->parse("test",
        "type udt\n"
        "    global dim arr(2, 3) as boolean\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_array_decl_invalid_double_float)
{
    ast = driver->parse("test",
        "type udt\n"
        "    global dim arr(2, 3) as double float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_array_decl_invalid_float)
{
    ast = driver->parse("test",
        "type udt\n"
        "    global dim arr(2, 3) as float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_float_array_decl_invalid_double_float)
{
    ast = driver->parse("test",
        "type udt\n"
        "    global dim arr#(2, 3) as double float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_float_array_decl_invalid_float)
{
    ast = driver->parse("test",
        "type udt\n"
        "    global dim arr#(2, 3) as float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_array_decl_invalid_string)
{
    ast = driver->parse("test",
        "type udt\n"
        "    global dim arr(2, 3) as string\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_string_array_decl_invalid_string)
{
    ast = driver->parse("test",
        "type udt\n"
        "    global dim arr$(2, 3) as string\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_array_decl_invalid_nested_udt)
{
    ast = driver->parse("test",
        "type udt\n"
        "    global dim arr(2, 3) as nestedudt\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_var_decl_as_double_integer_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var# as double integer\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_var_decl_as_integer_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var# as integer\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_var_decl_as_dword_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var# as dword\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_var_decl_as_word_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var# as word\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_var_decl_as_byte_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var# as byte\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_var_decl_as_boolean_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var# as boolean\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_var_decl_as_string_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var# as string\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_var_decl_as_nested_udt_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var# as nestedudt\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, udt_type_with_float_annotation_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var as nestedudt#\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_var_decl_as_double_integer_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var$ as double integer\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_var_decl_as_integer_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var$ as integer\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_var_decl_as_dword_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var$ as dword\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_var_decl_as_word_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var$ as word\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_var_decl_as_byte_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var$ as byte\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_var_decl_as_boolean_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var$ as boolean\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_var_decl_as_double_float_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var$ as double float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_var_decl_as_float_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var$ as float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_var_decl_as_nested_udt_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var$ as nestedudt\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, udt_type_with_string_annotation_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var as nestedudt$\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_array_decl_as_double_integer_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr#(2, 3) as double integer\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_array_decl_as_integer_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr#(2, 3) as integer\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_array_decl_as_dword_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr#(2, 3) as dword\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_array_decl_as_word_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr#(2, 3) as word\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_array_decl_as_byte_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr#(2, 3) as byte\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_array_decl_as_boolean_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr#(2, 3) as boolean\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_array_decl_as_string_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr#(2, 3) as string\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_array_decl_as_nested_udt_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr#(2, 3) as nestedudt\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, array_udt_type_with_float_annotation_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr(2, 3) as nestedudt#\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_array_decl_as_double_integer_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr$(2, 3) as double integer\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_array_decl_as_integer_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr$(2, 3) as integer\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_array_decl_as_dword_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr$(2, 3) as dword\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_array_decl_as_word_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr$(2, 3) as word\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_array_decl_as_byte_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr$(2, 3) as byte\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_array_decl_as_boolean_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr$(2, 3) as boolean\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_array_decl_as_double_float_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr$(2, 3) as double float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_array_decl_as_float_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr$(2, 3) as float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_array_decl_as_nested_udt_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr$(2, 3) as nestedudt\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_udt_type_with_string_annotation_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr(2, 3) as nestedudt$\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_array_decl_as_double_integer)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr() as double integer\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_array_decl_as_integer)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr() as integer\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_array_decl_as_dword)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr() as dword\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_array_decl_as_word)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr() as word\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_array_decl_as_byte)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr() as byte\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_array_decl_as_boolean)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr() as boolean\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_array_decl_as_double_float)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr() as double float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_array_decl_as_float)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr() as float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_float_array_decl_as_double_float)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr#() as double float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_float_array_decl_as_float)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr#() as float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_array_decl_as_string)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr() as string\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_string_array_decl_as_string)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr$() as string\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_array_decl_as_nested_udt)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr() as nestedudt\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, udt_type_annotated_float_is_invalid)
{
    ast = driver->parse("test",
        "type udt#\n"
        "    x as integer\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, udt_type_annotated_string_is_invalid)
{
    ast = driver->parse("test",
        "type udt$\n"
        "    x as integer\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_udt_is_invalid)
{
    ast = driver->parse("test",
        "type udt\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, var_decl_as_double_integer)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var as double integer\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("udt"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTDeclBody(_)).After(exp);
    exp = EXPECT_CALL(v, visitDoubleIntegerVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitInitializerList(InitializerListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleIntegerLiteral(DoubleIntegerLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_decl_as_integer)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var as integer\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("udt"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTDeclBody(_)).After(exp);
    exp = EXPECT_CALL(v, visitIntegerVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitInitializerList(InitializerListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitIntegerLiteral(IntegerLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_decl_as_dword)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var as dword\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("udt"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTDeclBody(_)).After(exp);
    exp = EXPECT_CALL(v, visitDwordVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitInitializerList(InitializerListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitDwordLiteral(DwordLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_decl_as_word)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var as word\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("udt"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTDeclBody(_)).After(exp);
    exp = EXPECT_CALL(v, visitWordVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitInitializerList(InitializerListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitWordLiteral(WordLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_decl_as_byte)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var as byte\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("udt"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTDeclBody(_)).After(exp);
    exp = EXPECT_CALL(v, visitByteVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitInitializerList(InitializerListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_decl_as_boolean)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var as boolean\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("udt"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTDeclBody(_)).After(exp);
    exp = EXPECT_CALL(v, visitBooleanVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitInitializerList(InitializerListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitBooleanLiteral(BooleanLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_decl_as_double_float)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var as double float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("udt"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTDeclBody(_)).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitInitializerList(InitializerListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_decl_as_float)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var as float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("udt"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTDeclBody(_)).After(exp);
    exp = EXPECT_CALL(v, visitFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitInitializerList(InitializerListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, float_var_decl_as_float)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var# as float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("udt"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTDeclBody(_)).After(exp);
    exp = EXPECT_CALL(v, visitFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::FLOAT, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitInitializerList(InitializerListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_decl_as_string)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var as string\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("udt"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTDeclBody(_)).After(exp);
    exp = EXPECT_CALL(v, visitStringVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitInitializerList(InitializerListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitStringLiteral(StringLiteralEq(""))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, string_var_decl_as_string)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var$ as string\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("udt"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTDeclBody(_)).After(exp);
    exp = EXPECT_CALL(v, visitStringVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::STRING, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitInitializerList(InitializerListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitStringLiteral(StringLiteralEq(""))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_decl_as_nested_udt)
{
    ast = driver->parse("test",
        "type udt\n"
        "    var as nestedudt\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("udt"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTDeclBody(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTRef(UDTRefEq("nestedudt"))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, arr_decl_as_double_integer)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr(2, 3) as double integer\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("udt"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTDeclBody(_)).After(exp);
    exp = EXPECT_CALL(v, visitDoubleIntegerArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, arr_decl_as_integer)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr(2, 3) as integer\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("udt"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTDeclBody(_)).After(exp);
    exp = EXPECT_CALL(v, visitIntegerArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, arr_decl_as_dword)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr(2, 3) as dword\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("udt"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTDeclBody(_)).After(exp);
    exp = EXPECT_CALL(v, visitDwordArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, arr_decl_as_word)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr(2, 3) as word\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("udt"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTDeclBody(_)).After(exp);
    exp = EXPECT_CALL(v, visitWordArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, arr_decl_as_byte)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr(2, 3) as byte\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("udt"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTDeclBody(_)).After(exp);
    exp = EXPECT_CALL(v, visitByteArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, arr_decl_as_boolean)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr(2, 3) as boolean\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("udt"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTDeclBody(_)).After(exp);
    exp = EXPECT_CALL(v, visitBooleanArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, arr_decl_as_double_float)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr(2, 3) as double float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("udt"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTDeclBody(_)).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, arr_decl_as_float)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr(2, 3) as float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("udt"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTDeclBody(_)).After(exp);
    exp = EXPECT_CALL(v, visitFloatArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, float_arr_decl_as_float)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr#(2, 3) as float\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("udt"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTDeclBody(_)).After(exp);
    exp = EXPECT_CALL(v, visitFloatArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::FLOAT, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, arr_decl_as_string)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr(2, 3) as string\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("udt"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTDeclBody(_)).After(exp);
    exp = EXPECT_CALL(v, visitStringArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, string_arr_decl_as_string)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr$(2, 3) as string\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("udt"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTDeclBody(_)).After(exp);
    exp = EXPECT_CALL(v, visitStringArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::STRING, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, arr_decl_as_nested_udt)
{
    ast = driver->parse("test",
        "type udt\n"
        "    dim arr(2, 3) as nestedudt\n"
        "endtype\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("udt"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTDeclBody(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);
    exp = EXPECT_CALL(v, visitUDTRef(UDTRefEq("nestedudt"))).After(exp);

    ast->accept(&v);
}
