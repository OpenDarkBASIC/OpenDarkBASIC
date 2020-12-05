#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/keywords/KeywordIndex.hpp"
#include "odb-compiler/keywords/KeywordMatcher.hpp"
#include "odb-compiler/keywords/SDKType.hpp"
#include "odb-compiler/ast/Node.hpp"
#include <vector>
#include <string>
#include <memory>

namespace odb {
    class Plugin;
}

class Args
{
public:
    typedef bool (Args::*HandlerFunc)(const std::vector<std::string>& args);

    bool parse(int argc, char** argv);
    int parseFullOption(int argc, char** argv);
    int parseShortOptiones(int argc, char** argv);

    // Global commands
    bool disableBanner(const std::vector<std::string>& args);

    // Sequential commands
    bool printHelp(const std::vector<std::string>& args);
    bool setSDKRootDir(const std::vector<std::string>& args);
    bool setSDKType(const std::vector<std::string>& args);
    bool setAdditionalPluginsDir(const std::vector<std::string>& args);
    bool printSDKRootDir(const std::vector<std::string>& args);
    bool parseDBA(const std::vector<std::string>& args);
    bool dumpASTDOT(const std::vector<std::string>& args);
    bool dumpASTJSON(const std::vector<std::string>& args);
    bool dumpkWJSON(const std::vector<std::string>& args);
    bool dumpkWINI(const std::vector<std::string>& args);
    bool dumpkWNames(const std::vector<std::string>& args);

private:
    bool loadPluginsFromDirOrFile(const std::string& dir);

private:
    bool printBanner_ = true;
    bool kwIndexDirty_ = true;
    bool kwMatcherDirty_ = true;

    std::string programName_;
    std::string sdkRootDir_;
    odb::SDKType sdkType_ = odb::SDKType::ODB;
    std::vector<std::unique_ptr<odb::Plugin>> plugins_;

    odb::KeywordIndex kwIndex_;
    odb::KeywordMatcher kwMatcher_;
    odb::ast::Block* ast_ = nullptr;
};
