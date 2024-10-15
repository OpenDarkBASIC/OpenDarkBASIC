#pragma once

#include "odb-cli/Actions.argdef.hpp"

extern "C" {
#include "odb-compiler/parser/db_source.h"
}

struct ast;

int initAST(void);
void deinitAST(void);

bool parse_dba(const std::vector<std::string>& args);
bool run_semantic_checks(const std::vector<std::string>& args);
bool dump_ast_pre_semantic(const std::vector<std::string>& args);
bool dump_ast_post_semantic(const std::vector<std::string>& args);

struct ast* getAST();
const char* getSourceFilepath();
const char* getSource();

ActionHandler parseDBPro(const std::vector<std::string>& args);
ActionHandler autoDetectInput(const std::vector<std::string>& args);

