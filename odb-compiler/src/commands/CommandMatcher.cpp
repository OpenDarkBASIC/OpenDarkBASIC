#include "odb-compiler/commands/CommandMatcher.hpp"
#include "odb-compiler/commands/CommandIndex.hpp"
#include "odb-sdk/Str.hpp"
#include <algorithm>
#include <iostream>
#include <cstring>

namespace odb {
namespace cmd {

// ----------------------------------------------------------------------------
static bool charIsSymbolToken(char c)
{
    if (c >= '0' && c <= '9')
        return true;
    if (c >= 'A' && c <= 'Z')
        return true;
    if (c >= 'a' && c <= 'z')
        return true;
    if (c == '_')
        return true;
    return false;
}

// ----------------------------------------------------------------------------
void CommandMatcher::updateFromIndex(const CommandIndex* db)
{
    longestCommandLength_ = 0;
    longestCommandWordCount_ = 0;
    commands_ = db->commandNamesAsList();

    // Commands are case insensitive, so transform all to lower case
    for (auto& s : commands_)
        str::toLowerInplace(s);

    // Lexicographic sort so we can binary search later
    std::sort(commands_.begin(), commands_.end(), [](const std::string& a,const  std::string& b) { return a < b; });

    // Find the longest command so the lexicographic comparison is faster during
    // binary search
    auto longestCommand = std::max_element(commands_.begin(), commands_.end(),
            [](const std::string& a, const std::string& b) { return a.size() < b.size(); });
    if (longestCommand != commands_.end())
        longestCommandLength_ = (int)longestCommand->size();

    // Find the maximum number of words that appear in a command. This is not
    // necessarily the longest command. Counting the number of spaces should
    // work in all cases.
    auto wordCount = [](const std::string& str) {
        return std::count(str.begin(), str.end(), ' ') + 1;
    };
    auto longestCommandWordCount = std::max_element(commands_.begin(), commands_.end(),
            [&wordCount](const std::string& a, const std::string& b) {
                return wordCount(a) < wordCount(b);
            });
    if (longestCommandWordCount != commands_.end())
        longestCommandWordCount_ = (int)wordCount(*longestCommandWordCount);
}

// ----------------------------------------------------------------------------
CommandMatcher::MatchResult CommandMatcher::findLongestCommandMatching(const std::string& str) const
{
    auto first = commands_.begin();
    auto last = commands_.end();
    std::vector<std::vector<std::string>::const_iterator> commandStack;
    commandStack.reserve(longestCommandWordCount());
    int matchedLen = 0;
    while (str[matchedLen])
    {
        auto compare = [&](const std::string& a, const std::string& b) {
            return std::tolower(a[matchedLen]) < std::tolower(b[matchedLen]);
        };
        first = std::lower_bound(first, last, str, compare);
        last = std::upper_bound(first, last, str, compare);
        if (first == last)
            break;

        ++matchedLen;
        if (!charIsSymbolToken(str[matchedLen]))
            commandStack.push_back(first);
    }

    while (commandStack.size())
    {
        auto command = commandStack.back();
        if (command != commands_.end() && str::strncicmp(str.c_str(), command->c_str(), command->length()) == 0)
            return { (int)command->length(), true };
        commandStack.pop_back();
    }

    return { matchedLen, false };
}

// ----------------------------------------------------------------------------
int CommandMatcher::longestCommandLength() const
{
    return longestCommandLength_;
}

// ----------------------------------------------------------------------------
int CommandMatcher::longestCommandWordCount() const
{
    return longestCommandWordCount_;
}

}
}
