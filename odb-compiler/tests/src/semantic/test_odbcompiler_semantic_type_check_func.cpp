#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_integrity.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_func

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, no_return_default_to_void)
{
    ASSERT_THAT(
        parse("function test()\n"
              "endfunction\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(runSemanticCheck(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast_verify_connectivity(&ast), Eq(0));

    ast_id func = ast.nodes[0].block.stmt;
    ast_id decl = ast.nodes[func].func.decl;
    ast_id def = ast.nodes[func].func.def;
    ast_id ret = ast.nodes[def].func_def.retval;
    ASSERT_THAT(ret, Eq(-1));
    ASSERT_THAT(ast.nodes[def].info.type_info, Eq(TYPE_VOID));
    ASSERT_THAT(ast.nodes[decl].info.type_info, Eq(TYPE_VOID));
    ASSERT_THAT(ast.nodes[func].info.type_info, Eq(TYPE_VOID));
}
