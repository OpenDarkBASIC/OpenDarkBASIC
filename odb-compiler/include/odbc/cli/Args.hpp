#pragma once

#include "odbc/config.hpp"
#include "odbc/parsers/keywords/KeywordDB.hpp"
#include "odbc/parsers/keywords/KeywordMatcher.hpp"
#include "odbc/ast/Node.hpp"
#include <vector>
#include <string>

class Args
{
    typedef bool (Args::*HandlerFunc)(int, char**);
public:
    bool parse(int argc, char** argv);
    bool printHelp(int argc, char** argv);

    bool expectOption(int argc, char** argv);
    bool expectOptionOrNothing(int argc, char** argv);
    bool parseFullOption(int argc, char** argv);
    bool parseShortOptiones(int argc, char** argv);

    bool disableBanner(int argc, char** argv);
    bool loadKeywordFile(int argc, char** argv);
    bool loadKeywordDir(int argc, char** argv);
    bool parseDBA(int argc, char** argv);
    bool dumpASTDOT(int argc, char** argv);
    bool dumpkWStdOut(int argc, char** argv);

private:
    odbc::KeywordDB keywordDB_;
    odbc::KeywordMatcher keywordMatcher_;
    odbc::ast::Node* ast_ = nullptr;
    bool keywordMatcherDirty_ = true;
    bool printBanner_ = true;
};
