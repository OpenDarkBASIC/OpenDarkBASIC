#include "odb-compiler/ast/AnnotatedSymbol.hpp"
#include "odb-compiler/ast/Assignment.hpp"
#include "odb-compiler/ast/BinaryOp.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/DepthFirstIterator.hpp"
#include "odb-compiler/ast/FuncCall.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"

using namespace odb;
using namespace ast;

struct ASTDepthFirstIteratorTest : public ParserTestHarness
{
};

TEST_F(ASTDepthFirstIteratorTest, EmptyRange)
{
    auto range = depthFirst(static_cast<Node*>(nullptr));
    EXPECT_EQ(range.begin(), range.end());
    EXPECT_TRUE(range.empty());
}

TEST_F(ASTDepthFirstIteratorTest, SingleNodeTraversal)
{
    Reference<ByteLiteral> literalNode = new ByteLiteral(10, new InlineSourceLocation("", "", 0, 0, 0, 0));
    auto range = depthFirst(literalNode);
    auto begin = range.begin();
    auto end = range.end();

    ASSERT_NE(dynamic_cast<ByteLiteral*>(*begin), nullptr);
    EXPECT_EQ(dynamic_cast<ByteLiteral*>(*begin)->value(), 10);
    begin++;
    EXPECT_EQ(begin, end);
    // Once begin == end, this should be true even if we increment again.
    begin++;
    EXPECT_EQ(begin, end);
    end++;
    EXPECT_EQ(begin, end);
}

TEST_F(ASTDepthFirstIteratorTest, TraversalOrder)
{
    parse("result = 1 + foo()");
    auto range = depthFirst(ast);
    std::vector<Node*> nodes(range.begin(), range.end());
    ASSERT_EQ(nodes.size(), 8);

    // We expect these nodes at these positions.
    EXPECT_NE(dynamic_cast<Block*>(nodes[0]), nullptr);
    EXPECT_NE(dynamic_cast<VarAssignment*>(nodes[1]), nullptr);
    EXPECT_NE(dynamic_cast<VarRef*>(nodes[2]), nullptr);
    EXPECT_NE(dynamic_cast<AnnotatedSymbol*>(nodes[3]), nullptr);
    EXPECT_NE(dynamic_cast<BinaryOp*>(nodes[4]), nullptr);
    EXPECT_NE(dynamic_cast<ByteLiteral*>(nodes[5]), nullptr);
    EXPECT_NE(dynamic_cast<FuncCallExpr*>(nodes[6]), nullptr);
    EXPECT_NE(dynamic_cast<AnnotatedSymbol*>(nodes[7]), nullptr);

    // We expect the pointers of the nodes to be equal.
    EXPECT_EQ(nodes[0], ast);
    EXPECT_EQ(nodes[1], ast->children()[0]);
    EXPECT_EQ(nodes[2], ast->children()[0]->children()[0]);
    EXPECT_EQ(nodes[3], ast->children()[0]->children()[0]->children()[0]);
    EXPECT_EQ(nodes[4], ast->children()[0]->children()[1]);
    EXPECT_EQ(nodes[5], ast->children()[0]->children()[1]->children()[0]);
    EXPECT_EQ(nodes[6], ast->children()[0]->children()[1]->children()[1]);
    EXPECT_EQ(nodes[7], ast->children()[0]->children()[1]->children()[1]->children()[0]);
}

TEST_F(ASTDepthFirstIteratorTest, TraversalWithParents)
{
    parse("result = 1");
    auto range = depthFirst(ast);
    auto it = range.begin();

    ASSERT_NE(dynamic_cast<Block*>(*it), nullptr);
    EXPECT_EQ(it.parent(), nullptr);
    it++;

    ASSERT_NE(dynamic_cast<VarAssignment*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast);
    it++;

    ASSERT_NE(dynamic_cast<VarRef*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->statements()[0]);
    it++;

    ASSERT_NE(dynamic_cast<AnnotatedSymbol*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->statements()[0]->children()[0]);
    it++;

    ASSERT_NE(dynamic_cast<ByteLiteral*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->statements()[0]);
    it++;

    ASSERT_EQ(it, range.end());
}

TEST_F(ASTDepthFirstIteratorTest, ReplaceNodeWhilstIterating)
{
    parse("result = 1");
    auto range = depthFirst(ast);
    auto it = range.begin();

    ASSERT_NE(dynamic_cast<Block*>(*it), nullptr);
    EXPECT_EQ(it.parent(), nullptr);
    it++;

    ASSERT_NE(dynamic_cast<VarAssignment*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast);
    it++;

    ASSERT_NE(dynamic_cast<VarRef*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->statements()[0]);

    // Replace the VarRef with a different VarRef. That way, we can be sure we traverse into the _new_ children.
    Node* previousNode = *it;
    it.replaceNode(new VarRef(new AnnotatedSymbol(Annotation::FLOAT, "someFloat", (*it)->children()[0]->location()),
                              (*it)->location()));
    ASSERT_NE(dynamic_cast<VarRef*>(*it), nullptr);
    EXPECT_NE(*it, previousNode);
    EXPECT_EQ(it.parent(), ast->statements()[0]);
    it++;

    ASSERT_NE(dynamic_cast<AnnotatedSymbol*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->statements()[0]->children()[0]);
    // Ensure we traversed over the new child of the replacement VarRef.
    EXPECT_EQ(dynamic_cast<AnnotatedSymbol*>(*it)->annotation(), Annotation::FLOAT);
    EXPECT_EQ(dynamic_cast<AnnotatedSymbol*>(*it)->name(), "someFloat");
    it++;

    ASSERT_NE(dynamic_cast<ByteLiteral*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->statements()[0]);
    it++;

    ASSERT_EQ(it, range.end());
}

TEST_F(ASTDepthFirstIteratorTest, ModifyChildrenWhilstIterating)
{
    parse("result = 1");
    auto range = depthFirst(ast);
    auto it = range.begin();

    ASSERT_NE(dynamic_cast<Block*>(*it), nullptr);
    EXPECT_EQ(it.parent(), nullptr);
    it++;

    ASSERT_NE(dynamic_cast<VarAssignment*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast);
    it++;

    ASSERT_NE(dynamic_cast<VarRef*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->statements()[0]);

    // Perform the same modification as in the previous test, but instead modify the child of VarRef directly (which may
    // affect iteration).
    Node* childToReplace = dynamic_cast<VarRef*>(*it)->symbol();
    (*it)->swapChild(childToReplace, new AnnotatedSymbol(Annotation::FLOAT, "someFloat", childToReplace->location()));

    it++;

    ASSERT_NE(dynamic_cast<AnnotatedSymbol*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->statements()[0]->children()[0]);
    // Ensure we traversed over the replacement child of the VarRef.
    EXPECT_EQ(dynamic_cast<AnnotatedSymbol*>(*it)->annotation(), Annotation::FLOAT);
    EXPECT_EQ(dynamic_cast<AnnotatedSymbol*>(*it)->name(), "someFloat");
    it++;

    ASSERT_NE(dynamic_cast<ByteLiteral*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->statements()[0]);
    it++;

    ASSERT_EQ(it, range.end());
}