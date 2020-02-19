#include "odbc/parsers/db/Driver.hpp"
#include "odbc/ast/Node.hpp"
#include <stdio.h>
#include <fstream>

int main(int argc, char** argv)
{
    if (argc < 1)
    {
        printf("Usage: %s <db source file>\n", argv[0]);
        return 1;
    }

    FILE* fp = fopen(argv[1], "r");
    if (fp == nullptr)
    {
        printf("Failed to open file %s\n", argv[1]);
        return 1;
    }

    odbc::db::Driver driver;
    driver.parseStream(fp);

    std::ofstream os("out.dot");
    odbc::ast::dumpToDOT(os, driver.getAST());

    fclose(fp);

    return 0;
}
