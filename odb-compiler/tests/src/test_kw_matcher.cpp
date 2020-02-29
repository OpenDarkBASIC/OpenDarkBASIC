#include <gmock/gmock.h>
#include "odbc/parsers/keywords/KeywordMatcher.hpp"
#include "odbc/parsers/keywords/KeywordDB.hpp"

#define NAME kw_matcher

using namespace testing;

class NAME : public Test
{
public:
    void SetUp() override
    {
        matcher = new odbc::KeywordMatcher;
    }

    void TearDown() override
    {
        delete matcher;
    }

    odbc::KeywordMatcher* matcher;
};

using namespace odbc;

TEST_F(NAME, empty_db)
{
    auto result = matcher->findLongestKeywordMatching("randomize");

    EXPECT_THAT(result.found, IsFalse());
    EXPECT_THAT(result.matchedLength, Eq(0));
}

TEST_F(NAME, exact_string)
{
    KeywordDB db;
    db.addKeyword({"projection matrix4", "", {}, false});
    db.addKeyword({"randomize", "", {}, false});
    db.addKeyword({"randomize matrix", "", {}, false});
    db.addKeyword({"randomize mesh", "", {}, false});
    db.addKeyword({"read", "", {}, false});
    matcher->updateFromDB(&db);

    auto result = matcher->findLongestKeywordMatching("randomize");

    EXPECT_THAT(result.found, IsTrue());
    EXPECT_THAT(result.matchedLength, Eq(9));
}

TEST_F(NAME, trailing_space)
{
    KeywordDB db;
    db.addKeyword({"projection matrix4", "", {}, false});
    db.addKeyword({"randomize", "", {}, false});
    db.addKeyword({"randomize matrix", "", {}, false});
    db.addKeyword({"randomize mesh", "", {}, false});
    db.addKeyword({"read", "", {}, false});
    matcher->updateFromDB(&db);

    auto result = matcher->findLongestKeywordMatching("randomize ");

    EXPECT_THAT(result.found, IsTrue());
    EXPECT_THAT(result.matchedLength, Eq(9));
}

TEST_F(NAME, longer_symbol)
{
    KeywordDB db;
    db.addKeyword({"projection matrix4", "", {}, false});
    db.addKeyword({"randomize", "", {}, false});
    db.addKeyword({"randomize matrix", "", {}, false});
    db.addKeyword({"randomize mesh", "", {}, false});
    db.addKeyword({"read", "", {}, false});
    matcher->updateFromDB(&db);

    auto result = matcher->findLongestKeywordMatching("randomized");

    EXPECT_THAT(result.found, IsFalse());
    EXPECT_THAT(result.matchedLength, Eq(9));
}

TEST_F(NAME, match_longer_string_to_shorter_command)
{
    KeywordDB db;
    db.addKeyword({"projection matrix4", "", {}, false});
    db.addKeyword({"randomize", "", {}, false});
    db.addKeyword({"randomize matrix", "", {}, false});
    db.addKeyword({"randomize mesh", "", {}, false});
    db.addKeyword({"read", "", {}, false});
    matcher->updateFromDB(&db);

    auto result = matcher->findLongestKeywordMatching("randomize timer");

    EXPECT_THAT(result.found, IsTrue());
    EXPECT_THAT(result.matchedLength, Eq(9)); // strlen("randomize")
}

TEST_F(NAME, dont_match_shorter_string_to_longer_command)
{
    KeywordDB db;
    db.addKeyword({"dec", "", {}, false});
    matcher->updateFromDB(&db);

    auto result = matcher->findLongestKeywordMatching("decalmax");

    EXPECT_THAT(result.found, IsFalse());
    EXPECT_THAT(result.matchedLength, Eq(3)); // strlen("dec")
}

TEST_F(NAME, match_when_multiple_options_include_spaces_and_non_spaces)
{
    KeywordDB db;
    db.addKeyword({"DELETE OBJECT COLLISION BOX", "", {}, false});
    db.addKeyword({"DELETE OBJECT", "", {}, false});
    db.addKeyword({"DELETE OBJECTS", "", {}, false});
    matcher->updateFromDB(&db);

    auto result = matcher->findLongestKeywordMatching("delete object 100");

    EXPECT_THAT(result.found, IsTrue());
    EXPECT_THAT(result.matchedLength, Eq(strlen("delete object")));
}
