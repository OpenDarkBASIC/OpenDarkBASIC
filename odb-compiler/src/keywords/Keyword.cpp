#include "odb-compiler/keywords/Keyword.hpp"
#include "odb-sdk/DynamicLibrary.hpp"

namespace odb {
namespace kw {

// ----------------------------------------------------------------------------
Keyword::Keyword(DynamicLibrary* sourceLibrary,
                 const std::string& dbSymbol,
                 const std::string& cppSymbol,
                 Type returnType,
                 const std::vector<Arg>& args,
                 const std::string helpFile) :
    library_(sourceLibrary),
    dbSymbol_(dbSymbol),
    cppSymbol_(cppSymbol),
    helpFile_(helpFile),
    args_(args),
    returnType_(returnType)
{
}

// ----------------------------------------------------------------------------
const std::string& Keyword::dbSymbol() const
{
    return dbSymbol_;
}

// ----------------------------------------------------------------------------
const std::string& Keyword::cppSymbol() const
{
    return cppSymbol_;
}

// ----------------------------------------------------------------------------
const std::string& Keyword::helpFile() const
{
    return helpFile_;
}

// ----------------------------------------------------------------------------
const std::vector<Keyword::Arg>& Keyword::args() const
{
    return args_;
}

// ----------------------------------------------------------------------------
Keyword::Type Keyword::returnType() const
{
    return returnType_;
}

// ----------------------------------------------------------------------------
DynamicLibrary* Keyword::library() const
{
    return library_;
}

}
}
