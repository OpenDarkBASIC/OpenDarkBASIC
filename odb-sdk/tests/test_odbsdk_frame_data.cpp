#include <gmock/gmock.h>
#include "vh/frame_data.h"

#define NAME vh_frame_data

using namespace testing;

TEST(NAME, alloc_structure_works)
{
    struct frame_data fd;
    frame_data_alloc_structure(&fd, 2, 2);
    for (int fighter = 0; fighter != 2; ++fighter)
        for (int frame = 0; frame != 2; ++frame)
        {
            fd.timestamp  [fighter][frame] = 0x100 * fighter + 0x10 * frame + 0x1;
            fd.motion     [fighter][frame] = 0x100 * fighter + 0x10 * frame + 0x2;
            fd.frames_left[fighter][frame] = 0x100 * fighter + 0x10 * frame + 0x3;
            fd.posx       [fighter][frame] = (float)0x100 * fighter + 0x10 * frame + 0x4;
            fd.posy       [fighter][frame] = (float)0x100 * fighter + 0x10 * frame + 0x5;
            fd.damage     [fighter][frame] = (float)0x100 * fighter + 0x10 * frame + 0x6;
            fd.hitstun    [fighter][frame] = (float)0x100 * fighter + 0x10 * frame + 0x7;
            fd.shield     [fighter][frame] = (float)0x100 * fighter + 0x10 * frame + 0x8;
            fd.status     [fighter][frame] = 0x100 * fighter + 0x10 * frame + 0x9;
            fd.hit_status [fighter][frame] = 0x10 * fighter + 0x1 * frame + 0xA;
            fd.stocks     [fighter][frame] = 0x10 * fighter + 0x1 * frame + 0xB;
            fd.flags      [fighter][frame] = 0x10 * fighter + 0x1 * frame + 0xC;
        }

    for (int fighter = 0; fighter != 2; ++fighter)
        for (int frame = 0; frame != 2; ++frame)
        {
            EXPECT_THAT(fd.timestamp  [fighter][frame], Eq(0x100 * fighter + 0x10 * frame + 0x1));
            EXPECT_THAT(fd.motion     [fighter][frame], Eq(0x100 * fighter + 0x10 * frame + 0x2));
            EXPECT_THAT(fd.frames_left[fighter][frame], Eq(0x100 * fighter + 0x10 * frame + 0x3));
            EXPECT_THAT(fd.posx       [fighter][frame], FloatEq((float)0x100 * fighter + 0x10 * frame + 0x4));
            EXPECT_THAT(fd.posy       [fighter][frame], FloatEq((float)0x100 * fighter + 0x10 * frame + 0x5));
            EXPECT_THAT(fd.damage     [fighter][frame], FloatEq((float)0x100 * fighter + 0x10 * frame + 0x6));
            EXPECT_THAT(fd.hitstun    [fighter][frame], FloatEq((float)0x100 * fighter + 0x10 * frame + 0x7));
            EXPECT_THAT(fd.shield     [fighter][frame], FloatEq((float)0x100 * fighter + 0x10 * frame + 0x8));
            EXPECT_THAT(fd.status     [fighter][frame], Eq(0x100 * fighter + 0x10 * frame + 0x9));
            EXPECT_THAT(fd.hit_status [fighter][frame], Eq(0x10 * fighter + 0x1 * frame + 0xA));
            EXPECT_THAT(fd.stocks     [fighter][frame], Eq(0x10 * fighter + 0x1 * frame + 0xB));
            EXPECT_THAT(fd.flags      [fighter][frame], Eq(0x10 * fighter + 0x1 * frame + 0xC));
        }

    frame_data_free(&fd);
}

TEST(NAME, save_and_load_works)
{
    struct frame_data fd;
    frame_data_alloc_structure(&fd, 2, 2);
    for (int fighter = 0; fighter != 2; ++fighter)
        for (int frame = 0; frame != 2; ++frame)
        {
            fd.timestamp  [fighter][frame] = 0x100 * fighter + 0x10 * frame + 0x1;
            fd.motion     [fighter][frame] = 0x100 * fighter + 0x10 * frame + 0x2;
            fd.frames_left[fighter][frame] = 0x100 * fighter + 0x10 * frame + 0x3;
            fd.posx       [fighter][frame] = (float)0x100 * fighter + 0x10 * frame + 0x4;
            fd.posy       [fighter][frame] = (float)0x100 * fighter + 0x10 * frame + 0x5;
            fd.damage     [fighter][frame] = (float)0x100 * fighter + 0x10 * frame + 0x6;
            fd.hitstun    [fighter][frame] = (float)0x100 * fighter + 0x10 * frame + 0x7;
            fd.shield     [fighter][frame] = (float)0x100 * fighter + 0x10 * frame + 0x8;
            fd.status     [fighter][frame] = 0x100 * fighter + 0x10 * frame + 0x9;
            fd.hit_status [fighter][frame] = 0x10 * fighter + 0x1 * frame + 0xA;
            fd.stocks     [fighter][frame] = 0x10 * fighter + 0x1 * frame + 0xB;
            fd.flags      [fighter][frame] = 0x10 * fighter + 0x1 * frame + 0xC;
        }
    ASSERT_THAT(frame_data_save(&fd, 0), Eq(0));
    frame_data_free(&fd);

    ASSERT_THAT(frame_data_load(&fd, 0), Eq(0));
    for (int fighter = 0; fighter != 2; ++fighter)
        for (int frame = 0; frame != 2; ++frame)
        {
            EXPECT_THAT(fd.timestamp  [fighter][frame], Eq(0x100 * fighter + 0x10 * frame + 0x1));
            EXPECT_THAT(fd.motion     [fighter][frame], Eq(0x100 * fighter + 0x10 * frame + 0x2));
            EXPECT_THAT(fd.frames_left[fighter][frame], Eq(0x100 * fighter + 0x10 * frame + 0x3));
            EXPECT_THAT(fd.posx       [fighter][frame], FloatEq((float)0x100 * fighter + 0x10 * frame + 0x4));
            EXPECT_THAT(fd.posy       [fighter][frame], FloatEq((float)0x100 * fighter + 0x10 * frame + 0x5));
            EXPECT_THAT(fd.damage     [fighter][frame], FloatEq((float)0x100 * fighter + 0x10 * frame + 0x6));
            EXPECT_THAT(fd.hitstun    [fighter][frame], FloatEq((float)0x100 * fighter + 0x10 * frame + 0x7));
            EXPECT_THAT(fd.shield     [fighter][frame], FloatEq((float)0x100 * fighter + 0x10 * frame + 0x8));
            EXPECT_THAT(fd.status     [fighter][frame], Eq(0x100 * fighter + 0x10 * frame + 0x9));
            EXPECT_THAT(fd.hit_status [fighter][frame], Eq(0x10 * fighter + 0x1 * frame + 0xA));
            EXPECT_THAT(fd.stocks     [fighter][frame], Eq(0x10 * fighter + 0x1 * frame + 0xB));
            EXPECT_THAT(fd.flags      [fighter][frame], Eq(0x10 * fighter + 0x1 * frame + 0xC));
        }

    frame_data_free(&fd);
}
