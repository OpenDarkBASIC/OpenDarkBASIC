#include <gmock/gmock.h>
#include "odbc/parsers/db/Driver.hpp"
#include "odbc/ast/Node.hpp"
#include "odbc/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME db_udt

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odbc;
using namespace ast;

TEST_F(NAME, declare_udt)
{
    ASSERT_THAT(driver->parseString(
        "type mytype\n"
        "    x# as float\n"
        "    y# as float\n"
        "endtype\n"), IsTrue());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.statement->info.type, Eq(NT_SYM_UDT_DECL));
    ASSERT_THAT(ast->block.statement->sym.udt_decl.name, StrEq("mytype"));
    ASSERT_THAT(ast->block.statement->sym.udt_decl.subtypes_list, NotNull());

    Node* subtype = ast->block.statement->sym.udt_decl.subtypes_list;
    ASSERT_THAT(subtype->info.type, Eq(NT_UDT_SUBTYPE_LIST));
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl, NotNull());
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl->sym.var_decl.flag.datatype, Eq(SDT_FLOAT));
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl->sym.var_decl.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl->sym.var_decl.name, StrEq("x"));
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl->sym.var_decl.udt, IsNull());

    subtype = subtype->udt_subtype_list.next;
    ASSERT_THAT(subtype, NotNull());
    ASSERT_THAT(subtype->info.type, Eq(NT_UDT_SUBTYPE_LIST));
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl, NotNull());
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl->sym.var_decl.flag.datatype, Eq(SDT_FLOAT));
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl->sym.var_decl.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl->sym.var_decl.name, StrEq("y"));
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl->sym.var_decl.udt, IsNull());
    ASSERT_THAT(subtype->udt_subtype_list.next, IsNull());
}

TEST_F(NAME, declare_nested_udt)
{
    ASSERT_THAT(driver->parseString(
        "type vec2\n"
        "    x# as float\n"
        "    y# as float\n"
        "endtype\n"
        "type player\n"
        "    pos as vec2\n"
        "    health as integer\n"
        "endtype\n"), IsTrue());
}

TEST_F(NAME, declare_nested_udt_with_dims)
{
    ASSERT_THAT(driver->parseString(
        "type vec2\n"
        "    x# as float\n"
        "    y# as float\n"
        "endtype\n"
        "type player\n"
        "    dim pos(5) as vec2\n"
        "    health as integer\n"
        "endtype\n"), IsTrue());
}

TEST_F(NAME, declare_array_as_udt)
{
    ASSERT_THAT(driver->parseString(
        "type vec2\n"
        "    x# as float\n"
        "    y# as float\n"
        "endtype\n"
        "dim vecs(5) as vec2\n"), IsTrue());
}

TEST_F(NAME, ref_udt)
{
    ASSERT_THAT(driver->parseString(
        "type vec2\n"
        "    x# as float\n"
        "    y# as float\n"
        "endtype\n"
        "v as vec2\n"
        "v.x# = 2.3\n"
        "v.y# = 5.4\n"), IsTrue());
}

TEST_F(NAME, ref_udt_in_udt)
{
    ASSERT_THAT(driver->parseString(
        "type vec2\n"
        "    x# as float\n"
        "    y# as float\n"
        "endtype\n"
        "type player\n"
        "    pos as vec2\n"
        "    health as integer\n"
        "endtype\n"
        "p as player\n"
        "p.pos.x# = 2.3\n"
        "p.pos.y# = 5.4\n"), IsTrue());
}

TEST_F(NAME, ref_udt_array)
{
    ASSERT_THAT(driver->parseString(
        "type vec2\n"
        "    x# as float\n"
        "    y# as float\n"
        "endtype\n"
        "dim vecs(5) as vec2\n"
        "vecs(2).x# = 2.3\n"
        "vecs(2).y# = 5.4\n"), IsTrue());
}

TEST_F(NAME, ref_udt_array_with_arrays)
{
    ASSERT_THAT(driver->parseString(
        "type vec2\n"
        "    x# as float\n"
        "    y# as float\n"
        "endtype\n"
        "type player\n"
        "    pos(5) as vec2\n"
        "    health as integer\n"
        "endtype\n"
        "dim p(5) as player\n"
        "p(1).pos(4).x# = 2.3\n"
        "p(1).pos(4).y# = 5.4\n"), IsTrue());
}
