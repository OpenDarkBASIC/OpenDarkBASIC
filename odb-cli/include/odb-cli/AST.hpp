#pragma once

#include "odb-cli/Actions.argdef.hpp"

namespace odb::ast {
class Program;
}

bool initCommandMatcher(const std::vector<std::string>& args);
bool parseDBA(const std::vector<std::string>& args);
bool dumpASTDOT(const std::vector<std::string>& args);
bool dumpASTJSON(const std::vector<std::string>& args);

ActionHandler parseDBPro(const std::vector<std::string>& args);
ActionHandler autoDetectInput(const std::vector<std::string>& args);

odb::ast::Program* getAST();
