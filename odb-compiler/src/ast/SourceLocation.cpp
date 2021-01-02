#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-sdk/Str.hpp"
#include <fstream>
#include <vector>
#include <sstream>

#include <iostream>

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
SourceLocation::SourceLocation(int firstLine, int lastLine, int firstColumn, int lastColumn) :
    firstLine_(firstLine),
    lastLine_(lastLine),
    firstColumn_(firstColumn),
    lastColumn_(lastColumn)
{
}

// ----------------------------------------------------------------------------
int SourceLocation::getFirstLine() const
{
    return firstLine_;
}

// ----------------------------------------------------------------------------
int SourceLocation::getLastLine() const
{
    return lastLine_;
}

// ----------------------------------------------------------------------------
int SourceLocation::getFirstColumn() const
{
    return firstColumn_;
}

// ----------------------------------------------------------------------------
int SourceLocation::getLastColumn() const
{
    return lastColumn_;
}

// ----------------------------------------------------------------------------
std::vector<std::string> SourceLocation::getSectionHighlight(std::istream& code) const
{
    auto retError = [this]() -> std::vector<std::string> {
        return {"(Invalid location " + std::to_string(firstLine_) + ","
                                     + std::to_string(lastLine_) + ","
                                     + std::to_string(firstColumn_) + ","
                                     + std::to_string(lastColumn_) + ")"};
    };
    // Seek to first line
    int currentLine = 0;
    std::string line;
    while (currentLine < firstLine_)
    {
        if (code.eof())
            return retError();
        std::getline(code, line);
        currentLine++;
    }

    std::vector<std::string> lines;
    lines.push_back(line);

    // Section might span more than one line
    while (currentLine < lastLine_)
    {
        if (code.eof())
            return retError();
        std::getline(code, line);
        lines.push_back(line);
        currentLine++;
    }

    std::vector<std::string> squiggles;
    squiggles.emplace_back();
    for (int i = 1; i < firstColumn_; ++i)
    {
        if (i >= (int)lines[0].length())
            return retError();
        squiggles.back() += " ";
        if (lines[0][i-1] == '\t')
            squiggles.back() += "   ";
    }
    squiggles.back() += "^";
    if (firstColumn_ > 0 && lines[0][firstColumn_-1] == '\t')
            squiggles.back() += "~~~";

    if (lastLine_ != firstLine_)
    {
        for (int i = firstColumn_ + 1; i <= (int)lines[0].length(); ++i)
        {
            squiggles.back() += "~";
            if (lines[0][i-1] == '\t')
                squiggles.back() += "~~~";
        }

        for (int l = 1; l < (int)lines.size() - 1; ++l)
        {
            squiggles.emplace_back();
            auto pos = lines[l].find_first_not_of(" \t");
            if (pos == std::string::npos)
                pos = 0;
            for (int i = 0; i < (int)pos; ++i)
                squiggles.back() += lines[l][i] == '\t' ? "    " : " ";
            for (int i = (int)pos; i < (int)lines[l].length(); ++i)
            {
                squiggles.back() += "~";
                if (lines[l][i] == '\t')
                    squiggles.back() += "~~~";
            }
        }

        squiggles.emplace_back();
        auto pos = lines.back().find_first_not_of(" \t");
        if (pos == std::string::npos)
            pos = 0;
        for (int i = 0; i < (int)pos; ++i)
            squiggles.back() += lines.back()[i] == '\t' ? "    " : " ";
        for (int i = (int)pos; i < lastColumn_; ++i)
        {
            if (i >= (int)lines.back().length())
                return retError();
            squiggles.back() += "~";
            if (lines.back()[i] == '\t')
                squiggles.back() += "~~~";
        }
    }
    else
    {
        for (int i = firstColumn_; i < lastColumn_; ++i)
        {
            if (i >= (int)lines[0].length())
                return retError();
            squiggles.back() += "~";
            if (i > 0 && lines[0][i-1] == '\t')
                squiggles.back() += "~~~";
        }
    }

    for (auto& line : lines)
        str::replaceAll(line, "\t", "    ");

    for (size_t i = 0; i != squiggles.size(); ++i)
        lines.insert(lines.begin() + i*2 + 1, squiggles[i]);

    return lines;
}

// ----------------------------------------------------------------------------
FileSourceLocation::FileSourceLocation(const std::string& fileName,
        int firstLine, int lastLine, int firstColumn, int lastColumn) :
    SourceLocation(firstLine, lastLine, firstColumn, lastColumn),
    fileName_(fileName)
{
}

// ----------------------------------------------------------------------------
std::string FileSourceLocation::getFileLineColumn() const
{
    return fileName_ + ":" + std::to_string(firstLine_) + ":" + std::to_string(firstColumn_);
}

// ----------------------------------------------------------------------------
std::vector<std::string> FileSourceLocation::getSectionHighlight() const
{
    std::ifstream code(fileName_);
    if (!code.is_open())
        return {"(source file was removed)"};
    return SourceLocation::getSectionHighlight(code);
}

// ----------------------------------------------------------------------------
InlineSourceLocation::InlineSourceLocation(const std::string& sourceName, const std::string& code,
        int firstLine, int lastLine, int firstColumn, int lastColumn) :
    SourceLocation(firstLine, lastLine, firstColumn, lastColumn),
    sourceName_(sourceName),
    code_(code)
{
}

// ----------------------------------------------------------------------------
std::string InlineSourceLocation::getFileLineColumn() const
{
    return sourceName_ + ":" + std::to_string(firstLine_) + ":" + std::to_string(firstColumn_);
}

// ----------------------------------------------------------------------------
std::vector<std::string> InlineSourceLocation::getSectionHighlight() const
{
    std::stringstream ss(code_);
    return SourceLocation::getSectionHighlight(ss);
}

}
}
