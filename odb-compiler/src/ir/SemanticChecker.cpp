#include "odb-compiler/ir/SemanticChecker.hpp"
#include "semantic/ASTConverter.hpp"

namespace odb::ir {
Ptr<Program> runSemanticChecks(const ast::Block* ast, const cmd::CommandIndex& cmdIndex) {
    return ASTConverter(cmdIndex).generateProgram(ast);
}
}
