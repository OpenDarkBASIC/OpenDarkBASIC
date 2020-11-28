#pragma once

#include <ostream>
#include <memory>
#include "odbc/config.hpp"
#include "odbc/ir/Node.hpp"
#include "odbc/parsers/keywords/KeywordDB.hpp"

namespace odbc {
namespace ir {
ODBC_PUBLIC_API void generateLLVMIR(std::ostream& os, std::string module_name, Program& program, const KeywordDB& keywordDb);
ODBC_PUBLIC_API void generateLLVMBC(std::ostream& os, std::string module_name, Program& program, const KeywordDB& keywordDb);
ODBC_PUBLIC_API void generateObjectFile(std::ostream& os, std::string module_name, Program& program, const KeywordDB& keywordDb);
ODBC_PUBLIC_API void generateExecutable(std::ostream& os, std::string module_name, Program& program, const KeywordDB& keywordDb);
}
}