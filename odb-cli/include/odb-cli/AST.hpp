#pragma once

#include "odb-cli/Actions.argdef.hpp"

extern "C" {
#include "odb-compiler/parser/db_source.h"
}

struct ast;

int initAST(void);
void deinitAST(void);

bool parseDBA(const std::vector<std::string>& args);
bool dumpASTDOT(const std::vector<std::string>& args);
bool dumpASTJSON(const std::vector<std::string>& args);

struct ast* getAST();
const char* getSourceFilepath();
const char* getSource();

ActionHandler parseDBPro(const std::vector<std::string>& args);
ActionHandler autoDetectInput(const std::vector<std::string>& args);

