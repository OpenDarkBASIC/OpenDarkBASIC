#include "odb-compiler/ast/Assignment.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/Identifier.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/TreeIterator.hpp"
#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"

using namespace odb;
using namespace ast;

// Naive recursive pre-order (top down) traversal for testing.
void recursivePreOrderTraversal(Node* root, const std::function<void(Node*)>& functor)
{
    functor(root);
    for (auto child : root->children())
    {
        recursivePreOrderTraversal(child, functor);
    }
}

// Naive recursive post-order (bottom up) traversal for testing.
void recursivePostOrderTraversal(Node* root, const std::function<void(Node*)>& functor)
{
    for (auto child : root->children())
    {
        recursivePostOrderTraversal(child, functor);
    }
    functor(root);
}

struct ASTPreOrderIteratorTest : public ParserTestHarness
{
};

TEST_F(ASTPreOrderIteratorTest, EmptyRange)
{
    auto range = preOrderTraversal(static_cast<Node*>(nullptr));
    EXPECT_EQ(range.begin(), range.end());
    EXPECT_TRUE(range.empty());
}

TEST_F(ASTPreOrderIteratorTest, SingleNodeTraversal)
{
    Reference<ByteLiteral> literalNode = new ByteLiteral(ast, new InlineSourceLocation("", "", 0, 0, 0, 0), 10);
    auto range = preOrderTraversal(literalNode);
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

TEST_F(ASTPreOrderIteratorTest, TraversalOrder)
{
    parse("result = 1 + foo()");

    auto range = preOrderTraversal(ast);
    std::vector<Node*> nodes(range.begin(), range.end());

    std::vector<Node*> expectedNodes;
    recursivePreOrderTraversal(ast, [&](Node* n) { expectedNodes.push_back(n); });

    ASSERT_EQ(nodes.size(), expectedNodes.size());
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        EXPECT_EQ(nodes[i], expectedNodes[i]);
    }
}

TEST_F(ASTPreOrderIteratorTest, TraversalWithParents)
{
    parse("result = 1");
    auto range = preOrderTraversal(ast);
    auto it = range.begin();

    ASSERT_NE(dynamic_cast<Program*>(*it), nullptr);
    EXPECT_EQ(it.parent(), nullptr);
    it++;

    ASSERT_NE(dynamic_cast<Block*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast);
    it++;

    ASSERT_NE(dynamic_cast<VarAssignment*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->body());
    it++;

    ASSERT_NE(dynamic_cast<VarRef*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->body()->statements()[0]);
    it++;

    ASSERT_NE(dynamic_cast<Identifier*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->body()->statements()[0]->children()[0]);
    it++;

    ASSERT_NE(dynamic_cast<ByteLiteral*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->body()->statements()[0]);
    it++;

    ASSERT_EQ(it, range.end());
}

TEST_F(ASTPreOrderIteratorTest, ReplaceNodeWhilstIterating)
{
    parse("result = 1");
    auto range = preOrderTraversal(ast);
    auto it = range.begin();

    ASSERT_NE(dynamic_cast<Program*>(*it), nullptr);
    EXPECT_EQ(it.parent(), nullptr);
    it++;

    ASSERT_NE(dynamic_cast<Block*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast);
    it++;

    ASSERT_NE(dynamic_cast<VarAssignment*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->body());
    it++;

    ASSERT_NE(dynamic_cast<VarRef*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->body()->statements()[0]);

    // Replace the VarRef with a different VarRef. That way, we can be sure we traverse into the _new_ children.
    Node* previousNode = *it;
    it.replaceNode(new VarRef(ast, (*it)->location(), new Identifier(ast, (*it)->children()[0]->location(), "someFloat", Annotation::FLOAT)));
    ASSERT_NE(dynamic_cast<VarRef*>(*it), nullptr);
    EXPECT_NE(*it, previousNode);
    EXPECT_EQ(it.parent(), ast->body()->statements()[0]);
    it++;

    ASSERT_NE(dynamic_cast<Identifier*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->body()->statements()[0]->children()[0]);
    // Ensure we traversed over the new child of the replacement VarRef.
    EXPECT_EQ(dynamic_cast<Identifier*>(*it)->name(), "someFloat");
    EXPECT_EQ(dynamic_cast<Identifier*>(*it)->annotation(), Annotation::FLOAT);
    it++;

    ASSERT_NE(dynamic_cast<ByteLiteral*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->body()->statements()[0]);
    it++;

    ASSERT_EQ(it, range.end());
}

TEST_F(ASTPreOrderIteratorTest, ModifyChildrenWhilstIterating)
{
    parse("result = 1");
    auto range = preOrderTraversal(ast);
    auto it = range.begin();

    ASSERT_NE(dynamic_cast<Program*>(*it), nullptr);
    EXPECT_EQ(it.parent(), nullptr);
    it++;

    ASSERT_NE(dynamic_cast<Block*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast);
    it++;

    ASSERT_NE(dynamic_cast<VarAssignment*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->body());
    it++;

    ASSERT_NE(dynamic_cast<VarRef*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->body()->statements()[0]);

    // Perform the same modification as in the previous test, but instead modify the child of VarRef directly (which may
    // affect iteration).
    Node* childToReplace = dynamic_cast<VarRef*>(*it)->identifier();
    (*it)->swapChild(childToReplace, new Identifier(ast, childToReplace->location(), "someFloat", Annotation::FLOAT));

    it++;

    ASSERT_NE(dynamic_cast<Identifier*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->body()->statements()[0]->children()[0]);
    // Ensure we traversed over the replacement child of the VarRef.
    EXPECT_EQ(dynamic_cast<Identifier*>(*it)->name(), "someFloat");
    EXPECT_EQ(dynamic_cast<Identifier*>(*it)->annotation(), Annotation::FLOAT);
    it++;

    ASSERT_NE(dynamic_cast<ByteLiteral*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->body()->statements()[0]);
    it++;

    ASSERT_EQ(it, range.end());
}

struct ASTPostOrderIteratorTest : public ParserTestHarness
{
};

TEST_F(ASTPostOrderIteratorTest, EmptyRange)
{
    auto range = postOrderTraversal(static_cast<Node*>(nullptr));
    EXPECT_EQ(range.begin(), range.end());
    EXPECT_TRUE(range.empty());
}

TEST_F(ASTPostOrderIteratorTest, SingleNodeTraversal)
{
    Reference<ByteLiteral> literalNode = new ByteLiteral(ast, new InlineSourceLocation("", "", 0, 0, 0, 0), 10);
    auto range = postOrderTraversal(literalNode);
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

TEST_F(ASTPostOrderIteratorTest, TraversalOrder)
{
    parse("result = 1 + foo()");
    auto range = postOrderTraversal(ast);
    std::vector<Node*> nodes(range.begin(), range.end());

    std::vector<Node*> expectedNodes;
    recursivePostOrderTraversal(ast, [&](Node* n) { expectedNodes.push_back(n); });

    ASSERT_EQ(nodes.size(), expectedNodes.size());
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        EXPECT_EQ(nodes[i], expectedNodes[i]) << "at index " << i;
    }
}

TEST_F(ASTPostOrderIteratorTest, TraversalWithParents)
{
    parse("result = 1");
    auto range = postOrderTraversal(ast);
    auto it = range.begin();

    ASSERT_NE(dynamic_cast<Identifier*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->body()->statements()[0]->children()[0]);
    it++;

    ASSERT_NE(dynamic_cast<VarRef*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->body()->statements()[0]);
    it++;

    ASSERT_NE(dynamic_cast<ByteLiteral*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->body()->statements()[0]);
    it++;

    ASSERT_NE(dynamic_cast<VarAssignment*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->body());
    it++;

    ASSERT_NE(dynamic_cast<Block*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast);
    it++;

    ASSERT_NE(dynamic_cast<Program*>(*it), nullptr);
    EXPECT_EQ(it.parent(), nullptr);
    it++;

    ASSERT_EQ(it, range.end());
}

TEST_F(ASTPostOrderIteratorTest, ReplaceNodeWhilstIterating)
{
    parse("result = 1");
    auto range = postOrderTraversal(ast);
    auto it = range.begin();

    ASSERT_NE(dynamic_cast<Identifier*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->body()->statements()[0]->children()[0]);
    it++;

    ASSERT_NE(dynamic_cast<VarRef*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->body()->statements()[0]);

    // Replace the VarRef with a different VarRef. At this point, we've already traversed into the children, so this
    // shouldn't change anything except what the iterator points to.
    Node* previousNode = *it;
    it.replaceNode(new VarRef(ast, (*it)->location(), new Identifier(ast, (*it)->children()[0]->location(), "someFloat", Annotation::FLOAT)));
    ASSERT_NE(dynamic_cast<VarRef*>(*it), nullptr);
    EXPECT_NE(*it, previousNode);
    EXPECT_EQ(it.parent(), ast->body()->statements()[0]);
    it++;

    ASSERT_NE(dynamic_cast<ByteLiteral*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->body()->statements()[0]);
    it++;

    ASSERT_NE(dynamic_cast<VarAssignment*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->body());
    it++;

    ASSERT_NE(dynamic_cast<Block*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast);
    it++;

    ASSERT_NE(dynamic_cast<Program*>(*it), nullptr);
    EXPECT_EQ(it.parent(), nullptr);
    it++;

    ASSERT_EQ(it, range.end());
}

TEST_F(ASTPostOrderIteratorTest, ModifyChildrenWhilstIterating)
{
    parse("result = 1");
    auto range = postOrderTraversal(ast);
    auto it = range.begin();

    ASSERT_NE(dynamic_cast<Identifier*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->body()->statements()[0]->children()[0]);
    it++;

    ASSERT_NE(dynamic_cast<VarRef*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->body()->statements()[0]);

    // Perform the same modification as in the previous test, but instead modify the child of VarRef directly. This
    // should not affect iteration.
    Node* childToReplace = dynamic_cast<VarRef*>(*it)->identifier();
    (*it)->swapChild(childToReplace, new Identifier(ast, childToReplace->location(), "someFloat", Annotation::FLOAT));

    it++;

    ASSERT_NE(dynamic_cast<ByteLiteral*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->body()->statements()[0]);
    it++;

    ASSERT_NE(dynamic_cast<VarAssignment*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast->body());
    it++;

    ASSERT_NE(dynamic_cast<Block*>(*it), nullptr);
    EXPECT_EQ(it.parent(), ast);
    it++;

    ASSERT_NE(dynamic_cast<Program*>(*it), nullptr);
    EXPECT_EQ(it.parent(), nullptr);
    it++;

    ASSERT_EQ(it, range.end());
}