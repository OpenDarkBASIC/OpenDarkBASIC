#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_calculate_scope_ids

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
    bool
    subtreeHasScope(
        ast_id        n,
        int32_t       scope_id,
        enum ast_type terminate_at = AST_GC,
        bool          first = true)
    {
        if (ast_node_type(ast, n) == terminate_at)
            return true;

        if (ast->nodes[n].info.scope_id != scope_id)
            return false;

        ast_id left = ast->nodes[n].base.left;
        ast_id right = ast->nodes[n].base.right;
        if (left > -1)
            if (!subtreeHasScope(left, scope_id, terminate_at, false))
                return false;
        if (right > -1)
            if (!subtreeHasScope(right, scope_id, terminate_at, false))
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

    ASSERT_THAT(ast_count(ast), Eq(95));

    ast_id block1 = ast->root;
    ast_id x = ast->nodes[block1].block.stmt;
    ASSERT_THAT(ast->nodes[block1].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[x].info.scope_id, Eq(0));

    ast_id block2 = ast->nodes[block1].block.next;
    ASSERT_THAT(ast->nodes[block2].info.scope_id, Eq(0));
    ast_id poly1 = ast->nodes[block2].block.stmt;
    ast_id f11 = ast->nodes[poly1].func_poly.func;
    ast_id f12 = ast->nodes[f11].func1.func2;
    ast_id f13 = ast->nodes[f12].func2.func3;
    ast_id f14 = ast->nodes[f13].func3.func4;
    ast_id body1 = ast->nodes[f14].func4.body;
    ast_id ident1 = ast->nodes[f11].func1.identifier;
    ASSERT_THAT(ast->nodes[f11].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[f12].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[ident1].info.scope_id, Eq(0));
    ASSERT_THAT(subtreeHasScope(f13, 1, AST_FUNC3), IsTrue());

    ast_id block3 = body1;
    ast_id block4 = ast->nodes[block3].block.next;
    ASSERT_THAT(ast->nodes[block4].info.scope_id, Eq(1));
    ast_id poly2 = ast->nodes[block4].block.stmt;
    ast_id f21 = ast->nodes[poly2].func_poly.func;
    ast_id f22 = ast->nodes[f21].func1.func2;
    ast_id f23 = ast->nodes[f22].func2.func3;
    ast_id ident2 = ast->nodes[f21].func1.identifier;
    ASSERT_THAT(ast->nodes[f21].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[f22].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[ident2].info.scope_id, Eq(1));
    ASSERT_THAT(subtreeHasScope(f23, 2), IsTrue());

    ast_id block5 = ast->nodes[block4].block.next;
    ASSERT_THAT(ast->nodes[block5].info.scope_id, Eq(1));
    ast_id poly3 = ast->nodes[block5].block.stmt;
    ast_id f31 = ast->nodes[poly3].func_poly.func;
    ast_id f32 = ast->nodes[f31].func1.func2;
    ast_id f33 = ast->nodes[f32].func2.func3;
    ast_id ident3 = ast->nodes[f31].func1.identifier;
    ASSERT_THAT(ast->nodes[f31].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[f32].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[ident3].info.scope_id, Eq(1));
    ASSERT_THAT(subtreeHasScope(f33, 3), IsTrue());

    ast_id block6 = ast->nodes[block5].block.next;
    ASSERT_THAT(subtreeHasScope(block6, 1), Eq(1));
}
