#pragma once

#include "odb-compiler/config.hpp"
#include "odb-sdk/Reference.hpp"
#include <istream>
#include <vector>
#include <memory>

namespace odb {
namespace ast {

class ODBCOMPILER_PUBLIC_API SourceLocation : public RefCounted
{
public:
    SourceLocation(int firstLine, int lastLine, int firstColumn, int lastColumn);

    virtual std::string getFileLineColumn() const = 0;
    virtual std::vector<std::string> getSectionHighlight() const = 0;

    int getFirstLine() const;
    int getLastLine() const;
    int getFirstColumn() const;
    int getLastColumn() const;

protected:
    std::vector<std::string> getSectionHighlight(std::istream& code) const;

protected:
    int firstLine_;
    int lastLine_;
    int firstColumn_;
    int lastColumn_;
};

class ODBCOMPILER_PUBLIC_API FileSourceLocation : public SourceLocation
{
public:
    FileSourceLocation(const std::string& fileName,
        int firstLine, int lastLine, int firstColumn, int lastColumn);

    std::string getFileLineColumn() const override;
    std::vector<std::string> getSectionHighlight() const override;

private:
    std::string fileName_;
};

class ODBCOMPILER_PUBLIC_API InlineSourceLocation : public SourceLocation
{
public:
    InlineSourceLocation(const std::string& sourceName, const std::string& code,
        int firstLine, int lastLine, int firstColumn, int lastColumn);

    std::string getFileLineColumn() const override;
    std::vector<std::string> getSectionHighlight() const override;

private:
    std::string sourceName_;
    std::string code_;
};

}
}
