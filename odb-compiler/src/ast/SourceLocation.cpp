#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-sdk/Str.hpp"
#include <fstream>
#include <vector>
#include <sstream>
#include <cassert>

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
SourceLocation::SourceLocation(int firstLine, int lastLine, int firstColumn, int lastColumn, Log::Color color) :
    firstLine_(firstLine),
    lastLine_(lastLine),
    firstColumn_(firstColumn),
    lastColumn_(lastColumn),
    color_(color)
{
}

// ----------------------------------------------------------------------------
int SourceLocation::firstLine() const
{
    return firstLine_;
}

// ----------------------------------------------------------------------------
int SourceLocation::lastLine() const
{
    return lastLine_;
}

// ----------------------------------------------------------------------------
int SourceLocation::firstColumn() const
{
    return firstColumn_;
}

// ----------------------------------------------------------------------------
int SourceLocation::lastColumn() const
{
    return lastColumn_;
}

// ----------------------------------------------------------------------------
Log::Color SourceLocation::color() const
{
    return color_;
}

// ----------------------------------------------------------------------------
void SourceLocation::unionize(const SourceLocation* other)
{
    if (firstLine_ > other->firstLine())
        firstColumn_ = other->firstColumn();
    else if (firstLine_ == other->firstLine())
        if (firstColumn_ > other->firstColumn())
            firstColumn_ = other->firstColumn_;

    if (lastLine_ < other->lastLine())
        lastColumn_ = other->lastColumn();
    else if (lastLine_ == other->lastLine())
        if (lastColumn_ < other->lastColumn())
            lastColumn_ = other->lastColumn();

    if (firstLine_ > other->firstLine())
        firstLine_ = other->firstLine();
    if (lastLine_ < other->lastLine())
        lastLine_ = other->lastLine();
}

// ----------------------------------------------------------------------------
std::vector<std::string> SourceLocation::getUnderlinedSection(std::istream& code) const
{
    auto retError = [this]() -> std::vector<std::string> {
        return {"(Invalid location " + std::to_string(firstLine_) + ","
                                     + std::to_string(lastLine_) + ","
                                     + std::to_string(firstColumn_) + ","
                                     + std::to_string(lastColumn_) + ")",
                ""};
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
        for (int i = (int)pos; i < lastColumn_-1; ++i)
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
        for (int i = firstColumn_; i < lastColumn_-1; ++i)
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
void SourceLocation::printUnderlinedSection(Log& log) const
{
    int gutterWidth = (int)std::to_string(lastLine()).length();
    auto sourceHighlightLines = getUnderlinedSection();
    for (int i = 0; i < (int)sourceHighlightLines.size(); i += 2)
    {
        // Lines are return in groups of 2. The first line is a line from the
        // affected source code. The second line is the error highlight
        // (squiggly lines). We only want to associate a line number with each
        // source code line
        log.log("%*d | ", gutterWidth, firstLine_ + i/2);
        for (size_t j = 0; j < sourceHighlightLines[i].length(); ++j)
        {
            char c1 = sourceHighlightLines[i][j];
            char c2 = j < sourceHighlightLines[i+1].length() ? sourceHighlightLines[i+1][j] : '\0';
            if (c2 == '~' || c2 == '^')
                log.log(Log::FG_BRIGHT_RED, "%c", c1);
            else
                log.log("%c", c1);
        }
        log.log("\n");
        log.log("%*s | ", gutterWidth, "");
        log.log(Log::FG_BRIGHT_RED, "%s\n", sourceHighlightLines[i+1].c_str());
    }
}

// ----------------------------------------------------------------------------
std::string SourceLocation::getLineColumnExtents() const
{
    return std::to_string(firstLine_) + "-" + std::to_string(lastLine_) + ":"
         + std::to_string(firstColumn_) + "-" + std::to_string(lastColumn_);
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
std::vector<std::string> FileSourceLocation::getUnderlinedSection() const
{
    std::ifstream code(fileName_);
    if (!code.is_open())
        return {"(source file was removed)"};
    return SourceLocation::getUnderlinedSection(code);
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
std::vector<std::string> InlineSourceLocation::getUnderlinedSection() const
{
    std::stringstream ss(code_);
    return SourceLocation::getUnderlinedSection(ss);
}

}
}
