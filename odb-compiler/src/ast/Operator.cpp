#include "odb-compiler/ast/Operators.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
const char* binaryOpTypeEnumString(BinaryOpType op)
{
    static const char* table[] = {
#define X(op, tok) #op,
        ODB_BINARY_OP_LIST
#undef X
    };
    return table[static_cast<int>(op)];
}

// ----------------------------------------------------------------------------
const char* unaryOpTypeEnumString(UnaryOpType op)
{
    static const char* table[] = {
#define X(op, tok) #op,
        ODB_UNARY_OP_LIST
#undef X
    };
    return table[static_cast<int>(op)];
}

}
