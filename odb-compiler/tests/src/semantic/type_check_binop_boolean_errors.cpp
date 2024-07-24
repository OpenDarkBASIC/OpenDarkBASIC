#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-sdk/tests/LogHelper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_binop_boolean_errors

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};
