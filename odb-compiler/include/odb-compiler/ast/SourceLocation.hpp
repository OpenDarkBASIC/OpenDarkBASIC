#pragma once

#include "odb-compiler/config.hpp"
#include "odb-sdk/Log.hpp"
#include "odb-sdk/RefCounted.hpp"
#include <istream>
#include <vector>
#include <memory>

namespace odb {
namespace ast {

/*!
 * @brief Stores information on where a particular token or group of tokens
 * originated from (file, line and column).
 *
 * The data is stored as first/last line and first/last column.
 *
 * The first/last line range is inclusive, so lines 1-3 would mean lines 1, 2
 * and 3. The first/last column range is exclusive, so columns 1-3 would mean
 * columns 1 and 2. This is just an artefact of the way lexing is implemented.
 */
class ODBCOMPILER_PUBLIC_API SourceLocation : public RefCounted
{
public:
    SourceLocation(int firstLine, int lastLine, int firstColumn, int lastColumn, Log::Color color=Log::RESET);

    /*!
     * @brief Returns the location in the format "fl-ll:fc-lc" where fl=first line,
     * ll=last line, fc=first column, lc=last column
     */
    std::string getLineColumnExtents() const;

    /*!
     * @brief Returns the location in the format "file:line:column" (first line
     * and first column)
     */
    virtual std::string getFileLineColumn() const = 0;

    /*!
     * @brief Returns the affected lines from the source file or source text,
     * and inserts "squiggles" to highlight the parts that are relevant. The
     * strings can be "\n".join()'d to produce a printable block.
     */
    virtual std::vector<std::string> getUnderlinedSection() const = 0;

    void printUnderlinedSection(Log& log) const;

    int firstLine() const;
    int lastLine() const;
    int firstColumn() const;
    int lastColumn() const;
    Log::Color color() const;

    void unionize(const SourceLocation* other);

protected:
    std::vector<std::string> getUnderlinedSection(std::istream& code) const;

protected:
    int firstLine_;
    int lastLine_;
    int firstColumn_;
    int lastColumn_;
    Log::Color color_;
};

class ODBCOMPILER_PUBLIC_API FileSourceLocation : public SourceLocation
{
public:
    FileSourceLocation(const std::string& fileName,
        int firstLine, int lastLine, int firstColumn, int lastColumn);

    std::string getFileLineColumn() const override;
    std::vector<std::string> getUnderlinedSection() const override;

private:
    std::string fileName_;
};

class ODBCOMPILER_PUBLIC_API InlineSourceLocation : public SourceLocation
{
public:
    InlineSourceLocation(const std::string& sourceName, const std::string& code,
        int firstLine, int lastLine, int firstColumn, int lastColumn);

    std::string getFileLineColumn() const override;
    std::vector<std::string> getUnderlinedSection() const override;

private:
    std::string sourceName_;
    std::string code_;
};

}
}
