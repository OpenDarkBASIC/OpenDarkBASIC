#pragma once

#include "odb-compiler/commands/CommandMatcher.hpp"
#include "odb-compiler/commands/CommandIndex.hpp"
#include "odb-sdk/Reference.hpp"
#include "gmock/gmock.h"

namespace odb {
namespace ast {
    class Block;
}
namespace db {
    class Driver;
}
}

class ParserTestHarness : public testing::Test
{
public:
    void checkParentConnectionConsistencies(const odb::ast::Block* ast);
    void SetUp() override;
    void TearDown() override;

    odb::cmd::CommandIndex cmdIndex;
    odb::cmd::CommandMatcher matcher;
    odb::db::Driver* driver;
    odb::Reference<odb::ast::Block> ast;
};
