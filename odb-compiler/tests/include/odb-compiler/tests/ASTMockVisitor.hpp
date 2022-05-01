#pragma once

#include "gmock/gmock.h"
#include "odb-compiler/ast/Visitor.hpp"

class ASTMockVisitor : public odb::ast::ConstVisitor
{
public:
#define X(nodeType) MOCK_METHOD(void, visit##nodeType, (const odb::ast::nodeType* node), (override));
    ODB_AST_NODE_TYPE_LIST
#undef X
};
