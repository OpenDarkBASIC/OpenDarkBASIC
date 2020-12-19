#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/parsers/db/ErrorPrinter.hpp"
#include "odb-sdk/Reference.hpp"
#include <cstdio>
#include <sstream>

namespace odb {
namespace db {

// ----------------------------------------------------------------------------
void vprintParserMessage(log::Severity severity,
                         const DBLTYPE *locp,
                         dbscan_t scanner,
                         const char* fmt,
                         va_list args)
{
    odb::db::Driver* driver = static_cast<odb::db::Driver*>(dbget_extra(scanner));
    odb::Reference<odb::ast::SourceLocation> location = driver->newLocation(locp);
    std::string fileLocInfo = location->getFileLineColumn();
    std::string fmtMsg;

    va_list copy;
    va_copy(copy, args);
    fmtMsg.resize(vsnprintf(nullptr, 0, fmt, copy) + 1);
    va_end(copy);
    va_copy(copy, args);
    snprintf( fmtMsg.data(), fmtMsg.size(), fmt, args);
    va_end(args);

    std::string msg = fileLocInfo + ": " + fmtMsg;
    odb::log::dbParser(severity, "%s", msg.c_str());
    for (const auto& line : location->getSectionHighlight())
        odb::log::info("%s\n", line.c_str());

}

// ----------------------------------------------------------------------------
void printSyntaxMessage(log::Severity severity,
                        const DBLTYPE* loc,
                        dbscan_t scanner,
                        std::pair<dbtokentype, std::string> unexpectedToken,
                        const std::vector<std::pair<dbtokentype, std::string>> expectedTokens)
{
    std::stringstream ss;
    odb::db::Driver* driver = static_cast<odb::db::Driver*>(dbget_extra(scanner));
    Reference<ast::SourceLocation> location = driver->newLocation(loc);

    ss << location->getFileLineColumn() << (severity >= log::ERROR ? ": syntax error" : ": syntax warning");
    if (unexpectedToken.first != TOK_DBEMPTY)
        ss << ", unexpected " << unexpectedToken.second;
    if (expectedTokens.size() > 0)
    {
        ss << ", expected ";
        for (int i = 0; i != (int)expectedTokens.size(); ++i)
            ss << (i != 0 ? " or " : "") << expectedTokens[i].second;
    }
    log::dbParser(severity, "%s\n", ss.str().c_str());

    printLocationHighlight(location);
}

// ----------------------------------------------------------------------------
void printLocationHighlight(const DBLTYPE* loc, dbscan_t scanner)
{
    odb::db::Driver* driver = static_cast<odb::db::Driver*>(dbget_extra(scanner));
    Reference<ast::SourceLocation> location = driver->newLocation(loc);
    printLocationHighlight(location);
}

void printLocationHighlight(const ast::SourceLocation* location)
{
    int gutterWidth = std::to_string(location->getLastLine()).length();
    auto sourceHighlightLines = location->getSectionHighlight();
    for (int i = 0; i != (int)sourceHighlightLines.size(); ++i)
    {
        // Lines are return in groups of 2. The first line is a line from the
        // affected source code. The second line is the error highlight
        // (squiggly lines). We only want to associate a line number with each
        // source code line
        if (i%2 == 0)
            log::info("%*d | ", gutterWidth, location->getFirstLine() + i/2);
        else
            log::info("%*s | ", gutterWidth, "");
        log::info("%s\n", sourceHighlightLines[i].c_str());
    }
}

}
}
