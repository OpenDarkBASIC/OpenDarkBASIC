#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_calculate_scope_ids

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
    bool
    subtreeHasScope(ast_id n, int32_t scope_id)
    {
        if (ast->nodes[n].info.scope_id != scope_id)
            return false;

        ast_id left = ast->nodes[n].base.left;
        ast_id right = ast->nodes[n].base.right;
        if (left > -1)
            if (!subtreeHasScope(left, scope_id))
                return false;
        if (right > -1)
            if (!subtreeHasScope(right, scope_id))
                return false;

        return true;
    }
};

TEST_F(NAME, nested_functions)
{
    const char* source
        = "GLOBAL x# AS FLOAT\n"
          "FUNCTION func1(a, b, c)\n"
          "  a = b + c\n"
          "  FUNCTION func2(d, e, f)\n"
          "    d = e + f\n"
          "  ENDFUNCTION d\n"
          "  FUNCTION func3(g, h, i)\n"
          "    g = h + i\n"
          "  ENDFUNCTION g\n"
          "  b = c + a\n"
          "ENDFUNCTION a\n";
    ASSERT_THAT(parse(source), Eq(0));
    ASSERT_THAT(semantic(&semantic_calculate_scope_ids), Eq(0));

    ASSERT_THAT(ast_count(ast), Eq(62));

    ast_id block1 = ast->root;
    ast_id x = ast->nodes[block1].block.stmt;
    ASSERT_THAT(ast->nodes[block1].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[x].info.scope_id, Eq(0));

    ast_id block2 = ast->nodes[block1].block.next;
    ASSERT_THAT(ast->nodes[block2].info.scope_id, Eq(0));
    ast_id func1 = ast->nodes[block2].block.stmt;
    ast_id decl1 = ast->nodes[func1].func.decl;
    ast_id def1 = ast->nodes[func1].func.def;
    ast_id ident1 = ast->nodes[decl1].func_decl.identifier;
    ast_id paramlist1 = ast->nodes[decl1].func_decl.paramlist;
    ast_id body1 = ast->nodes[def1].func_def.body;
    ast_id ret1 = ast->nodes[def1].func_def.retval;
    ASSERT_THAT(ast->nodes[func1].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[decl1].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[def1].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[ident1].info.scope_id, Eq(0));
    ASSERT_THAT(subtreeHasScope(paramlist1, 1), IsTrue());
    ASSERT_THAT(ast->nodes[ret1].info.scope_id, Eq(1));

    ast_id block3 = body1;
    ast_id block4 = ast->nodes[block3].block.next;
    ASSERT_THAT(ast->nodes[block4].info.scope_id, Eq(1));
    ast_id func2 = ast->nodes[block4].block.stmt;
    ast_id decl2 = ast->nodes[func2].func.decl;
    ast_id def2 = ast->nodes[func2].func.def;
    ast_id ident2 = ast->nodes[decl2].func_decl.identifier;
    ast_id paramlist2 = ast->nodes[decl2].func_decl.paramlist;
    ast_id body2 = ast->nodes[def2].func_def.body;
    ast_id ret2 = ast->nodes[def2].func_def.retval;
    ASSERT_THAT(ast->nodes[func2].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[decl2].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[def2].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[ident2].info.scope_id, Eq(1));
    ASSERT_THAT(subtreeHasScope(body2, 2), IsTrue());
    ASSERT_THAT(subtreeHasScope(paramlist2, 2), IsTrue());
    ASSERT_THAT(ast->nodes[ret2].info.scope_id, Eq(2));

    ast_id block5 = ast->nodes[block4].block.next;
    ASSERT_THAT(ast->nodes[block5].info.scope_id, Eq(1));
    ast_id func3 = ast->nodes[block5].block.stmt;
    ast_id decl3 = ast->nodes[func3].func.decl;
    ast_id def3 = ast->nodes[func3].func.def;
    ast_id ident3 = ast->nodes[decl3].func_decl.identifier;
    ast_id paramlist3 = ast->nodes[decl3].func_decl.paramlist;
    ast_id body3 = ast->nodes[def3].func_def.body;
    ast_id ret3 = ast->nodes[def3].func_def.retval;
    ASSERT_THAT(ast->nodes[func3].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[decl3].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[def3].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[ident3].info.scope_id, Eq(1));
    ASSERT_THAT(subtreeHasScope(paramlist3, 3), IsTrue());
    ASSERT_THAT(subtreeHasScope(body3, 3), IsTrue());
    ASSERT_THAT(ast->nodes[ret3].info.scope_id, Eq(3));

    ast_id block6 = ast->nodes[block5].block.next;
    ASSERT_THAT(ast->nodes[block6].info.scope_id, Eq(1));
}
