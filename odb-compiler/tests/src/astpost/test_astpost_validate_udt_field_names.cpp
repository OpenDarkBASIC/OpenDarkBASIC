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

/*
 * NOTE: Most of the field name validations are already covered in test_db_parser_udt_fields.cpp
 * Missing tests are
 * CommandExpr/CommandStmnt
 * FuncCallExpr/FuncCallStmnt
 */
