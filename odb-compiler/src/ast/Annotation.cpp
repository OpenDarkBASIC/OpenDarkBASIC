#include "odb-compiler/ast/Annotation.hpp"

namespace odb::ast {

const char* typeAnnotationEnumString(Annotation annotation)
{
    static const char* table[] = {
        "NONE",
#define X(enum_, chr, str, dbname) #enum_,
        ODB_TYPE_ANNOTATION_LIST
#undef X
    };

    return table[static_cast<int>(annotation)];
}

}
