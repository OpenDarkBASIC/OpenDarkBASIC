#pragma once

#include "odb-cli/Actions.argdef.hpp"

namespace odb::ast {
class Block;
}

bool initCommandMatcher(const std::vector<std::string>& args);
bool parseDBA(const std::vector<std::string>& args);
bool dumpASTDOT(const std::vector<std::string>& args);
bool dumpASTJSON(const std::vector<std::string>& args);

MetaHandlerResult parseDBPro(const std::vector<std::string>& args);
MetaHandlerResult autoDetectInput(const std::vector<std::string>& args);

const odb::ast::Block* getAST();
