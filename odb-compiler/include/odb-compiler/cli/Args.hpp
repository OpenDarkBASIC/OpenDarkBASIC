#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/commands/CommandIndex.hpp"
#include "odb-compiler/commands/CommandMatcher.hpp"
#include "odb-compiler/commands/SDKType.hpp"
#include "odb-compiler/ast/Block.hpp"
#include <vector>
#include <string>
#include <memory>

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
    bool loadCommands(const std::vector<std::string>& args);
    bool parseDBA(const std::vector<std::string>& args);
    bool dumpASTDOT(const std::vector<std::string>& args);
    bool dumpASTJSON(const std::vector<std::string>& args);
    bool dumpkWJSON(const std::vector<std::string>& args);
    bool dumpkWINI(const std::vector<std::string>& args);
    bool dumpkWNames(const std::vector<std::string>& args);

private:
    bool printBanner_ = true;

    std::string programName_;
    std::string sdkRootDir_;
    std::vector<std::string> pluginDirs_;
    odb::SDKType sdkType_ = odb::SDKType::ODB;

    odb::cmd::CommandIndex cmdIndex_;
    odb::cmd::CommandMatcher cmdMatcher_;
    odb::Reference<odb::ast::Block> ast_;
};
