#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/keywords/KeywordDB.hpp"
#include "odb-compiler/keywords/KeywordMatcher.hpp"
#include "odb-compiler/ast/Node.hpp"
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
    bool setSDKRootDir(const std::vector<std::string>& args);
    bool printSDKRootDir(const std::vector<std::string>& args);
    bool parseDBA(const std::vector<std::string>& args);
    bool dumpASTDOT(const std::vector<std::string>& args);
    bool dumpASTJSON(const std::vector<std::string>& args);
    bool dumpkWJSON(const std::vector<std::string>& args);
    bool dumpkWINI(const std::vector<std::string>& args);
    bool dumpkWNames(const std::vector<std::string>& args);

private:
    odb::KeywordDB keywordDB_;
    odb::KeywordMatcher keywordMatcher_;
    odb::ast::Node* ast_ = nullptr;
    std::string programName_;
    std::string sdkRootDir_;
    bool sdkRootDirChanged_ = true;
    bool printBanner_ = true;
};
