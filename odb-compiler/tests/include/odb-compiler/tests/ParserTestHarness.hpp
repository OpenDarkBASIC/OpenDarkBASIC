#pragma once

#include "odb-compiler/commands/CommandMatcher.hpp"
#include "odb-compiler/commands/CommandIndex.hpp"
#include "odb-sdk/Reference.hpp"
#include "gmock/gmock.h"

namespace odb::ast {
    class Block;
}
namespace odb::db {
    class FileParserDriver;
}

class ParserTestHarness : public testing::Test
{
public:
    void checkParentConnectionConsistencies(const odb::ast::Block* ast);
    void SetUp() override;
    void TearDown() override;

    odb::cmd::CommandIndex cmdIndex;
    odb::cmd::CommandMatcher matcher;
    odb::db::StringParserDriver* driver;
    odb::Reference<odb::ast::Block> ast;
};
