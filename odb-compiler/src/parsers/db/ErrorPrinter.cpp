#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/SelectCase.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/parsers/db/ErrorPrinter.hpp"
#include "odb-sdk/Reference.hpp"
#include <cstdio>
#include <sstream>

namespace odb::db {

// ----------------------------------------------------------------------------
void printSyntaxMessage(Log::Severity severity,
                        const DBLTYPE* loc,
                        dbscan_t scanner,
                        std::pair<dbtokentype, std::string> unexpectedToken,
                        const std::vector<std::pair<dbtokentype, std::string>>& expectedTokens)
{
    ColorState state(Log::info, Log::FG_WHITE);

    odb::db::Driver* driver = static_cast<odb::db::Driver*>(dbget_extra(scanner));
    Reference<ast::SourceLocation> location = driver->newLocation(loc);

    Log::info.print((severity >= Log::ERROR ? Log::FG_BRIGHT_RED : Log::FG_BRIGHT_YELLOW), "[db parser] ");
    Log::info.print(Log::FG_BRIGHT_WHITE, "%s: ", location->getFileLineColumn().c_str());
    Log::info.print((severity >= Log::ERROR ? Log::FG_BRIGHT_RED : Log::FG_BRIGHT_YELLOW), (severity >= Log::ERROR ? "syntax error: " : "syntax warning: "));

    if (unexpectedToken.first != TOK_DBEMPTY)
    {
        Log::info.print("unexpected ");
        Log::info.print(Log::FG_BRIGHT_WHITE, "%s", unexpectedToken.second.c_str());
    }
    if (expectedTokens.size() > 0)
    {
        Log::info.print(", expected ");
        for (int i = 0; i != (int)expectedTokens.size(); ++i)
        {
            if (i != 0)
                Log::info.print(" or ");
            Log::info.print(Log::FG_BRIGHT_WHITE, expectedTokens[i].second.c_str());
        }
        Log::info.print("\n");
    }

    location->printUnderlinedSection(Log::info);
}

// ----------------------------------------------------------------------------
void printUnderlinedSection(const DBLTYPE* loc, dbscan_t scanner)
{
    odb::db::Driver* driver = static_cast<odb::db::Driver*>(dbget_extra(scanner));
    Reference<ast::SourceLocation> location = driver->newLocation(loc);
    location->printUnderlinedSection(Log::info);
}

}
