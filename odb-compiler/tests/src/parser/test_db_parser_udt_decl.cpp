#include <gmock/gmock.h>
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME db_parser_udt

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odb;
using namespace ast;

TEST_F(NAME, declare_udt)
{
    ASSERT_THAT(driver->parseString(
        "type mytype\n"
        "    x# as float\n"
        "    y# as float\n"
        "endtype\n"), IsTrue());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_SYM_UDT_DECL));
    ASSERT_THAT(ast->block.stmnt->sym.udt_decl.name, StrEq("mytype"));
    ASSERT_THAT(ast->block.stmnt->sym.udt_decl.subtypes, NotNull());

    Node* subtype = ast->block.stmnt->sym.udt_decl.subtypes;
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

    Node* block = ast;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.stmnt, NotNull());
    ASSERT_THAT(block->block.stmnt->info.type, Eq(NT_SYM_UDT_DECL));
    ASSERT_THAT(block->block.stmnt->sym.udt_decl.name, StrEq("vec2"));

    Node* subtype = block->block.stmnt->sym.udt_decl.subtypes;
    ASSERT_THAT(subtype, NotNull());
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

    block = block->block.next;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.stmnt, NotNull());
    ASSERT_THAT(block->block.stmnt->info.type, Eq(NT_SYM_UDT_DECL));
    ASSERT_THAT(block->block.stmnt->sym.udt_decl.name, StrEq("player"));

    subtype = block->block.stmnt->sym.udt_decl.subtypes;
    ASSERT_THAT(subtype, NotNull());
    ASSERT_THAT(subtype->info.type, Eq(NT_UDT_SUBTYPE_LIST));
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl, NotNull());
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl->sym.var_decl.flag.datatype, Eq(SDT_UDT));
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl->sym.var_decl.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl->sym.var_decl.name, StrEq("pos"));
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl->sym.var_decl.udt, NotNull());
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl->sym.var_decl.udt->info.type, Eq(NT_SYM_UDT_TYPE_REF));
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl->sym.var_decl.udt->sym.udt_type_ref.name, StrEq("vec2"));
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl->sym.var_decl.udt->sym.udt_type_ref.flag.datatype, Eq(SDT_UDT));

    subtype = subtype->udt_subtype_list.next;
    ASSERT_THAT(subtype, NotNull());
    ASSERT_THAT(subtype->info.type, Eq(NT_UDT_SUBTYPE_LIST));
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl, NotNull());
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl->sym.var_decl.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl->sym.var_decl.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl->sym.var_decl.name, StrEq("health"));
    ASSERT_THAT(subtype->udt_subtype_list.sym_decl->sym.var_decl.udt, IsNull());
    ASSERT_THAT(subtype->udt_subtype_list.next, IsNull());
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
        "dim vecs(5) as vec2\n"), IsTrue());
}
