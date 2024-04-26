#pragma once

#include "odb-compiler/config.h"
#include <string>
#include <string_view>
#include <vector>

namespace odb {
namespace cmd {

class CommandIndex;

/*!
 * The Purpose of this class is to provide the lexer a way to determine if
 * a sequence of symbol tokens represents a command or not.
 *
 * This class only stores the commands as strings and sorts them lexicographically
 * to make matching fast.
 */
class ODBCOMPILER_PUBLIC_API CommandMatcher
{
public:
    struct MatchResult
    {
        int matchedLength = 0;
        bool found = false;
    };

    /*!
     * Loads all commands as strings and sorts them lexicographically.
     */
    void updateFromIndex(const CommandIndex* index);

    /*!
     * Matches as many characters of the input string as possible.
     *
     * The function returns the number of characters that were successfully
     * matched, and if the input string was an exact match, MatchResult::found
     * will be set to true.
     *
     * Note that commands can be partially matched (matchedLength>0, found=false)
     * but also exact matches shorter than the input string can occur
     * (matchedLength < str.length(), found=true).
     */
    MatchResult findLongestCommandMatching(const std::string& str) const;

    /*!
     * Returns the longest possible match length. Useful for preallocating a
     * buffer.
     */
    int longestCommandLength() const;

    /*!
     * Returns the maximum number of words (separated by space) that can make
     * up a command. Useful for preallocating a buffer.
     */
    int longestCommandWordCount() const;

private:
    std::vector<std::string> commands_;
    int longestCommandLength_ = 0;
    int longestCommandWordCount_ = 0;
};

}
}
