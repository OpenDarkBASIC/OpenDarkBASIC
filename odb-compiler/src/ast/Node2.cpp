#include "odbc/ast/Node2.hpp"
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <algorithm>

namespace odbc {
namespace ast2 {
namespace {
struct SymbolTable {
    std::unordered_map<std::string, std::pair<ast::Node*, FunctionDefinition*>> functions;
};

void initialiseNode(Node& node, ast::Node* src) {
    node.location = src->info.loc;
}

void initialiseExpression(Expression& expression, ast::Node* src) {
    initialiseNode(expression, src);
}

void initialiseStatement(Statement& statement, ast::Node* src, FunctionDefinition* containing_function) {
    initialiseNode(statement, src);
    statement.containing_function = containing_function;
}

Type getTypeFromSym(ast::Node* sym_node) {
    const auto &symbol_base = sym_node->sym.base;

    Type result{};
    result.is_udt = false;
    switch (symbol_base.flag.datatype) {
        case ast::SDT_INTEGER:
            result.builtin = ast::LT_INTEGER;
            break;
        case ast::SDT_FLOAT:
            result.builtin = ast::LT_FLOAT;
            break;
        case ast::SDT_STRING:
            result.builtin = ast::LT_STRING;
            break;
        case ast::SDT_BOOLEAN:
            result.builtin = ast::LT_BOOLEAN;
            break;
        case ast::SDT_UDT:
            result.is_udt = true;
            result.udt = nullptr;
            std::cerr << "getTypeFromVarDecl UDT not implemented." << std::endl;
            break;
        case ast::SDT_NONE:
            std::cerr << "getTypeFromVarDecl encountered SDT_NONE." << std::endl;
            break;
    }
    return result;
}

std::vector<ast::Node *> getNodesFromBlockNode(ast::Node *block) {
    std::vector<ast::Node *> nodes;
    for (ast::Node *current_block_node = block;
         current_block_node != nullptr;
         current_block_node = current_block_node->block.next) {
        nodes.emplace_back(current_block_node->block.stmnt);
    }
    return nodes;
}

std::vector<ast::Node *> getNodesFromOpCommaNode(ast::Node *op_comma) {
    std::vector<ast::Node *> nodes;

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

Ptr<Expression> convertExpression(SymbolTable& symbol_table, ast::Node *node);
void convertBlock(SymbolTable& symbol_table, const std::vector<ast::Node*>& block, FunctionDefinition *containing_function,
                  StatementBlock &out_statements);

// Conversions.
template <typename T>
void convertExpression(SymbolTable& symbol_table, ast::Node *node, T& expression);

template <>
void convertExpression(SymbolTable& symbol_table, ast::Node *node, KeywordFunctionCallExpression& call_expression) {
    call_expression.keyword = node->sym.keyword.name;

    auto arg_nodes = getNodesFromOpCommaNode(node->sym.keyword.arglist);
    for (ast::Node* expression_node : arg_nodes) {
        call_expression.arguments.emplace_back(convertExpression(symbol_table, expression_node));
    }
}

template <>
void convertExpression(SymbolTable& symbol_table, ast::Node *node, UserFunctionCallExpression& call_expression) {
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

template <typename T>
Ptr<T> convertExpression(SymbolTable& symbol_table, ast::Node* node) {
    auto expression_ptr = std::make_unique<T>();
    initialiseExpression(*expression_ptr, node);
    convertExpression(symbol_table, node, *expression_ptr);
    return expression_ptr;
}

Ptr<Expression> convertExpression(SymbolTable& symbol_table, ast::Node *node) {
    if (node == nullptr) {
        return nullptr;
    }

    switch (node->info.type) {
        case ast::NT_OP_INC:
        case ast::NT_OP_DEC: {
            // TODO other unary ops
            auto unary_expression = std::make_unique<UnaryExpression>();
            initialiseExpression(*unary_expression, node);
            unary_expression->op = node->info.type;
            unary_expression->inner = convertExpression(symbol_table, node->op.base.left);
            return unary_expression;
        }
        case ast::NT_OP_ADD:
        case ast::NT_OP_SUB:
        case ast::NT_OP_MUL:
        case ast::NT_OP_DIV:
        case ast::NT_OP_MOD:
        case ast::NT_OP_POW:
        case ast::NT_OP_EQ:
        case ast::NT_OP_NE: {
            // TODO other binary ops
            auto binary_expression = std::make_unique<BinaryExpression>();
            initialiseExpression(*binary_expression, node);
            binary_expression->op = node->info.type;
            binary_expression->left = convertExpression(symbol_table, node->op.base.left);
            binary_expression->right = convertExpression(symbol_table, node->op.base.right);
            return binary_expression;
        }
        case ast::NT_SYM_VAR_REF: {
            auto var_ref_expression = std::make_unique<VariableExpression>();
            initialiseExpression(*var_ref_expression, node);
            var_ref_expression->name = node->sym.var_ref.name;
            var_ref_expression->type = getTypeFromSym(node);
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
}

Ptr<Statement> convertStatement(SymbolTable& symbol_table, ast::Node *node, FunctionDefinition *containing_function) {
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
                    convertBlock(symbol_table, getNodesFromBlockNode(branch_paths.is_true), containing_function, branch_statement->true_branch);
                }
                if (branch_paths.is_false) {
                    convertBlock(symbol_table, getNodesFromBlockNode(branch_paths.is_false), containing_function, branch_statement->true_branch);
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

            for (ast::Node* case_list = node->select.cases; case_list != nullptr; case_list = case_list->case_list.next) {
                assert(case_list->info.type == ast::NT_CASE_LIST);
                ast::Node* case_node = case_list->case_list.case_;
                assert(case_node->info.type == ast::NT_CASE);

                SelectStatement::Case case_;
                case_.condition = convertExpression(symbol_table, case_node->case_.condition);
                convertBlock(symbol_table,getNodesFromBlockNode(case_node->case_.body), containing_function, case_.statements);
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
            endfunction_statement->expression = convertExpression(symbol_table, node->func_return.retval);
           return endfunction_statement;
        }

        case ast::NT_SUB_RETURN: {
//            os << "N" << node->info.guid << "[label=\"return\"];\n";
        } break;

        case ast::NT_GOTO: {
//            os << "N" << node->info.guid << "[label=\"goto\"];\n";
//            os << "N" << node->info.guid << " -> " << "N" << node->goto_.label->info.guid << "[label=\"label\"];\n";
//            dumpToDOTRecursive(os, node->goto_.label);
        } break;

        case ast::NT_LOOP: {
            auto doloop_statement = std::make_unique<DoLoopStatement>();
            initialiseStatement(*doloop_statement, node, containing_function);
            assert(node->loop.body->info.type == ast::NT_BLOCK);
            convertBlock(symbol_table,getNodesFromBlockNode(node->loop.body), containing_function, doloop_statement->block);
            return doloop_statement;
        }

        case ast::NT_LOOP_WHILE: {
        } break;

        case ast::NT_LOOP_UNTIL: {
        } break;

        case ast::NT_BREAK: {
        } break;

        case ast::NT_UDT_SUBTYPE_LIST: {
        } break;
        case ast::NT_SYM_CONST_DECL:
            break;
        case ast::NT_SYM_CONST_REF:
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
        case ast::NT_SYM_UDT_TYPE_REF:
            break;
        case ast::NT_SYM_FUNC_CALL: {
            auto call_statement = std::make_unique<UserFunctionCallStatement>();
            initialiseStatement(*call_statement, node, containing_function);
            convertExpression(symbol_table, node, call_statement->expr);
            return call_statement;
        }
        case ast::NT_SYM_FUNC_DECL:
            break;
        case ast::NT_SYM_SUB_CALL:
            break;
        case ast::NT_SYM_LABEL:
            break;
        case ast::NT_SYM_KEYWORD: {
            auto call_statement = std::make_unique<KeywordFunctionCallStatement>();
            initialiseStatement(*call_statement, node, containing_function);
            convertExpression(symbol_table, node, call_statement->expr);
            return call_statement;
        }
        default:
            return nullptr;
    }
    return nullptr;
}

Ptr<FunctionDefinition> convertFunctionWithoutBody(ast::Node *func_decl_node) {
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

void convertBlock(SymbolTable& symbol_table,const std::vector<ast::Node*>& block, FunctionDefinition *containing_function,
                  StatementBlock &out_statements) {
    for (ast::Node* node : block) {
        out_statements.emplace_back(convertStatement(symbol_table, node, containing_function));
    }
}

void convertRootBlock(Program &program, ast::Node *block) {
    assert(block->info.type == ast::NT_BLOCK);

    bool reached_end_of_main = false;

    // The root block consists of two sections: the main function, and other functions.

    // First pass: Extract main function statements, and function nodes.
    std::vector<ast::Node*> main_statements;
    SymbolTable symbol_table;
    for (ast::Node *s : getNodesFromBlockNode(block)) {
        // If we've reached the end of the main function, we should only processing functions.
        if (reached_end_of_main) {
            if (s->info.type != ast::NT_SYM_FUNC_DECL) {
                std::cerr << "We've reached the end of main, but encountered a node that isn't a function.";
                return;
            }
            symbol_table.functions.emplace(s->sym.func_decl.name, std::pair<ast::Node*, FunctionDefinition*>{s, nullptr});
        } else {
            if (s->info.type == ast::NT_SYM_FUNC_DECL) {
                // We've reached the end of main now.
                reached_end_of_main = true;
                symbol_table.functions.emplace(s->sym.func_decl.name, std::pair<ast::Node*, FunctionDefinition*>{s, nullptr});
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
        convertBlock(symbol_table, getNodesFromBlockNode(function_entry.second.first->sym.func_decl.body), function_entry.second.second, function_entry.second.second->statements);
    }
}
}

Program Program::fromAst(ast::Node *root) {
    Program program;
    convertRootBlock(program, root);
    return program;
}
}
}