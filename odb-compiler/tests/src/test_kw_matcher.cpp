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
    int matchedLen = 5;
    bool matchFound = matcher->findLongestKeywordMatching("randomize", &matchedLen);

    EXPECT_THAT(matchFound, IsFalse());
    EXPECT_THAT(matchedLen, Eq(0));
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

    int matchedLen = 5;
    bool matchFound = matcher->findLongestKeywordMatching("randomize", &matchedLen);

    EXPECT_THAT(matchFound, IsTrue());
    EXPECT_THAT(matchedLen, Eq(9));
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

    int matchedLen;
    bool matchFound = matcher->findLongestKeywordMatching("randomize timer", &matchedLen);

    EXPECT_THAT(matchFound, IsTrue());
    EXPECT_THAT(matchedLen, Eq(9)); // strlen("randomize")
}

TEST_F(NAME, dont_match_shorter_string_to_longer_command)
{
    KeywordDB db;
    db.addKeyword({"dec", "", {}, false});
    matcher->updateFromDB(&db);

    int matchedLen;
    bool matchFound = matcher->findLongestKeywordMatching("decalmax", &matchedLen);

    EXPECT_THAT(matchFound, IsFalse());
    EXPECT_THAT(matchedLen, Eq(3)); // strlen("dec")
}

TEST_F(NAME, match_when_multiple_options_include_spaces_and_non_spaces)
{
    KeywordDB db;
    db.addKeyword({"DELETE OBJECT COLLISION BOX", "", {}, false});
    db.addKeyword({"DELETE OBJECT", "", {}, false});
    db.addKeyword({"DELETE OBJECTS", "", {}, false});
    matcher->updateFromDB(&db);

    int matchedLen;
    bool matchFound = matcher->findLongestKeywordMatching("delete object 100", &matchedLen);

    EXPECT_THAT(matchFound, IsTrue());
    EXPECT_THAT(matchedLen, Eq(strlen("delete object")));
}
