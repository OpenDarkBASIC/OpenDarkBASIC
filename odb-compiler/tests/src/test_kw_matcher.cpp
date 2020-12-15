#include <gmock/gmock.h>
#include "odb-compiler/keywords/KeywordMatcher.hpp"
#include "odb-compiler/keywords/KeywordIndex.hpp"
#include "odb-compiler/keywords/Keyword.hpp"

#define NAME kw_matcher

using namespace testing;

class NAME : public Test
{
public:
    void SetUp() override
    {
        matcher = new odb::KeywordMatcher;
    }

    void TearDown() override
    {
        delete matcher;
    }

    odb::KeywordMatcher* matcher;
};

using namespace odb;

TEST_F(NAME, empty_db)
{
    auto result = matcher->findLongestKeywordMatching("randomize");

    EXPECT_THAT(result.found, IsFalse());
    EXPECT_THAT(result.matchedLength, Eq(0));
}

TEST_F(NAME, exact_string)
{
    KeywordIndex kwIndex;
    kwIndex.addKeyword(new Keyword(nullptr, "projection matrix4", "", Keyword::Type::Void, {}));
    kwIndex.addKeyword(new Keyword(nullptr, "randomize", "", Keyword::Type::Void, {}));
    kwIndex.addKeyword(new Keyword(nullptr, "randomize matrix", "", Keyword::Type::Void, {}));
    kwIndex.addKeyword(new Keyword(nullptr, "randomize mesh", "", Keyword::Type::Void, {}));
    kwIndex.addKeyword(new Keyword(nullptr, "read", "", Keyword::Type::Void, {}));
    matcher->updateFromIndex(&kwIndex);

    auto result = matcher->findLongestKeywordMatching("randomize");

    EXPECT_THAT(result.found, IsTrue());
    EXPECT_THAT(result.matchedLength, Eq(9));
}

TEST_F(NAME, trailing_space)
{
    KeywordIndex kwIndex;
    kwIndex.addKeyword(new Keyword(nullptr, "projection matrix4", "", Keyword::Type::Void, {}));
    kwIndex.addKeyword(new Keyword(nullptr, "randomize", "", Keyword::Type::Void, {}));
    kwIndex.addKeyword(new Keyword(nullptr, "randomize matrix", "", Keyword::Type::Void, {}));
    kwIndex.addKeyword(new Keyword(nullptr, "randomize mesh", "", Keyword::Type::Void, {}));
    kwIndex.addKeyword(new Keyword(nullptr, "read", "", Keyword::Type::Void, {}));
    matcher->updateFromIndex(&kwIndex);

    auto result = matcher->findLongestKeywordMatching("randomize ");

    EXPECT_THAT(result.found, IsTrue());
    EXPECT_THAT(result.matchedLength, Eq(9));
}

TEST_F(NAME, longer_symbol)
{
    KeywordIndex kwIndex;
    kwIndex.addKeyword(new Keyword(nullptr, "projection matrix4", "", Keyword::Type::Void, {}));
    kwIndex.addKeyword(new Keyword(nullptr, "randomize", "", Keyword::Type::Void, {}));
    kwIndex.addKeyword(new Keyword(nullptr, "randomize matrix", "", Keyword::Type::Void, {}));
    kwIndex.addKeyword(new Keyword(nullptr, "randomize mesh", "", Keyword::Type::Void, {}));
    kwIndex.addKeyword(new Keyword(nullptr, "read", "", Keyword::Type::Void, {}));
    matcher->updateFromIndex(&kwIndex);

    auto result = matcher->findLongestKeywordMatching("randomized");

    EXPECT_THAT(result.found, IsFalse());
    EXPECT_THAT(result.matchedLength, Eq(9));
}

TEST_F(NAME, match_longer_string_to_shorter_command)
{
    KeywordIndex kwIndex;
    kwIndex.addKeyword(new Keyword(nullptr, "projection matrix4", "", Keyword::Type::Void, {}));
    kwIndex.addKeyword(new Keyword(nullptr, "randomize", "", Keyword::Type::Void, {}));
    kwIndex.addKeyword(new Keyword(nullptr, "randomize matrix", "", Keyword::Type::Void, {}));
    kwIndex.addKeyword(new Keyword(nullptr, "randomize mesh", "", Keyword::Type::Void, {}));
    kwIndex.addKeyword(new Keyword(nullptr, "read", "", Keyword::Type::Void, {}));
    matcher->updateFromIndex(&kwIndex);

    auto result = matcher->findLongestKeywordMatching("randomize timer");

    EXPECT_THAT(result.found, IsTrue());
    EXPECT_THAT(result.matchedLength, Eq(9)); // strlen("randomize")
}

TEST_F(NAME, dont_match_shorter_string_to_longer_command)
{
    KeywordIndex kwIndex;
    kwIndex.addKeyword(new Keyword(nullptr, "dec", "", Keyword::Type::Void, {}));
    matcher->updateFromIndex(&kwIndex);

    auto result = matcher->findLongestKeywordMatching("decalmax");

    EXPECT_THAT(result.found, IsFalse());
    EXPECT_THAT(result.matchedLength, Eq(3)); // strlen("dec")
}

TEST_F(NAME, match_when_multiple_options_include_spaces_and_non_spaces)
{
    KeywordIndex kwIndex;
    kwIndex.addKeyword(new Keyword(nullptr, "DELETE OBJECT COLLISION BOX", "", Keyword::Type::Void, {}));
    kwIndex.addKeyword(new Keyword(nullptr, "DELETE OBJECT", "", Keyword::Type::Void, {}));
    kwIndex.addKeyword(new Keyword(nullptr, "DELETE OBJECTS", "", Keyword::Type::Void, {}));
    matcher->updateFromIndex(&kwIndex);

    auto result = matcher->findLongestKeywordMatching("delete object 100");

    EXPECT_THAT(result.found, IsTrue());
    EXPECT_THAT(result.matchedLength, Eq(strlen("delete object")));
}
