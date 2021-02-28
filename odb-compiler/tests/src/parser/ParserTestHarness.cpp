#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/Exporters.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ASTParentConsistenciesChecker.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include <cstdio>
#include <filesystem>

void ParserTestHarness::checkParentConnectionConsistencies(const odb::ast::Block* ast)
{
    ASTParentConsistenciesChecker checker;
    ast->accept(&checker);
}

void ParserTestHarness::SetUp()
{
    ast = nullptr;
    driver = new odb::db::StringParserDriver;
}

void ParserTestHarness::TearDown()
{
    if (ast)
        checkParentConnectionConsistencies(ast);

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
    }

    delete driver;
}
