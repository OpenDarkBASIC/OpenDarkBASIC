#include <gmock/gmock.h>
#include "odb-compiler/keywords/KeywordMatcher.hpp"
#include "odb-compiler/keywords/KeywordDB.hpp"

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
    KeywordDB db;
    db.addKeyword({"projection matrix4", "", {}, ""});
    db.addKeyword({"randomize", "", {}, ""});
    db.addKeyword({"randomize matrix", "", {}, ""});
    db.addKeyword({"randomize mesh", "", {}, ""});
    db.addKeyword({"read", "", {}, ""});
    matcher->updateFromDB(&db);

    auto result = matcher->findLongestKeywordMatching("randomize");

    EXPECT_THAT(result.found, IsTrue());
    EXPECT_THAT(result.matchedLength, Eq(9));
}

TEST_F(NAME, trailing_space)
{
    KeywordDB db;
    db.addKeyword({"projection matrix4", "", {}, ""});
    db.addKeyword({"randomize", "", {}, ""});
    db.addKeyword({"randomize matrix", "", {}, ""});
    db.addKeyword({"randomize mesh", "", {}, ""});
    db.addKeyword({"read", "", {}, ""});
    matcher->updateFromDB(&db);

    auto result = matcher->findLongestKeywordMatching("randomize ");

    EXPECT_THAT(result.found, IsTrue());
    EXPECT_THAT(result.matchedLength, Eq(9));
}

TEST_F(NAME, longer_symbol)
{
    KeywordDB db;
    db.addKeyword({"projection matrix4", "", {}, ""});
    db.addKeyword({"randomize", "", {}, ""});
    db.addKeyword({"randomize matrix", "", {}, ""});
    db.addKeyword({"randomize mesh", "", {}, ""});
    db.addKeyword({"read", "", {}, ""});
    matcher->updateFromDB(&db);

    auto result = matcher->findLongestKeywordMatching("randomized");

    EXPECT_THAT(result.found, IsFalse());
    EXPECT_THAT(result.matchedLength, Eq(9));
}

TEST_F(NAME, match_longer_string_to_shorter_command)
{
    KeywordDB db;
    db.addKeyword({"projection matrix4", "", {}, ""});
    db.addKeyword({"randomize", "", {}, ""});
    db.addKeyword({"randomize matrix", "", {}, ""});
    db.addKeyword({"randomize mesh", "", {}, ""});
    db.addKeyword({"read", "", {}, ""});
    matcher->updateFromDB(&db);

    auto result = matcher->findLongestKeywordMatching("randomize timer");

    EXPECT_THAT(result.found, IsTrue());
    EXPECT_THAT(result.matchedLength, Eq(9)); // strlen("randomize")
}

TEST_F(NAME, dont_match_shorter_string_to_longer_command)
{
    KeywordDB db;
    db.addKeyword({"dec", "", {}, ""});
    matcher->updateFromDB(&db);

    auto result = matcher->findLongestKeywordMatching("decalmax");

    EXPECT_THAT(result.found, IsFalse());
    EXPECT_THAT(result.matchedLength, Eq(3)); // strlen("dec")
}

TEST_F(NAME, match_when_multiple_options_include_spaces_and_non_spaces)
{
    KeywordDB db;
    db.addKeyword({"DELETE OBJECT COLLISION BOX", "", {}, ""});
    db.addKeyword({"DELETE OBJECT", "", {}, ""});
    db.addKeyword({"DELETE OBJECTS", "", {}, ""});
    matcher->updateFromDB(&db);

    auto result = matcher->findLongestKeywordMatching("delete object 100");

    EXPECT_THAT(result.found, IsTrue());
    EXPECT_THAT(result.matchedLength, Eq(strlen("delete object")));
}
