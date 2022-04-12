#pragma once

#include "odb-compiler/ast/SourceLocation.hpp"

#include <cstdio>

namespace odb::ir {
template <typename... Args>
[[noreturn]] void fatalError(ast::SourceLocation* location, const char* message, Args&&... args)
{
    fprintf(stderr, "%s: FATAL ERROR: ", location->getFileLineColumn().c_str());
    fprintf(stderr, message, args...);
    fprintf(stderr, "\n");
    std::terminate();
}

template <typename... Args> [[noreturn]] void fatalError(const char* message, Args&&... args)
{
    fprintf(stderr, "<unknown>: FATAL ERROR: ");
    fprintf(stderr, message, args...);
    fprintf(stderr, "\n");
    std::terminate();
}
} // namespace