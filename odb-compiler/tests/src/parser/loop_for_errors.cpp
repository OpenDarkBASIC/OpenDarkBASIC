#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-sdk/tests/LogHelper.hpp"

#include "gmock/gmock.h"

#define NAME odbcompiler_db_parser_loop_for

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};
// TODO:
