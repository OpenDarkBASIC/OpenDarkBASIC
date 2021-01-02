#pragma once

#include <string>
#include <vector>

bool initCommandMatcher(const std::vector<std::string>& args);
bool parseDBA(const std::vector<std::string>& args);
bool dumpASTDOT(const std::vector<std::string>& args);
bool dumpASTJSON(const std::vector<std::string>& args);
