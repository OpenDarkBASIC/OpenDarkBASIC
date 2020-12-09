#pragma once

#include <string>
#include <vector>
#include <optional>

namespace odb {

class Keyword
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
        std::string name;
        std::string description;
    };

    std::string name;
    std::string helpFile;
    std::vector<Arg> args;
    Type returnType;
};

}
