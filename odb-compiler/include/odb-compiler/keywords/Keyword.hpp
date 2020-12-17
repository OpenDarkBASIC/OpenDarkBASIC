#pragma once

#include "odb-compiler/config.hpp"
#include "odb-sdk/Reference.hpp"
#include <string>
#include <vector>
#include <optional>

namespace odb {
class DynamicLibrary;

namespace kw {

class Keyword : public RefCounted
{
public:
    // See https://github.com/TheGameCreators/Dark-Basic-Pro/blob/Initial-Files/Install/Help/documents/1%20Third%20Party%20Commands.htm#L112
    // for a table of keyword types.
    enum class Type: char
    {
        Integer = 'L',
        Float   = 'F',
        String  = 'S',
        Double  = 'O',
        Long    = 'R',
        Dword   = 'D', // Boolean, BYTE, WORD and DWORD.
        Void    = '0'
    };

    struct Arg
    {
        Type type;
        std::string symName;
        std::string description;
    };

    Keyword(DynamicLibrary* sourceLibrary,
            const std::string& dbSymbol,
            const std::string& cppSymbol,
            Type returnType,
            const std::vector<Arg>& args,
            const std::string helpFile="");

    const std::string& dbSymbol() const;
    const std::string& cppSymbol() const;
    const std::string& helpFile() const;
    const std::vector<Arg>& args() const;
    Type returnType() const;

    DynamicLibrary* library() const;

private:
    Reference<DynamicLibrary> library_;
    std::string dbSymbol_;
    std::string cppSymbol_;
    std::string helpFile_;
    std::vector<Arg> args_;
    Type returnType_;
};

}
}
