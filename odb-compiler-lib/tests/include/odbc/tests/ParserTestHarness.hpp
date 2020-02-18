#pragma once

#include "odbc/Driver.hpp"
#include <gmock/gmock.h>
#include <fstream>
#include <filesystem>

class ParserTestHarness : public testing::Test
{
public:
    void SetUp() override { driver = new odbc::Driver; }
    void TearDown() override
    {
        if (driver->getAST())
        {
            const testing::TestInfo* info = testing::UnitTest::GetInstance()->current_test_info();
            std::string filename = std::string("ast/") + info->test_suite_name()
                    + "__" + info->name() + ".dot";
            std::filesystem::create_directory("ast");
            std::ofstream out(filename);
            odbc::ast::dumpToDOT(out, driver->getAST());
        }

        delete driver;
    }
    odbc::Driver* driver;
};
