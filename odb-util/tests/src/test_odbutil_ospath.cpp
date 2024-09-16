extern "C" {
#include "odb-util/ospath.h"
}

#include <gmock/gmock.h>
#include <string_view>

#define NAME odbutil_ospath

using namespace testing;

struct NAME : public Test
{
    void SetUp() override
    {
        path = empty_ospath();
    }
    void TearDown() override
    {
        ospath_deinit(path);
    }

    struct ospath path;
};

/*
class OsPathCEqMatcher : public testing::MatcherInterface<struct ospathc>
{
public:
    explicit StrViewEqMatcher(const std::string& expected)
        : expected(expected)
    {}

    bool MatchAndExplain(const struct str_view actual, testing::MatchResultListener* listener) const override
    {
        *listener << "\"" << std::string_view(actual.data, actual.len) << "\"";
        return cstr_equal(actual, expected.c_str());
    }
    void DescribeTo(std::ostream* os) const override
    {
        *os << "Equals \"" << expected << "\"";
    }
    void DescribeNegationTo(std::ostream* os) const override
    {
        *os << "Does not equal \"" << expected << "\"";
    }

private:
    const std::string expected;
};

inline testing::Matcher<struct str_view> StrViewEq(const std::string& expected)
{
    return MakeMatcher(new StrViewEqMatcher(expected));
}
*/

TEST_F(NAME, set_empty)
{
    ospath_set(&path, cstr_ospathc(""));
    EXPECT_THAT(ospath_cstr(path), StrEq(""));
}

TEST_F(NAME, set_replace_slashes)
{
#ifdef _WIN32
    ospath_set(&path, cstr_ospathc("some/unix/path"));
    EXPECT_THAT(ospath_cstr(path), StrEq("some\\unix\\path"));
#else
    ospath_set(&path, cstr_ospathc("some\\windows\\path"));
    EXPECT_THAT(ospath_cstr(path), StrEq("some/windows/path"));
#endif
}

TEST_F(NAME, set_remove_trailing_slashes)
{
#ifdef _WIN32
    ospath_set(&path, cstr_ospathc("some/unix/path/"));
    EXPECT_THAT(ospath_cstr(path), StrEq("some\\unix\\path"));
#else
    ospath_set(&path, cstr_ospathc("some\\windows\\path\\"));
    EXPECT_THAT(ospath_cstr(path), StrEq("some/windows/path"));
#endif
}

TEST_F(NAME, set_dont_remove_root_slash)
{
#ifdef _WIN32
    ospath_set(&path, cstr_ospathc("/"));
    EXPECT_THAT(ospath_cstr(path), StrEq(""));
#else
    ospath_set(&path, cstr_ospathc("\\"));
    EXPECT_THAT(ospath_cstr(path), StrEq("/"));
#endif
}

TEST_F(NAME, join_empty)
{
    ospath_set(&path, cstr_ospathc(""));
    ospath_join(&path, cstr_ospathc(""));
    EXPECT_THAT(ospath_cstr(path), StrEq(""));
}

TEST_F(NAME, join_1)
{
#ifdef _WIN32
    ospath_set(&path, cstr_ospathc("some/"));
    ospath_join(&path, cstr_ospathc("unix/path/"));
    EXPECT_THAT(ospath_cstr(path), StrEq("some\\unix\\path"));
#else
    ospath_set(&path, cstr_ospathc("some\\"));
    ospath_join(&path, cstr_ospathc("windows\\path\\"));
    EXPECT_THAT(ospath_cstr(path), StrEq("some/windows/path"));
#endif
}

TEST_F(NAME, join_2)
{
#ifdef _WIN32
    ospath_set(&path, cstr_ospathc("some"));
    ospath_join(&path, cstr_ospathc("unix/path"));
    EXPECT_THAT(ospath_cstr(path), StrEq("some\\unix\\path"));
#else
    ospath_set(&path, cstr_ospathc("some"));
    ospath_join(&path, cstr_ospathc("windows\\path"));
    EXPECT_THAT(ospath_cstr(path), StrEq("some/windows/path"));
#endif
}

TEST_F(NAME, dirname_empty)
{
    ospath_set(&path, cstr_ospathc(""));
    ospath_dirname(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("."));
}

TEST_F(NAME, dirname_file_1)
{
    ospath_set(&path, cstr_ospathc("file.dat.xz"));
    ospath_dirname(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("."));
}

TEST_F(NAME, dirname_on_file_2)
{
#ifdef _WIN32
    ospath_set(&path, cstr_ospathc("/file.dat.xz"));
    ospath_dirname(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq(""));
#else
    ospath_set(&path, cstr_ospathc("\\file.dat.xz"));
    ospath_dirname(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("/"));
#endif
}

TEST_F(NAME, dirname_on_file_3)
{
#ifdef _WIN32
    ospath_set(&path, cstr_ospathc("some/unix/file.dat.xz"));
    ospath_dirname(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("some\\unix"));
#else
    ospath_set(&path, cstr_ospathc("some\\windows\\file.dat.xz"));
    ospath_dirname(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("some/windows"));
#endif
}

TEST_F(NAME, dirname_on_path_1)
{
#ifdef _WIN32
    ospath_set(&path, cstr_ospathc("some/unix/path"));
    ospath_dirname(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("some\\unix"));
#else
    ospath_set(&path, cstr_ospathc("some\\windows\\path"));
    ospath_dirname(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("some/windows"));
#endif
}

TEST_F(NAME, dirname_on_path_2)
{
#ifdef _WIN32
    ospath_set(&path, cstr_ospathc("some/unix/path/"));
    ospath_dirname(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("some\\unix"));
#else
    ospath_set(&path, cstr_ospathc("some\\windows\\path\\"));
    ospath_dirname(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("some/windows"));
#endif
}

TEST_F(NAME, dirname_on_path_3)
{
#ifdef _WIN32
    ospath_set(&path, cstr_ospathc("/some/unix/path"));
    ospath_dirname(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq(""));
#else
    ospath_set(&path, cstr_ospathc("\\some\\windows\\path"));
    ospath_dirname(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("/some/windows"));
#endif
}

TEST_F(NAME, dirname_on_path_4)
{
    ospath_set(&path, cstr_ospathc("dir"));
    ospath_dirname(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("."));
}

TEST_F(NAME, dirname_on_path_5)
{
#ifdef _WIN32
    ospath_set(&path, cstr_ospathc("/"));
    ospath_dirname(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("."));
#else
    ospath_set(&path, cstr_ospathc("\\"));
    ospath_dirname(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("/"));
#endif
}

#if 0
TEST_F(NAME, path_basename_empty)
{
    ospath_set(&path, cstr_ospathc(""));
    path_basename(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq(""));
}

TEST_F(NAME, path_basename_file_1)
{
    ospath_set(&path, cstr_ospathc("file.dat.xz"));
    path_basename(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("file.dat.xz"));
}

TEST_F(NAME, path_basename_on_file_2)
{
#ifdef _WIN32
    ospath_set(&path, cstr_ospathc("/file.dat.xz"));
    path_basename(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq(""));  /* Paths starting with "\" are invalid on windows */
#else
    ospath_set(&path, cstr_ospathc("\\file.dat.xz"));
    path_basename(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("file.dat.xz"));
#endif
}

TEST_F(NAME, path_basename_on_file_3)
{
#ifdef _WIN32
    ospath_set(&path, cstr_ospathc("some/unix/file.dat.xz"));
    path_basename(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("file.dat.xz"));
#else
    ospath_set(&path, cstr_ospathc("some\\windows\\file.dat.xz"));
    path_basename(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("file.dat.xz"));
#endif
}

TEST_F(NAME, path_basename_on_path_1)
{
#ifdef _WIN32
    ospath_set(&path, cstr_ospathc("some/unix/path"));
    path_basename(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("path"));
#else
    ospath_set(&path, cstr_ospathc("some\\windows\\path"));
    path_basename(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("path"));
#endif
}

TEST_F(NAME, path_basename_on_path_2)
{
#ifdef _WIN32
    ospath_set(&path, cstr_ospathc("some/unix/path/"));
    path_basename(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("path"));
#else
    ospath_set(&path, cstr_ospathc("some\\windows\\path\\"));
    path_basename(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("path"));
#endif
}

TEST_F(NAME, path_basename_on_path_3)
{
#ifdef _WIN32
    ospath_set(&path, cstr_ospathc("/some/unix/path"));
    path_basename(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq(""));  /* paths starting with "\" are invalid on windows */
#else
    ospath_set(&path, cstr_ospathc("\\some\\windows\\path"));
    path_basename(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("path"));
#endif
}

TEST_F(NAME, path_basename_on_path_4)
{
    ospath_set(&path, cstr_ospathc("dir"));
    path_basename(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("dir"));
}

TEST_F(NAME, path_basename_on_path_5)
{
#ifdef _WIN32
    ospath_set(&path, cstr_ospathc("/"));
    path_basename(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq(""));  /* paths starting with "\" are invalid on windows */
#else
    ospath_set(&path, cstr_ospathc("\\"));
    path_basename(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("/"));
#endif
}

TEST_F(NAME, cpath_basename_empty)
{
    struct str_view path = cpath_basename_view("");
    EXPECT_THAT(path, StrViewEq(""));
}

TEST_F(NAME, cpath_basename_file_1)
{
    struct str_view path = cpath_basename_view("file.dat.xz");
    EXPECT_THAT(path, StrViewEq("file.dat.xz"));
}

TEST_F(NAME, cpath_basename_on_file_2)
{
#ifdef _WIN32
    struct str_view path = cpath_basename_view("/file.dat.xz");
    EXPECT_THAT(path, StrViewEq(""));  /* Paths starting with "\" are invalid on windows */
#else
    struct str_view path = cpath_basename_view("\\file.dat.xz");
    EXPECT_THAT(path, StrViewEq("file.dat.xz"));
#endif
}

TEST_F(NAME, cpath_basename_on_file_3)
{
#ifdef _WIN32
    struct str_view path = cpath_basename_view("some/unix/file.dat.xz");
    EXPECT_THAT(path, StrViewEq("file.dat.xz"));
#else
    struct str_view path = cpath_basename_view("some\\windows\\file.dat.xz");
    EXPECT_THAT(path, StrViewEq("file.dat.xz"));
#endif
}

TEST_F(NAME, cpath_basename_on_path_1)
{
#ifdef _WIN32
    struct str_view path = cpath_basename_view("some/unix/path");
    EXPECT_THAT(path, StrViewEq("path"));
#else
    struct str_view path = cpath_basename_view("some\\windows\\path");
    EXPECT_THAT(path, StrViewEq("path"));
#endif
}

TEST_F(NAME, cpath_basename_on_path_2)
{
#ifdef _WIN32
    struct str_view path = cpath_basename_view("some/unix/path/");
    EXPECT_THAT(path, StrViewEq("path"));
#else
    struct str_view path = cpath_basename_view("some\\windows\\path\\");
    EXPECT_THAT(path, StrViewEq("path"));
#endif
}

TEST_F(NAME, cpath_basename_on_path_3)
{
#ifdef _WIN32
    struct str_view path = cpath_basename_view("/some/unix/path");
    EXPECT_THAT(path, StrViewEq(""));  /* paths starting with "\" are invalid on windows */
#else
    struct str_view path = cpath_basename_view("\\some\\windows\\path");
    EXPECT_THAT(path, StrViewEq("path"));
#endif
}

TEST_F(NAME, cpath_basename_on_path_4)
{
    struct str_view path = cpath_basename_view("dir");
    EXPECT_THAT(path, StrViewEq("dir"));
}

TEST_F(NAME, cpath_basename_on_path_5)
{
#ifdef _WIN32
    struct str_view path = cpath_basename_view("\\");
    EXPECT_THAT(path, StrViewEq(""));  /* paths starting with "\" are invalid on windows */
#else
    struct str_view path = cpath_basename_view("/");
    EXPECT_THAT(path, StrViewEq("/"));
#endif
}
#endif
