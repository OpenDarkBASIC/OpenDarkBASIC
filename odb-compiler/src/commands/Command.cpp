#include "odb-compiler/commands/Command.hpp"

namespace odb {
namespace cmd {

// ----------------------------------------------------------------------------
Command::Command(TargetLibParser* sourceLibrary,
                 const std::string& dbSymbol,
                 const std::string& cppSymbol,
                 Type returnType,
                 const std::vector<Arg>& args,
                 const std::string& helpFile) :
    library_(sourceLibrary),
    dbSymbol_(dbSymbol),
    cppSymbol_(cppSymbol),
    helpFile_(helpFile),
    args_(args),
    returnType_(returnType)
{
}

// ----------------------------------------------------------------------------
const std::string& Command::dbSymbol() const
{
    return dbSymbol_;
}

// ----------------------------------------------------------------------------
const std::string& Command::cppSymbol() const
{
    return cppSymbol_;
}

// ----------------------------------------------------------------------------
const std::string& Command::helpFile() const
{
    return helpFile_;
}

// ----------------------------------------------------------------------------
const std::vector<Command::Arg>& Command::args() const
{
    return args_;
}

// ----------------------------------------------------------------------------
Command::Type Command::returnType() const
{
    return returnType_;
}

// ----------------------------------------------------------------------------
TargetLibParser* Command::library() const
{
    return library_;
}

}
}
