#include "gmock/gmock.h"
#include "vh/db.h"

#define NAME vh_db

using namespace testing;

struct NAME : Test
{
    void SetUp() override
    {
        dbi = ::db("sqlite3");
        db = dbi->open("test.db");
        dbi->reinit(db);
    }

    void TearDown() override
    {
        dbi->close(db);
    }

    struct db_interface* dbi;
    struct db* db;
};

TEST_F(NAME, motions)
{
    EXPECT_THAT(dbi->motion.add(db, 0x1234, cstr_view("test")), Eq(0));
    EXPECT_THAT(dbi->motion.add(db, 0x1235, cstr_view("foo")), Eq(0));
    EXPECT_THAT(dbi->motion.exists(db, 0x1234), IsTrue());
    EXPECT_THAT(dbi->motion.exists(db, 0x1235), IsTrue());
    EXPECT_THAT(dbi->motion.exists(db, 0x1236), IsFalse());
}

TEST_F(NAME, duplicate_games_1v1)
{
    int round_type_id = dbi->round.add_or_get_type(db, cstr_view("WR"), cstr_view("Winner's Round"));

    int set_format_id = dbi->set_format.add_or_get(db, cstr_view("Bo3"), cstr_view("Best of 3"));

    int p1_id = dbi->person.add_or_get(db, -1, cstr_view("p1"), cstr_view("p1"), cstr_view(""), cstr_view(""));
    int p2_id = dbi->person.add_or_get(db, -1, cstr_view("p2"), cstr_view("p2"), cstr_view(""), cstr_view(""));

    int team1_id = dbi->team.add_or_get(db, cstr_view("p1"), cstr_view(""));
    dbi->team.add_member(db, team1_id, p1_id);
    int team2_id = dbi->team.add_or_get(db, cstr_view("p2"), cstr_view(""));
    dbi->team.add_member(db, team2_id, p2_id);
    int winner_team_id = team1_id;

    int round_number = 1;
    int stage_id = 3;
    uint64_t time_started = 1600000000;
    int duration = 100;

    EXPECT_THAT(dbi->game.exists_1v1(db, p1_id, p2_id, time_started), IsFalse());

    int game_id = dbi->game.add(db, round_type_id, round_number, set_format_id, winner_team_id, stage_id, time_started, duration);
    dbi->game.add_player(db, p1_id, game_id, 0, team1_id, 8, 0, 0);
    dbi->game.add_player(db, p2_id, game_id, 0, team2_id, 8, 0, 0);

    EXPECT_THAT(dbi->game.exists_1v1(db, p1_id, p2_id, time_started), IsTrue());

    /* The issue occurs when adding another player with the same name, but a different in-game tag */
    p1_id = dbi->person.add_or_get(db, -1, cstr_view("p1"), cstr_view("another tag"), cstr_view(""), cstr_view(""));

    EXPECT_THAT(dbi->game.exists_1v1(db, p1_id, p2_id, time_started), IsTrue());
}
