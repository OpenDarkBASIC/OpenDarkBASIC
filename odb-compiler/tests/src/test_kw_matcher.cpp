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
        matcher = new odb::kw::KeywordMatcher;
    }

    void TearDown() override
    {
        delete matcher;
    }

    odb::kw::KeywordMatcher* matcher;
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
    kw::KeywordIndex kwIndex;
    kwIndex.addKeyword(new kw::Keyword(nullptr, "projection matrix4", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "randomize", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "randomize matrix", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "randomize mesh", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "read", "", kw::Keyword::Type::Void, {}));
    matcher->updateFromIndex(&kwIndex);

    auto result = matcher->findLongestKeywordMatching("randomize");

    EXPECT_THAT(result.found, IsTrue());
    EXPECT_THAT(result.matchedLength, Eq(9));
}

TEST_F(NAME, trailing_space)
{
    kw::KeywordIndex kwIndex;
    kwIndex.addKeyword(new kw::Keyword(nullptr, "projection matrix4", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "randomize", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "randomize matrix", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "randomize mesh", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "read", "", kw::Keyword::Type::Void, {}));
    matcher->updateFromIndex(&kwIndex);

    auto result = matcher->findLongestKeywordMatching("randomize ");

    EXPECT_THAT(result.found, IsTrue());
    EXPECT_THAT(result.matchedLength, Eq(9));
}

TEST_F(NAME, longer_symbol)
{
    kw::KeywordIndex kwIndex;
    kwIndex.addKeyword(new kw::Keyword(nullptr, "projection matrix4", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "randomize", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "randomize matrix", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "randomize mesh", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "read", "", kw::Keyword::Type::Void, {}));
    matcher->updateFromIndex(&kwIndex);

    auto result = matcher->findLongestKeywordMatching("randomized");

    EXPECT_THAT(result.found, IsFalse());
    EXPECT_THAT(result.matchedLength, Eq(9));
}

TEST_F(NAME, match_longer_string_to_shorter_command)
{
    kw::KeywordIndex kwIndex;
    kwIndex.addKeyword(new kw::Keyword(nullptr, "projection matrix4", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "randomize", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "randomize matrix", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "randomize mesh", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "read", "", kw::Keyword::Type::Void, {}));
    matcher->updateFromIndex(&kwIndex);

    auto result = matcher->findLongestKeywordMatching("randomize timer");

    EXPECT_THAT(result.found, IsTrue());
    EXPECT_THAT(result.matchedLength, Eq(9)); // strlen("randomize")
}

TEST_F(NAME, dont_match_shorter_string_to_longer_command)
{
    kw::KeywordIndex kwIndex;
    kwIndex.addKeyword(new kw::Keyword(nullptr, "dec", "", kw::Keyword::Type::Void, {}));
    matcher->updateFromIndex(&kwIndex);

    auto result = matcher->findLongestKeywordMatching("decalmax");

    EXPECT_THAT(result.found, IsFalse());
    EXPECT_THAT(result.matchedLength, Eq(3)); // strlen("dec")
}

TEST_F(NAME, match_when_multiple_options_include_spaces_and_non_spaces)
{
    kw::KeywordIndex kwIndex;
    kwIndex.addKeyword(new kw::Keyword(nullptr, "DELETE OBJECT COLLISION BOX", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "DELETE OBJECT", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "DELETE OBJECTS", "", kw::Keyword::Type::Void, {}));
    matcher->updateFromIndex(&kwIndex);

    auto result = matcher->findLongestKeywordMatching("delete object 100");

    EXPECT_THAT(result.found, IsTrue());
    EXPECT_THAT(result.matchedLength, Eq(strlen("delete object")));
}
