#include "odbc/Driver.hpp"
#include "odbc/ASTNode.hpp"
#include "odbc/Parser.y.h"
#include "odbc/Scanner.lex.h"
#include <cassert>

namespace odbc {

#ifdef ODBC_DOT_EXPORT
static int nodeGUIDCounter;
#endif

// ----------------------------------------------------------------------------
Driver::Driver() :
    ast_(nullptr),
    location_({})
{
    yylex_init(&scanner_);
    parser_ = yypstate_new();
    yylex_init_extra(this, &scanner_);
}

// ----------------------------------------------------------------------------
Driver::~Driver()
{
    freeAST();
    yypstate_delete(parser_);
    yylex_destroy(scanner_);
}

// ----------------------------------------------------------------------------
bool Driver::parseString(const std::string& str)
{
    YYSTYPE pushedValue;
    int pushedChar;
    int parse_result;

    YY_BUFFER_STATE buf = yy_scan_bytes(str.data(), str.length(), scanner_);

    do
    {
        pushedChar = yylex(&pushedValue, scanner_);
        parse_result = yypush_parse(parser_, pushedChar, &pushedValue, &location_, scanner_);
    } while (parse_result == YYPUSH_MORE);

    yy_delete_buffer(buf, scanner_);

    return parse_result == 0;
}

// ----------------------------------------------------------------------------
bool Driver::parseStream(std::istream& is)
{
    return true;
}

// ----------------------------------------------------------------------------
#ifdef ODBC_DOT_EXPORT
static void dumpToDOT(std::ostream& os, ast::node_t* node)
{
    switch (node->info.type)
    {
        case ast::BLOCK: {
            os << "N" << node->info.guid << " -> N" << node->block.statement->info.guid << ";\n";
            os << "N" << node->info.guid << "[label=\"block (" << node->info.guid << ")\"];\n";
            dumpToDOT(os, node->block.statement);
            if (node->block.next)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->block.next->info.guid << ";\n";
                dumpToDOT(os, node->block.next);
            }
        } break;

        case ast::OP_ASSIGNMENT: {
            os << "N" << node->info.guid << " -> " << "N" << node->op_assignment.symbol->info.guid << ";\n";
            os << "N" << node->info.guid << " -> " << "N" << node->op_assignment.expression->info.guid << ";\n";
            os << "N" << node->info.guid << "[label=\"= (" << node->info.guid << ")\"];\n";
            dumpToDOT(os, node->op_assignment.symbol);
            dumpToDOT(os, node->op_assignment.expression);
        } break;

        case ast::SYMBOL: {
            os << "N" << node->info.guid << " [label=\"" << node->symbol.name << "\"];\n";
        } break;
        case ast::BOOLEAN_CONSTANT: {
            os << "N" << node->info.guid << " [label=\"" << (node->boolean_constant.value ? "true" : "false") << "\"];\n";
        } break;
        case ast::INTEGER_CONSTANT: {
            os << "N" << node->info.guid << " [label=\"" << node->integer_constant.value << "\"];\n";
        } break;
        case ast::FLOAT_CONSTANT: {
            os << "N" << node->info.guid << " [label=\"" << node->float_constant.value << "\"];\n";
        } break;
        case ast::STRING_CONSTANT: {
            os << "N" << node->info.guid << " [label=\"" << node->string_constant.value << "\"];\n";
        } break;
    }
}
void Driver::dumpToDOT(std::ostream& os)
{
    os << std::string("digraph name {\n");
    ::odbc::dumpToDOT(os, ast_);
    os << std::string("}");
}
#endif

// ----------------------------------------------------------------------------
static void init_info(ast::node_t* node, ast::Type type)
{
    node->boolean_constant.info.type = type;
#ifdef ODBC_DOT_EXPORT
    node->boolean_constant.info.guid = nodeGUIDCounter++;
#endif
}

// ----------------------------------------------------------------------------
ast::node_t* Driver::newAssignment(ast::node_t* symbol, ast::node_t* expression)
{
    assert(symbol->info.type == ast::SYMBOL);
    ast::node_t* ass = (ast::node_t*)malloc(sizeof *ass);
    init_info(ass, ast::OP_ASSIGNMENT);
    ass->op_assignment.symbol = symbol;
    ass->op_assignment.expression = expression;
    return ass;
}

// ----------------------------------------------------------------------------
ast::node_t* Driver::newBooleanConstant(bool value)
{
    ast::node_t* node = (ast::node_t*)malloc(sizeof *node);
    init_info(node, ast::BOOLEAN_CONSTANT);
    node->boolean_constant.value = value;
    return node;
}

// ----------------------------------------------------------------------------
ast::node_t* Driver::newIntegerConstant(int32_t value)
{
    ast::node_t* node = (ast::node_t*)malloc(sizeof *node);
    init_info(node, ast::INTEGER_CONSTANT);
    node->integer_constant.value = value;
    return node;
}

// ----------------------------------------------------------------------------
ast::node_t* Driver::newFloatConstant(double value)
{
    ast::node_t* node = (ast::node_t*)malloc(sizeof *node);
    init_info(node, ast::FLOAT_CONSTANT);
    node->float_constant.value = value;
    return node;
}

// ----------------------------------------------------------------------------
ast::node_t* Driver::newStringConstant(const char* value)
{
    ast::node_t* node = (ast::node_t*)malloc(sizeof *node);
    init_info(node, ast::STRING_CONSTANT);
    node->string_constant.value = strdup(value);
    return node;
}

// ----------------------------------------------------------------------------
ast::node_t* Driver::newStatementBlock(ast::node_t* expr)
{
    ast::node_t* node = (ast::node_t*)malloc(sizeof *node);
    init_info(node, ast::BLOCK);
    node->block.next = nullptr;
    node->block.statement = expr;
    return node;
}

// ----------------------------------------------------------------------------
ast::node_t* Driver::appendStatementToBlock(ast::node_t* block, ast::node_t* expr)
{
    assert(block->info.type == ast::BLOCK);
    ast::node_t* last = block;
    while (last->block.next)
        last = last->block.next;

    last->block.next = newStatementBlock(expr);
    return block;
}

// ----------------------------------------------------------------------------
ast::node_t* Driver::prependStatementToBlock(ast::node_t* block, ast::node_t* expr)
{
    ast::node_t* prev = newStatementBlock(expr);
    prev->block.next = block;
    return prev;
}

// ----------------------------------------------------------------------------
ast::node_t* Driver::newSymbol(const char* name, ast::node_t* expression)
{
    auto entry = symbolTable_.insert({name, nullptr});
    if (entry.second == false)
        return nullptr;

    ast::node_t* node = (ast::node_t*)malloc(sizeof *node);
    if (node == nullptr)
    {
        symbolTable_.erase(entry.first);
        return nullptr;
    }

    init_info(node, ast::SYMBOL);
    node->symbol.name = strdup(name);
    node->symbol.value = expression;
    node->symbol.function = nullptr;
    entry.first->second = node;
    return node;
}

// ----------------------------------------------------------------------------
ast::node_t* Driver::lookupSymbol(const char* name)
{
    auto it = symbolTable_.find(name);
    if (it == symbolTable_.end())
        return nullptr;
    return it->second;
}

// ----------------------------------------------------------------------------
ast::node_t* Driver::appendBlock(ast::node_t* block)
{
    assert(block->info.type == ast::BLOCK);

    if (ast_ == nullptr)
        ast_ = block;
    else
        ast_ = appendStatementToBlock(ast_, block);
    return ast_;
}

// ----------------------------------------------------------------------------
void Driver::freeAST()
{
    freeASTNodeRecursive(ast_);
    ast_ = nullptr;
}
void Driver::freeASTNode(ast::node_t* node)
{
    switch (node->info.type)
    {
        case ast::SYMBOL          : free(node->symbol.name); break;
        case ast::STRING_CONSTANT : free(node->string_constant.value); break;

        default: break;
    }

    free(node);
}
void Driver::freeASTNodeRecursive(ast::node_t* node)
{
    if (node == nullptr)
        return;

    if (isTerminal(node))
    {
        freeASTNode(node);
    }
    else
    {
        freeASTNodeRecursive(node->nonterminal.left);
        freeASTNodeRecursive(node->nonterminal.right);
        freeASTNode(node);
    }
}

}
