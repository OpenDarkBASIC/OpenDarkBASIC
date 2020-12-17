#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/keywords/SDKType.hpp"
#include "odb-sdk/Reference.hpp"
#include <string>
#include <vector>

namespace odb {
namespace kw {

class Keyword;

/*!
 * This class is a generic container for all keywords. It acts as an
 * intermediate storage when collecting keywords from plugins or config files.
 *
 * This class is not designed for fast keyword queries. It is recommended to
 * create a specialized container if this is required. An example of this is
 * the @see KeywordMatcher class.
 */
class ODBCOMPILER_PUBLIC_API KeywordIndex
{
public:
    void addKeyword(Keyword* keyword);

    /*!
     * @brief Tries to find any globally conflicting keywords, such as identical
     * keywords coming from different plugins, or keywords that share the same
     * overload.
     */
    bool findConflicts() const;

    const std::vector<Reference<Keyword>>& keywords() const;
    std::vector<std::string> keywordNamesAsList() const;
    std::vector<std::string> librariesAsList() const;

private:
    std::vector<Reference<Keyword>> keywords_;
};

}
}
