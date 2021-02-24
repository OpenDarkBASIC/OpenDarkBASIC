#pragma once

#include "gmock/gmock-matchers.h"
#include "odb-compiler/ast/Datatypes.hpp"

namespace odb::ast {
    template <typename T> class LiteralTemplate;

#define X(dbname, cppname) class dbname##Literal;
    ODB_DATATYPE_LIST
#undef X
}

#define X(dbname, cppname)                                                    \
class dbname##LiteralEqMatcher : public testing::MatcherInterface<const odb::ast::dbname##Literal*>\
{                                                                             \
public:                                                                       \
    explicit dbname##LiteralEqMatcher(const cppname& expectedValue);          \
    bool MatchAndExplain(const odb::ast::dbname##Literal* literal, testing::MatchResultListener* listener) const override;\
    void DescribeTo(::std::ostream* os) const override;                       \
    void DescribeNegationTo(::std::ostream* os) const override;               \
                                                                              \
private:                                                                      \
    const cppname expectedValue_;                                             \
};
ODB_DATATYPE_LIST
#undef X

#define X(dbname, cppname) \
inline testing::Matcher<const odb::ast::dbname##Literal*> dbname##LiteralEq(const cppname& value) { \
    return testing::MakeMatcher(new dbname##LiteralEqMatcher(value)); \
}
ODB_DATATYPE_LIST
#undef X
