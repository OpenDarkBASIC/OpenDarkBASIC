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

TEST_F(NAME, dirname_on_root)
{
#ifdef _WIN32
    ospath_set(&path, cstr_ospathc("/"));
    ospath_dirname(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq(""));  /* Paths starting with "\" are invalid on windows */
#else
    ospath_set(&path, cstr_ospathc("\\"));
    ospath_dirname(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("/"));
#endif
}

TEST_F(NAME, dirname_on_current_dir)
{
    ospath_set(&path, cstr_ospathc("."));
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

TEST_F(NAME, filename_empty)
{
    ospath_set(&path, cstr_ospathc(""));
    ospath_filename(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq(""));
}

TEST_F(NAME, filename_on_root)
{
#ifdef _WIN32
    ospath_set(&path, cstr_ospathc("/"));
    ospath_filename(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq(""));
#else
    ospath_set(&path, cstr_ospathc("\\"));
    ospath_filename(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq(""));
#endif
}

TEST_F(NAME, filename_1)
{
    ospath_set(&path, cstr_ospathc("file.dat.xz"));
    ospath_filename(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("file.dat.xz"));
}

TEST_F(NAME, filename_2)
{
#ifdef _WIN32
    ospath_set(&path, cstr_ospathc("/file.dat.xz"));
    ospath_filename(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq(""));
#else
    ospath_set(&path, cstr_ospathc("\\file.dat.xz"));
    ospath_filename(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("file.dat.xz"));
#endif
}

TEST_F(NAME, filename_3)
{
#ifdef _WIN32
    ospath_set(&path, cstr_ospathc("some/unix/file.dat.xz"));
    ospath_filename(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("filename.dat.xz"));
#else
    ospath_set(&path, cstr_ospathc("some\\windows\\file.dat.xz"));
    ospath_filename(&path);
    EXPECT_THAT(ospath_cstr(path), StrEq("file.dat.xz"));
#endif
}

TEST_F(NAME, c_filename_on_root)
{
#ifdef _WIN32
    struct ospathc cpath = cstr_ospathc("/");
    ospathc_filename(&cpath);
    EXPECT_THAT(ospathc_cstr(cpath), StrEq(""));
#else
    struct ospathc cpath = cstr_ospathc("\\");
    ospathc_filename(&cpath);
    EXPECT_THAT(ospathc_cstr(cpath), StrEq(""));
#endif
}

TEST_F(NAME, c_filename_1)
{
    struct ospathc cpath = cstr_ospathc("file.dat.xz");
    ospathc_filename(&cpath);
    EXPECT_THAT(ospathc_cstr(cpath), StrEq("file.dat.xz"));
}

TEST_F(NAME, c_filename_2)
{
#ifdef _WIN32
    struct ospathc cpath = cstr_ospathc("/file.dat.xz");
    ospathc_filename(&cpath);
    EXPECT_THAT(ospathc_cstr(cpath), StrEq(""));
#else
    struct ospathc cpath = cstr_ospathc("\\file.dat.xz");
    ospathc_filename(&cpath);
    EXPECT_THAT(ospathc_cstr(cpath), StrEq("file.dat.xz"));
#endif
}

TEST_F(NAME, c_filename_3)
{
#ifdef _WIN32
    struct ospathc cpath = cstr_ospathc("some/unix/file.dat.xz");
    ospathc_filename(&cpath);
    EXPECT_THAT(ospath_cstr(path), StrEq("filename.dat.xz"));
#else
    struct ospathc cpath = cstr_ospathc("some\\windows\\file.dat.xz");
    ospathc_filename(&cpath);
    EXPECT_THAT(ospathc_cstr(cpath), StrEq("file.dat.xz"));
#endif
}

