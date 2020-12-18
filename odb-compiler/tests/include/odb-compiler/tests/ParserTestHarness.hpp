#pragma once

#include "odb-compiler/keywords/KeywordMatcher.hpp"
#include "odb-compiler/keywords/KeywordIndex.hpp"
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

    odb::kw::KeywordIndex kwIndex;
    odb::kw::KeywordMatcher matcher;
    odb::db::Driver* driver;
    odb::Reference<odb::ast::Block> ast;
};
