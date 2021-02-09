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
