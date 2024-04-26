extern "C" {
#include "odb-sdk/fs.h"
}

#include <gmock/gmock.h>
#include <string_view>

#define NAME vh_fs

using namespace testing;

struct NAME : public Test
{
    void SetUp() override
    {
        path_init(&path);
    }
    void TearDown() override
    {
        path_deinit(&path);
    }

    struct path path;
};

class StrViewEqMatcher : public testing::MatcherInterface<struct str_view>
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

TEST_F(NAME, path_set_empty)
{
    path_set(&path, cstr_view(""));
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq(""));
}

TEST_F(NAME, path_set_replace_slashes)
{
#ifdef _WIN32
    path_set(&path, cstr_view("some/unix/path"));
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("some\\unix\\path"));
#else
    path_set(&path, cstr_view("some\\windows\\path"));
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("some/windows/path"));
#endif
}

TEST_F(NAME, path_set_remove_trailing_slashes)
{
#ifdef _WIN32
    path_set(&path, cstr_view("some/unix/path/"));
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("some\\unix\\path"));
#else
    path_set(&path, cstr_view("some\\windows\\path\\"));
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("some/windows/path"));
#endif
}

TEST_F(NAME, path_set_dont_remove_root_slash)
{
#ifdef _WIN32
    path_set(&path, cstr_view("/"));
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq(""));
#else
    path_set(&path, cstr_view("\\"));
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("/"));
#endif
}

TEST_F(NAME, path_join_empty)
{
    path_set(&path, cstr_view(""));
    path_join(&path, cstr_view(""));
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq(""));
}

TEST_F(NAME, path_join_1)
{
#ifdef _WIN32
    path_set(&path, cstr_view("some/"));
    path_join(&path, cstr_view("unix/path/"));
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("some\\unix\\path"));
#else
    path_set(&path, cstr_view("some\\"));
    path_join(&path, cstr_view("windows\\path\\"));
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("some/windows/path"));
#endif
}

TEST_F(NAME, path_join_2)
{
#ifdef _WIN32
    path_set(&path, cstr_view("some"));
    path_join(&path, cstr_view("unix/path"));
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("some\\unix\\path"));
#else
    path_set(&path, cstr_view("some"));
    path_join(&path, cstr_view("windows\\path"));
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("some/windows/path"));
#endif
}

TEST_F(NAME, path_dirname_empty)
{
    path_set(&path, cstr_view(""));
    path_dirname(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq(""));
}

TEST_F(NAME, path_dirname_file_1)
{
    path_set(&path, cstr_view("file.dat.xz"));
    path_dirname(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq(""));
}

TEST_F(NAME, path_dirname_on_file_2)
{
#ifdef _WIN32
    path_set(&path, cstr_view("/file.dat.xz"));
    path_dirname(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq(""));
#else
    path_set(&path, cstr_view("\\file.dat.xz"));
    path_dirname(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("/"));
#endif
}

TEST_F(NAME, path_dirname_on_file_3)
{
#ifdef _WIN32
    path_set(&path, cstr_view("some/unix/file.dat.xz"));
    path_dirname(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("some\\unix"));
#else
    path_set(&path, cstr_view("some\\windows\\file.dat.xz"));
    path_dirname(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("some/windows"));
#endif
}

TEST_F(NAME, path_dirname_on_path_1)
{
#ifdef _WIN32
    path_set(&path, cstr_view("some/unix/path"));
    path_dirname(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("some\\unix"));
#else
    path_set(&path, cstr_view("some\\windows\\path"));
    path_dirname(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("some/windows"));
#endif
}

TEST_F(NAME, path_dirname_on_path_2)
{
#ifdef _WIN32
    path_set(&path, cstr_view("some/unix/path/"));
    path_dirname(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("some\\unix"));
#else
    path_set(&path, cstr_view("some\\windows\\path\\"));
    path_dirname(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("some/windows"));
#endif
}

TEST_F(NAME, path_dirname_on_path_3)
{
#ifdef _WIN32
    path_set(&path, cstr_view("/some/unix/path"));
    path_dirname(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq(""));
#else
    path_set(&path, cstr_view("\\some\\windows\\path"));
    path_dirname(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("/some/windows"));
#endif
}

TEST_F(NAME, path_dirname_on_path_4)
{
    path_set(&path, cstr_view("dir"));
    path_dirname(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq(""));
}

TEST_F(NAME, path_dirname_on_path_5)
{
#ifdef _WIN32
    path_set(&path, cstr_view("/"));
    path_dirname(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq(""));
#else
    path_set(&path, cstr_view("\\"));
    path_dirname(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("/"));
#endif
}

TEST_F(NAME, path_basename_empty)
{
    path_set(&path, cstr_view(""));
    path_basename(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq(""));
}

TEST_F(NAME, path_basename_file_1)
{
    path_set(&path, cstr_view("file.dat.xz"));
    path_basename(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("file.dat.xz"));
}

TEST_F(NAME, path_basename_on_file_2)
{
#ifdef _WIN32
    path_set(&path, cstr_view("/file.dat.xz"));
    path_basename(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq(""));  /* Paths starting with "\" are invalid on windows */
#else
    path_set(&path, cstr_view("\\file.dat.xz"));
    path_basename(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("file.dat.xz"));
#endif
}

TEST_F(NAME, path_basename_on_file_3)
{
#ifdef _WIN32
    path_set(&path, cstr_view("some/unix/file.dat.xz"));
    path_basename(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("file.dat.xz"));
#else
    path_set(&path, cstr_view("some\\windows\\file.dat.xz"));
    path_basename(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("file.dat.xz"));
#endif
}

TEST_F(NAME, path_basename_on_path_1)
{
#ifdef _WIN32
    path_set(&path, cstr_view("some/unix/path"));
    path_basename(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("path"));
#else
    path_set(&path, cstr_view("some\\windows\\path"));
    path_basename(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("path"));
#endif
}

TEST_F(NAME, path_basename_on_path_2)
{
#ifdef _WIN32
    path_set(&path, cstr_view("some/unix/path/"));
    path_basename(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("path"));
#else
    path_set(&path, cstr_view("some\\windows\\path\\"));
    path_basename(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("path"));
#endif
}

TEST_F(NAME, path_basename_on_path_3)
{
#ifdef _WIN32
    path_set(&path, cstr_view("/some/unix/path"));
    path_basename(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq(""));  /* paths starting with "\" are invalid on windows */
#else
    path_set(&path, cstr_view("\\some\\windows\\path"));
    path_basename(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("path"));
#endif
}

TEST_F(NAME, path_basename_on_path_4)
{
    path_set(&path, cstr_view("dir"));
    path_basename(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("dir"));
}

TEST_F(NAME, path_basename_on_path_5)
{
#ifdef _WIN32
    path_set(&path, cstr_view("/"));
    path_basename(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq(""));  /* paths starting with "\" are invalid on windows */
#else
    path_set(&path, cstr_view("\\"));
    path_basename(&path);
    path_terminate(&path);
    EXPECT_THAT(path.str.data, StrEq("/"));
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
