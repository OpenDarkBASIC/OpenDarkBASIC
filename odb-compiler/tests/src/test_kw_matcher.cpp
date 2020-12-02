#include <gmock/gmock.h>
#include "odb-compiler/keywords/KeywordMatcher.hpp"
#include "odb-compiler/keywords/KeywordIndex.hpp"

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
    kwIndex.addKeyword({"projection matrix4", "", "", {}, std::nullopt});
    kwIndex.addKeyword({"randomize", "", "", {}, std::nullopt});
    kwIndex.addKeyword({"randomize matrix", "", "", {}, std::nullopt});
    kwIndex.addKeyword({"randomize mesh", "", "", {}, std::nullopt});
    kwIndex.addKeyword({"read", "", "", {}, std::nullopt});
    matcher->updateFromIndex(&kwIndex);

    auto result = matcher->findLongestKeywordMatching("randomize");

    EXPECT_THAT(result.found, IsTrue());
    EXPECT_THAT(result.matchedLength, Eq(9));
}

TEST_F(NAME, trailing_space)
{
    KeywordIndex kwIndex;
    kwIndex.addKeyword({"projection matrix4", "", "", {}, std::nullopt});
    kwIndex.addKeyword({"randomize", "", "", {}, std::nullopt});
    kwIndex.addKeyword({"randomize matrix", "", "", {}, std::nullopt});
    kwIndex.addKeyword({"randomize mesh", "", "", {}, std::nullopt});
    kwIndex.addKeyword({"read", "", "", {}, std::nullopt});
    matcher->updateFromIndex(&kwIndex);

    auto result = matcher->findLongestKeywordMatching("randomize ");

    EXPECT_THAT(result.found, IsTrue());
    EXPECT_THAT(result.matchedLength, Eq(9));
}

TEST_F(NAME, longer_symbol)
{
    KeywordIndex kwIndex;
    kwIndex.addKeyword({"projection matrix4", "", "", {}, std::nullopt});
    kwIndex.addKeyword({"randomize", "", "", {}, std::nullopt});
    kwIndex.addKeyword({"randomize matrix", "", "", {}, std::nullopt});
    kwIndex.addKeyword({"randomize mesh", "", "", {}, std::nullopt});
    kwIndex.addKeyword({"read", "", "", {}, std::nullopt});
    matcher->updateFromIndex(&kwIndex);

    auto result = matcher->findLongestKeywordMatching("randomized");

    EXPECT_THAT(result.found, IsFalse());
    EXPECT_THAT(result.matchedLength, Eq(9));
}

TEST_F(NAME, match_longer_string_to_shorter_command)
{
    KeywordIndex kwIndex;
    kwIndex.addKeyword({"projection matrix4", "", "", {}, std::nullopt});
    kwIndex.addKeyword({"randomize", "", "", {}, std::nullopt});
    kwIndex.addKeyword({"randomize matrix", "", "", {}, std::nullopt});
    kwIndex.addKeyword({"randomize mesh", "", "", {}, std::nullopt});
    kwIndex.addKeyword({"read", "", "", {}, std::nullopt});
    matcher->updateFromIndex(&kwIndex);

    auto result = matcher->findLongestKeywordMatching("randomize timer");

    EXPECT_THAT(result.found, IsTrue());
    EXPECT_THAT(result.matchedLength, Eq(9)); // strlen("randomize")
}

TEST_F(NAME, dont_match_shorter_string_to_longer_command)
{
    KeywordIndex kwIndex;
    kwIndex.addKeyword({"dec", "", "", {}, std::nullopt});
    matcher->updateFromIndex(&kwIndex);

    auto result = matcher->findLongestKeywordMatching("decalmax");

    EXPECT_THAT(result.found, IsFalse());
    EXPECT_THAT(result.matchedLength, Eq(3)); // strlen("dec")
}

TEST_F(NAME, match_when_multiple_options_include_spaces_and_non_spaces)
{
    KeywordIndex kwIndex;
    kwIndex.addKeyword({"DELETE OBJECT COLLISION BOX", "", "", {}, std::nullopt});
    kwIndex.addKeyword({"DELETE OBJECT", "", "", {}, std::nullopt});
    kwIndex.addKeyword({"DELETE OBJECTS", "", "", {}, std::nullopt});
    matcher->updateFromIndex(&kwIndex);

    auto result = matcher->findLongestKeywordMatching("delete object 100");

    EXPECT_THAT(result.found, IsTrue());
    EXPECT_THAT(result.matchedLength, Eq(strlen("delete object")));
}
