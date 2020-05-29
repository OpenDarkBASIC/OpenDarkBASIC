#pragma once

#include "odbc/config.hpp"
#include "odbc/parsers/keywords/KeywordDB.hpp"

namespace odbc {

ODBC_PUBLIC_API bool addKeywordDBFromDLL(KeywordDB& keywordDB, const std::string& dllPath);

}
