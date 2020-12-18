#pragma once

#include "odb-compiler/config.hpp"
#include <string>
#include <string_view>
#include <vector>

namespace odb {
namespace kw {

class KeywordIndex;

/*!
 * The Purpose of this class is to provide the lexer a way to determine if
 * a sequence of symbol tokens represents a keyword or not.
 *
 * This class only stores the keywords as strings and sorts them lexicographically
 * to make matching fast.
 */
class ODBCOMPILER_PUBLIC_API KeywordMatcher
{
public:
    struct MatchResult
    {
        int matchedLength = 0;
        bool found = false;
    };

    /*!
     * Loads all keywords as strings and sorts them lexicographically.
     */
    void updateFromIndex(const KeywordIndex* index);

    /*!
     * Matches as many characters of the input string as possible.
     *
     * The function returns the number of characters that were successfully
     * matched, and if the input string was an exact match, MatchResult::found
     * will be set to true.
     *
     * Note that keywords can be partially matched (matchedLength>0, found=false)
     * but also exact matches shorter than the input string can occur
     * (matchedLength < str.length(), found=true).
     */
    MatchResult findLongestKeywordMatching(const std::string& str) const;

    /*!
     * Returns the longest possible match length. Useful for preallocating a
     * buffer.
     */
    int longestKeywordLength() const;

    /*!
     * Returns the maximum number of words (separated by space) that can make
     * up a keyword. Useful for preallocating a buffer.
     */
    int longestKeywordWordCount() const;

private:
    std::vector<std::string> keywords_;
    int longestKeywordLength_ = 0;
    int longestKeywordWordCount_ = 0;
};

}
}
