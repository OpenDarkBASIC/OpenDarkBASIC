#include "odbc/ast/Node2.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <unordered_map>

namespace odbc {
namespace ast2 {
namespace {
struct SymbolTable {
    SymbolTable(const odbc::KeywordDB& keyword_db) : keyword_db(keyword_db) {}

    const odbc::KeywordDB& keyword_db;
    std::unordered_map<std::string, std::pair<ast::Node*, FunctionDefinition*>> functions;
    std::unordered_map<std::string, std::string> constant_map;
};

void initialiseNode(Node& node, ast::Node* src) {
    node.location = src->info.loc;
}

void initialiseExpression(Expression& expression, ast::Node* src) {
    initialiseNode(expression, src);
}

void initialiseStatement(Statement& statement, ast::Node* src,
                         FunctionDefinition* containing_function) {
    initialiseNode(statement, src);
    statement.containing_function = containing_function;
}

UnaryOp getUnaryOpFromType(ast::NodeType type) {
    switch (type) {
    case ast::NT_OP_BNOT:
        return UnaryOp::BinaryNot;
    case ast::NT_OP_LNOT:
        return UnaryOp::LogicalNot;
    default:
        assert(false && "Converting unknown node type to UnaryOp enum.");
        std::terminate();
    }
}

BinaryOp getBinaryOpFromType(ast::NodeType type) {
    switch (type) {
    case ast::NT_OP_ADD:
        return BinaryOp::Add;
    case ast::NT_OP_SUB:
        return BinaryOp::Sub;
    case ast::NT_OP_BNOT:
        return BinaryOp::Mul;
    case ast::NT_OP_LNOT:
        return BinaryOp::Div;
    case ast::NT_OP_MOD:
        return BinaryOp::Mod;
    case ast::NT_OP_POW:
        return BinaryOp::Pow;
    case ast::NT_OP_BSHL:
        return BinaryOp::LeftShift;
    case ast::NT_OP_BSHR:
        return BinaryOp::RightShift;
    case ast::NT_OP_BOR:
        return BinaryOp::BinaryOr;
    case ast::NT_OP_BAND:
        return BinaryOp::BinaryAnd;
    case ast::NT_OP_BXOR:
        return BinaryOp::BinaryXor;
    case ast::NT_OP_LT:
        return BinaryOp::LessThan;
    case ast::NT_OP_LE:
        return BinaryOp::LessThanOrEqual;
    case ast::NT_OP_GT:
        return BinaryOp::GreaterThan;
    case ast::NT_OP_GE:
        return BinaryOp::GreaterThanOrEqual;
    case ast::NT_OP_EQ:
        return BinaryOp::Equal;
    case ast::NT_OP_NE:
        return BinaryOp::NotEqual;
    case ast::NT_OP_LOR:
        return BinaryOp::LogicalOr;
    case ast::NT_OP_LAND:
        return BinaryOp::LogicalAnd;
    case ast::NT_OP_LXOR:
        return BinaryOp::LogicalXor;
    default:
        assert(false && "Converting unknown node type to BinaryOp enum.");
        std::terminate();
    }
}

Type getTypeFromSym(ast::Node* sym_node) {
    const auto& symbol_base = sym_node->sym.base;
    switch (symbol_base.flag.datatype) {
    case ast::SDT_INTEGER:
        return Type{ast::LT_INTEGER};
    case ast::SDT_FLOAT:
        return Type{ast::LT_FLOAT};
    case ast::SDT_STRING:
        return Type{ast::LT_STRING};
    case ast::SDT_BOOLEAN:
        return Type{ast::LT_BOOLEAN};
    case ast::SDT_UDT:
        std::cerr << "getTypeFromVarDecl UDT not implemented." << std::endl;
        return Type{nullptr};
    default:
        std::cerr << "getTypeFromVarDecl encountered unknown type, returning void." << std::endl;
        return Type{};
    }
}

std::vector<ast::Node*> getNodesFromBlockNode(ast::Node* block) {
    std::vector<ast::Node*> nodes;
    for (ast::Node* current_block_node = block; current_block_node != nullptr;
         current_block_node = current_block_node->block.next) {
        nodes.emplace_back(current_block_node->block.stmnt);
    }
    return nodes;
}

std::vector<ast::Node*> getNodesFromOpCommaNode(ast::Node* op_comma) {
    std::vector<ast::Node*> nodes;

    ast::Node* current_arg_node = op_comma;
    while (current_arg_node != nullptr) {
        if (current_arg_node->info.type == ast::NT_OP_COMMA) {
            nodes.emplace_back(current_arg_node->op.comma.right);
            current_arg_node = current_arg_node->op.comma.left;
        } else {
            nodes.emplace_back(current_arg_node);
            current_arg_node = nullptr;
        }
    }
    std::reverse(nodes.begin(), nodes.end());

    return nodes;
}

// Forward declarations.

Ptr<Expression> convertExpression(SymbolTable& symbol_table, ast::Node* node);
void convertBlock(SymbolTable& symbol_table, const std::vector<ast::Node*>& block,
                  FunctionDefinition* containing_function, StatementBlock& out_statements);

// Conversions.
template <typename T>
void convertExpression(SymbolTable& symbol_table, ast::Node* node, T& expression);

template <>
void convertExpression(SymbolTable& symbol_table, ast::Node* node,
                       KeywordFunctionCallExpression& call_expression) {
    call_expression.keyword = symbol_table.keyword_db.lookup(node->sym.keyword.name);
    assert(call_expression.keyword);

    // Extract arguments.
    auto arg_nodes = getNodesFromOpCommaNode(node->sym.keyword.arglist);
    PtrVector<Expression> arguments;
    for (ast::Node* expression_node : arg_nodes) {
        arguments.emplace_back(convertExpression(symbol_table, expression_node));
    }

    if (!arguments.empty()) {
        // Match argument list to overload.
        auto convertKeywordType = [](Keyword::Type type) -> Type {
            switch (type) {
                case Keyword::Type::Integer:
                    return Type{ast::LT_INTEGER};
                case Keyword::Type::Float:
                    return Type{ast::LT_FLOAT};
                case Keyword::Type::String:
                    return Type{ast::LT_STRING};
                case Keyword::Type::Double:
                    return Type{ast::LT_DOUBLE};
                case Keyword::Type::Long:
                    return Type{ast::LT_LONG};
                case Keyword::Type::Dword:
                    return Type{ast::LT_DWORD};
                case Keyword::Type::Void:
                    return Type{};
            }
            assert(false && "Unknown keyword type!");
        };

        int overload_idx = 0;
        bool found_overload = false;
        for (; overload_idx < call_expression.keyword->overloads.size(); ++overload_idx) {
            const auto &overload = call_expression.keyword->overloads[overload_idx];
            if (overload.args.size() != arguments.size()) {
                // Not this overload.
                continue;
            }

            bool matching_args = true;
            for (int i = 0; i < overload.args.size(); ++i) {
                if (overload.args[i].type == Keyword::Type{88} || overload.args[i].type == Keyword::Type{65}) {
                    continue;
                }
                if (convertKeywordType(overload.args[i].type) != arguments[i]->getType()) {
                    matching_args = false;
                    break;
                }
            }

            // If the args doesn't match, not this overload.
            if (!matching_args) {
                continue;
            }

            // We found a match.
            found_overload = true;
            break;
        }

        if (!found_overload) {
            std::cerr << "Unable to find matching overload for keyword " << node->sym.keyword.name << std::endl;
            return;
        }
        call_expression.keyword_overload = overload_idx;
    }

    call_expression.arguments = std::move(arguments);
}

template <>
void convertExpression(SymbolTable& symbol_table, ast::Node* node,
                       UserFunctionCallExpression& call_expression) {
    // Lookup function.
    auto function_name = node->sym.func_call.name;
    auto function_entry = symbol_table.functions.find(node->sym.func_call.name);
    if (function_entry == symbol_table.functions.end()) {
        // Function missing
        std::cerr << "Function " << function_name << " is not defined." << std::endl;
        assert(false);
    }
    call_expression.function = function_entry->second.second;

    auto arg_nodes = getNodesFromOpCommaNode(node->sym.func_call.arglist);
    for (ast::Node* expression_node : arg_nodes) {
        call_expression.arguments.emplace_back(convertExpression(symbol_table, expression_node));
    }
}

template <>
void convertExpression(SymbolTable& symbol_table, ast::Node* node,
                       VariableExpression& variable_expression) {
    assert(node->info.type == ast::NT_SYM_VAR_REF);
    variable_expression.name = node->sym.var_ref.name;
    variable_expression.type = getTypeFromSym(node);
}

template <typename T>
Ptr<T> convertExpression(SymbolTable& symbol_table, ast::Node* node) {
    auto expression_ptr = std::make_unique<T>();
    initialiseExpression(*expression_ptr, node);
    convertExpression(symbol_table, node, *expression_ptr);
    return expression_ptr;
}

Ptr<Expression> convertExpression(SymbolTable& symbol_table, ast::Node* node) {
    if (!node) {
        return nullptr;
    }

    switch (node->info.type) {
    case ast::NT_OP_BNOT:
    case ast::NT_OP_LNOT: {
        auto unary_expression = std::make_unique<UnaryExpression>();
        initialiseExpression(*unary_expression, node);
        unary_expression->op = getUnaryOpFromType(node->info.type);
        unary_expression->expr = convertExpression(symbol_table, node->op.base.left);
        return unary_expression;
    }
    case ast::NT_OP_ADD:
    case ast::NT_OP_INC:
    case ast::NT_OP_SUB:
    case ast::NT_OP_DEC:
    case ast::NT_OP_MUL:
    case ast::NT_OP_DIV:
    case ast::NT_OP_MOD:
    case ast::NT_OP_POW:
    case ast::NT_OP_BSHL:
    case ast::NT_OP_BSHR:
    case ast::NT_OP_BOR:
    case ast::NT_OP_BAND:
    case ast::NT_OP_BXOR:
    case ast::NT_OP_LT:
    case ast::NT_OP_LE:
    case ast::NT_OP_GT:
    case ast::NT_OP_GE:
    case ast::NT_OP_EQ:
    case ast::NT_OP_NE:
    case ast::NT_OP_LOR:
    case ast::NT_OP_LAND:
    case ast::NT_OP_LXOR: {
        auto binary_expression = std::make_unique<BinaryExpression>();
        initialiseExpression(*binary_expression, node);
        binary_expression->op = getBinaryOpFromType(node->info.type);
        binary_expression->left = convertExpression(symbol_table, node->op.base.left);
        binary_expression->right = convertExpression(symbol_table, node->op.base.right);
        return binary_expression;
    }
    case ast::NT_SYM_VAR_REF: {
        auto var_ref_expression = std::make_unique<VariableExpression>();
        initialiseExpression(*var_ref_expression, node);
        convertExpression(symbol_table, node, *var_ref_expression);
        return var_ref_expression;
    }
    case ast::NT_LITERAL: {
        auto literal_expression = std::make_unique<LiteralExpression>();
        initialiseExpression(*literal_expression, node);
        literal_expression->type = node->literal.type;
        switch (node->literal.type) {
        case ast::LT_BOOLEAN:
            literal_expression->value.b = node->literal.value.b;
            break;
        case ast::LT_INTEGER:
            literal_expression->value.i = node->literal.value.i;
            break;
        case ast::LT_FLOAT:
            literal_expression->value.f = node->literal.value.f;
            break;
        case ast::LT_STRING:
            literal_expression->value.s = node->literal.value.s;
            break;
        }
        return literal_expression;
    }
    case ast::NT_SYM_KEYWORD:
        return convertExpression<KeywordFunctionCallExpression>(symbol_table, node);
    case ast::NT_SYM_FUNC_CALL:
        return convertExpression<UserFunctionCallExpression>(symbol_table, node);
    default:
        std::cerr << "Unknown expression type " << node->info.type << std::endl;
        assert(false);
    }
}  // namespace

Ptr<Statement> convertStatement(SymbolTable& symbol_table, ast::Node* node,
                                FunctionDefinition* containing_function) {
    // Should be anything _except_ a function definition.
    switch (node->info.type) {
    case ast::NT_BLOCK:
        assert(false);

    // var = expr
    case ast::NT_ASSIGNMENT: {
        auto assignment_statement = std::make_unique<AssignmentStatement>();
        initialiseStatement(*assignment_statement, node, containing_function);
        assert(node->assignment.symbol->info.type == ast::NT_SYM_VAR_REF);
        assignment_statement->variable.location = node->assignment.symbol->info.loc;
        assignment_statement->variable.name = node->assignment.symbol->sym.var_ref.name;
        assignment_statement->variable.type = getTypeFromSym(node->assignment.symbol);
        assignment_statement->expression = convertExpression(symbol_table, node->assignment.expr);
        return assignment_statement;
    }

    // if expr : branch_true : else : branch_false : endif
    case ast::NT_BRANCH: {
        auto branch_statement = std::make_unique<BranchStatement>();
        initialiseStatement(*branch_statement, node, containing_function);
        branch_statement->expression = convertExpression(symbol_table, node->branch.condition);
        if (node->branch.paths) {
            assert(node->branch.paths->info.type == ast::NT_BRANCH_PATHS);
            const auto& branch_paths = node->branch.paths->branch_paths;
            if (branch_paths.is_true) {
                convertBlock(symbol_table, getNodesFromBlockNode(branch_paths.is_true),
                             containing_function, branch_statement->true_branch);
            }
            if (branch_paths.is_false) {
                convertBlock(symbol_table, getNodesFromBlockNode(branch_paths.is_false),
                             containing_function, branch_statement->true_branch);
            }
        }
        return branch_statement;
    }

    case ast::NT_BRANCH_PATHS:
        // Handled as part of ast::NT_BRANCH.
        assert(false);
        break;

    case ast::NT_SELECT: {
        auto select_statement = std::make_unique<SelectStatement>();
        initialiseStatement(*select_statement, node, containing_function);
        select_statement->expression = convertExpression(symbol_table, node->select.expr);

        for (ast::Node* case_list = node->select.cases; case_list != nullptr;
             case_list = case_list->case_list.next) {
            assert(case_list->info.type == ast::NT_CASE_LIST);
            ast::Node* case_node = case_list->case_list.case_;
            assert(case_node->info.type == ast::NT_CASE);

            SelectStatement::Case case_;
            case_.condition = convertExpression(symbol_table, case_node->case_.condition);
            convertBlock(symbol_table, getNodesFromBlockNode(case_node->case_.body),
                         containing_function, case_.statements);
            select_statement->cases.emplace_back(std::move(case_));
        }

        return select_statement;
    }

    case ast::NT_CASE_LIST:
    case ast::NT_CASE:
        // Handled as part of ast::NT_SELECT.
        assert(false);
        break;

    // endfunction
    case ast::NT_FUNC_RETURN: {
        auto endfunction_statement = std::make_unique<EndfunctionStatement>();
        initialiseStatement(*endfunction_statement, node, containing_function);
        endfunction_statement->expression =
            convertExpression(symbol_table, node->func_return.retval);
        return endfunction_statement;
    }

    case ast::NT_SUB_RETURN: {
        auto return_statement = std::make_unique<ReturnStatement>();
        initialiseStatement(*return_statement, node, containing_function);
        return return_statement;
    }

    case ast::NT_GOTO: {
        assert(node->goto_.label->info.type == ast::NT_SYM);
        auto goto_statement = std::make_unique<GotoStatement>();
        initialiseStatement(*goto_statement, node, containing_function);
        goto_statement->label = node->goto_.label->sym.base.name;
        return goto_statement;
    }

    case ast::NT_LOOP_WHILE: {
        auto while_loop_statement = std::make_unique<WhileStatement>();
        initialiseStatement(*while_loop_statement, node, containing_function);
        assert(node->loop.body->info.type == ast::NT_BLOCK);
        convertBlock(symbol_table, getNodesFromBlockNode(node->loop.body), containing_function,
                     while_loop_statement->block);
        return while_loop_statement;
    }

    case ast::NT_LOOP_UNTIL: {
        auto repeat_until_loop_statement = std::make_unique<RepeatUntilStatement>();
        initialiseStatement(*repeat_until_loop_statement, node, containing_function);
        assert(node->loop.body->info.type == ast::NT_BLOCK);
        convertBlock(symbol_table, getNodesFromBlockNode(node->loop.body), containing_function,
                     repeat_until_loop_statement->block);
        return repeat_until_loop_statement;
    }

    case ast::NT_LOOP: {
        auto do_loop_statement = std::make_unique<DoLoopStatement>();
        initialiseStatement(*do_loop_statement, node, containing_function);
        assert(node->loop.body->info.type == ast::NT_BLOCK);
        convertBlock(symbol_table, getNodesFromBlockNode(node->loop.body), containing_function,
                     do_loop_statement->block);
        return do_loop_statement;
    }

    case ast::NT_BREAK: {
        auto break_statement = std::make_unique<BreakStatement>();
        initialiseStatement(*break_statement, node, containing_function);
        return break_statement;
    }

    case ast::NT_SYM_CONST_DECL:
        symbol_table.constant_map.emplace(std::string{node->sym.const_decl.name},
                                          std::string{node->sym.const_decl.literal->sym.base.name});
        return nullptr;

    case ast::NT_SYM_CONST_REF:
        // Seems unused?
        break;

    case ast::NT_SYM_VAR_DECL:
        break;

    case ast::NT_SYM_VAR_REF:
        break;

    case ast::NT_SYM_ARRAY_DECL:
        break;

    case ast::NT_SYM_ARRAY_REF:
        break;

    case ast::NT_SYM_UDT_DECL:
        break;

    case ast::NT_UDT_SUBTYPE_LIST: {
    } break;

    case ast::NT_SYM_UDT_TYPE_REF:
        break;

    case ast::NT_OP_INC: {
        auto inc_statement = std::make_unique<IncStatement>();
        initialiseStatement(*inc_statement, node, containing_function);
        convertExpression(symbol_table, node->op.inc.left, inc_statement->variable);
        inc_statement->increment = convertExpression(symbol_table, node->op.inc.right);
        return inc_statement;
    }

    case ast::NT_OP_DEC: {
        auto dec_statement = std::make_unique<DecStatement>();
        initialiseStatement(*dec_statement, node, containing_function);
        convertExpression(symbol_table, node->op.inc.left, dec_statement->variable);
        dec_statement->decrement = convertExpression(symbol_table, node->op.inc.right);
        return dec_statement;
    }

    case ast::NT_SYM_FUNC_CALL: {
        auto call_statement = std::make_unique<UserFunctionCallStatement>();
        initialiseStatement(*call_statement, node, containing_function);
        convertExpression(symbol_table, node, call_statement->expr);
        return call_statement;
    }

    case ast::NT_SYM_FUNC_DECL:
        break;

    case ast::NT_SYM_SUB_CALL: {
        auto gosub_statement = std::make_unique<GosubStatement>();
        initialiseStatement(*gosub_statement, node, containing_function);
        gosub_statement->label = node->sym.sub_call.name;
        return gosub_statement;
    }

    case ast::NT_SYM_LABEL: {
        auto label_statement = std::make_unique<LabelStatement>();
        initialiseStatement(*label_statement, node, containing_function);
        label_statement->name = node->sym.label.name;
        return label_statement;
    }

    case ast::NT_SYM_KEYWORD: {
        auto call_statement = std::make_unique<KeywordFunctionCallStatement>();
        initialiseStatement(*call_statement, node, containing_function);
        convertExpression(symbol_table, node, call_statement->expr);
        return call_statement;
    }
    default:
        assert(false);
    }
    std::cerr << "Unknown node type " << node->info.type << std::endl;
    std::terminate();
}

Ptr<FunctionDefinition> convertFunctionWithoutBody(ast::Node* func_decl_node) {
    const auto& func_decl = func_decl_node->sym.func_decl;
    auto function = std::make_unique<FunctionDefinition>();
    initialiseNode(*function, func_decl_node);
    function->name = func_decl.name;

    // Extract arguments.
    auto arg_nodes = getNodesFromOpCommaNode(func_decl.arglist);
    for (ast::Node* var_decl_node : arg_nodes) {
        assert(var_decl_node->info.type == ast::NT_SYM_VAR_DECL);
        FunctionDefinition::Argument arg;
        arg.name = var_decl_node->sym.var_decl.name;
        arg.type = getTypeFromSym(var_decl_node);
        function->arguments.emplace_back(arg);
    }

    return function;
}

void convertBlock(SymbolTable& symbol_table, const std::vector<ast::Node*>& block,
                  FunctionDefinition* containing_function, StatementBlock& out_statements) {
    for (ast::Node* node : block) {
        out_statements.emplace_back(convertStatement(symbol_table, node, containing_function));
    }
}

void convertRootBlock(Program& program, ast::Node* block, const odbc::KeywordDB& keyword_db) {
    assert(block->info.type == ast::NT_BLOCK);

    bool reached_end_of_main = false;

    // The root block consists of two sections: the main function, and other functions.

    // First pass: Extract main function statements, and function nodes.
    std::vector<ast::Node*> main_statements;
    SymbolTable symbol_table(keyword_db);
    for (ast::Node* s : getNodesFromBlockNode(block)) {
        // If we've reached the end of the main function, we should only processing functions.
        if (reached_end_of_main) {
            if (s->info.type != ast::NT_SYM_FUNC_DECL) {
                std::cerr << "We've reached the end of main, but encountered a node that isn't a "
                             "function.";
                return;
            }
            symbol_table.functions.emplace(s->sym.func_decl.name,
                                           std::pair<ast::Node*, FunctionDefinition*>{s, nullptr});
        } else {
            if (s->info.type == ast::NT_SYM_FUNC_DECL) {
                // We've reached the end of main now.
                reached_end_of_main = true;
                symbol_table.functions.emplace(
                    s->sym.func_decl.name, std::pair<ast::Node*, FunctionDefinition*>{s, nullptr});
            } else {
                // Append to main block.
                main_statements.emplace_back(s);
            }
        }
    }

    // Second pass: Populate symbol table with empty definitions.
    for (auto& function_entry : symbol_table.functions) {
        auto function_def = convertFunctionWithoutBody(function_entry.second.first);
        function_entry.second.second = function_def.get();
        program.functions.emplace_back(std::move(function_def));
    }

    // Third pass: Generate statements.
    convertBlock(symbol_table, main_statements, nullptr, program.main_function);
    for (auto& function_entry : symbol_table.functions) {
        convertBlock(symbol_table,
                     getNodesFromBlockNode(function_entry.second.first->sym.func_decl.body),
                     function_entry.second.second, function_entry.second.second->statements);
    }
}
}  // namespace

Type::Type() : is_void(true), is_udt(false), void_tag() {
}

Type::Type(UDTDefinition* udt) : is_void(false), is_udt(true), udt(udt) {
}

Type::Type(ast::LiteralType builtin) : is_void(false), is_udt(false), builtin(builtin) {
}

bool Type::operator==(const Type& other) const {
    // Are the two types different?
    if (is_void != other.is_void || is_udt != other.is_udt) {
        return false;
    }
    if (is_void) {
        return true;
    } else if (is_udt) {
        return udt->name == other.udt->name;
    } else {
        return builtin == other.builtin;
    }
}

bool Type::operator!=(const Type& other) const {
    return !(*this == other);
}

Type UnaryExpression::getType() const {
    return expr->getType();
}

Type BinaryExpression::getType() const {
    switch (op) {
    case BinaryOp::Add:
    case BinaryOp::Sub:
    case BinaryOp::Mul:
    case BinaryOp::Div:
    case BinaryOp::Mod:
    case BinaryOp::Pow:
    case BinaryOp::LeftShift:
    case BinaryOp::RightShift:
    case BinaryOp::BinaryAnd:
    case BinaryOp::BinaryOr:
    case BinaryOp::BinaryXor:
        return left->getType();
    case BinaryOp::LessThan:
    case BinaryOp::LessThanOrEqual:
    case BinaryOp::GreaterThan:
    case BinaryOp::GreaterThanOrEqual:
    case BinaryOp::Equal:
    case BinaryOp::NotEqual:
    case BinaryOp::LogicalOr:
    case BinaryOp::LogicalAnd:
    case BinaryOp::LogicalXor: {
        return Type{ast::LT_BOOLEAN};
    }
    }
}

Type VariableExpression::getType() const {
    return type;
}

Type LiteralExpression::getType() const {
    return type;
}

Type KeywordFunctionCallExpression::getType() const {
    return return_type;
}

Type UserFunctionCallExpression::getType() const {
    return return_type;
}

Program Program::fromAst(ast::Node* root, const odbc::KeywordDB& keyword_db) {
    Program program;
    convertRootBlock(program, root, keyword_db);
    return program;
}
}  // namespace ast2
}  // namespace odbc