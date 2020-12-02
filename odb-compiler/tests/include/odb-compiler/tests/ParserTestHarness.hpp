#pragma once

#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/keywords/KeywordMatcher.hpp"
#include "odb-compiler/keywords/KeywordIndex.hpp"
#include "odb-compiler/ast/Node.hpp"
#include <gmock/gmock.h>
#include <cstdio>
#include <filesystem>

class ParserTestHarness : public testing::Test
{
public:
    void SetUp() override
    {

        ast = nullptr;
        matcher.updateFromIndex(&kwIndex);
        driver = new odb::db::Driver(&ast, &matcher);
    }

    void TearDown() override
    {
        if (ast)
        {
#if defined(ODBCOMPILER_DOT_EXPORT)
            const testing::TestInfo* info = testing::UnitTest::GetInstance()->current_test_info();
            std::string filename = std::string("ast/") + info->test_suite_name()
                    + "__" + info->name() + ".dot";
            std::filesystem::create_directory("ast");
            FILE* out = fopen(filename.c_str(), "w");
            odb::ast::dumpToDOT(out, ast);
            fclose(out);
#endif
            odb::ast::freeNodeRecursive(ast);
        }

        delete driver;
    }
    odb::KeywordIndex kwIndex;
    odb::KeywordMatcher matcher;
    odb::db::Driver* driver;
    odb::ast::Node* ast;
};
