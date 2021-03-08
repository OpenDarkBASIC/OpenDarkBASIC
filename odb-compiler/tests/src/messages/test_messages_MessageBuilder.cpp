#include "gmock/gmock.h"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/messages/MessageBuilder.hpp"

#define NAME messages

using namespace odb;

TEST(NAME, unexpected_x_expected_y_z)
{
    auto message = msg::MessageBuilder(msg::DB_PARSER)
        .syntaxError()
        .location(new ast::InlineSourceLocation("test", "variable anothervariable", 1, 1, 10, 25))
        .unexpected("symbol")
        .expected(".", "=")
        .build();
}

TEST(NAME, command_same_as_keyword_warning)
{
    auto message = msg::MessageBuilder(msg::DB_PARSER)
        .syntaxWarning()
        .location(new ast::InlineSourceLocation("test", "do : sync : loop", 1, 1, 1, 3))
        .text("Command").quote("do").text("has same name as built-in keyword. Command will be ignored")
        .notice()
        .text("This is normal behavior for DBPro plugins, but should not be ignored if using the ODB SDK")
        .build();
}

TEST(NAME, multiple_default_cases_error)
{
    const char* src =
        "select x\n"
        "  case default:\n"
        "  endcase\n"
        "  case default:\n"
        "  endcase\n"
        "  case default:\n"
        "  endcase\n"
        "endselect\n";

    auto builder = msg::MessageBuilder(msg::ASTPOST);
    builder
        .syntaxError()
        .location(new ast::InlineSourceLocation("test", src, 1, 1, 1, 9))
        .text("Select statement has multiple default cases")
        ;

    for (int i = 0; i != 2; ++i)
    {
        static const char* table[] = {"first", "second", "third"};
        builder
            .notice()
            .location(new ast::InlineSourceLocation("test", src, (i*2)+1, (i*2)+1, 3, 15))
            .text(table[i]).text("default case defined here")
            ;
    }

    auto message = builder.build();
}

TEST(NAME, binary_op_warning)
{
    const char* src = "result = foo() .. bar()\n";
    auto lhs = new ast::InlineSourceLocation("test", src, 1, 1, 10, 15);
    auto op  = new ast::InlineSourceLocation("test", src, 1, 1, 16, 18);
    auto rhs = new ast::InlineSourceLocation("test", src, 1, 1, 19, 24);

    auto message = msg::MessageBuilder(msg::ASTPOST)
        .semanticError()
        .binaryOpLocation(lhs, op, rhs)
        .text("RHS of bitwise-not operator causes side effects")
        .build();
}
