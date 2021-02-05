#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/parsers/db/Scanner.hpp"
#include "odb-compiler/parsers/db/Parser.y.hpp"
#include "odb-sdk/Log.hpp"
#include <string>
#include <vector>
#include <cstdarg>

namespace odb {
namespace ast {
    class SourceLocation;
}
namespace db {

/*!
 * @brief This is called directly by the yyerror() callback of BISON and is
 * normally not called unless the parser really shits itself (i.e. out of memory
 * error, or something just as catastrophic).
 */
ODBCOMPILER_PRIVATE_API
void vprintParserMessage(Log::Severity severity,
                         const DBLTYPE *locp,
                         dbscan_t scanner,
                         const char* fmt,
                         va_list args);

/*!
 * @brief This is the standard callback handler when a syntax error occurs in
 * the BISON parser.
 */
ODBCOMPILER_PRIVATE_API
void printSyntaxMessage(Log::Severity severity,
                        const DBLTYPE* loc,
                        dbscan_t scanner,
                        std::pair<dbtokentype, std::string> unexpectedToken,
                        const std::vector<std::pair<dbtokentype, std::string>>& expectedTokens);

ODBCOMPILER_PRIVATE_API
void printUnderlinedSection(const DBLTYPE* loc, dbscan_t scanner);

}
}
