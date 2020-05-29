#pragma once

#include "odbc/config.hpp"
#include "odbc/parsers/keywords/KeywordDB.hpp"
#include "odbc/parsers/keywords/KeywordMatcher.hpp"
#include "odbc/ast/Node.hpp"
#include <vector>
#include <string>

class Args
{
public:
    typedef bool (Args::*HandlerFunc)(const std::vector<std::string>& args);

    bool parse(int argc, char** argv);

    int parseFullOption(int argc, char** argv);
    int parseShortOptiones(int argc, char** argv);

    bool disableBanner(const std::vector<std::string>& args);

    bool printHelp(const std::vector<std::string>& args);
    bool loadKeywordsINI(const std::vector<std::string>& args);
    bool loadKeywordsJSON(const std::vector<std::string>& args);
    bool plugins(const std::vector<std::string>& visitNode);
    bool parseDBA(const std::vector<std::string>& args);
    bool dumpASTDOT(const std::vector<std::string>& args);
    bool dumpASTJSON(const std::vector<std::string>& args);
    bool dumpkWJSON(const std::vector<std::string>& args);
    bool dumpkWINI(const std::vector<std::string>& args);
    bool dumpkWNames(const std::vector<std::string>& args);
    bool emitLLVM(const std::vector<std::string>& args);

private:
    odbc::KeywordDB keywordDB_;
    odbc::KeywordMatcher keywordMatcher_;
    odbc::ast::Node* ast_ = nullptr;
    std::string programName_;
    bool keywordMatcherDirty_ = true;
    bool printBanner_ = true;
};
