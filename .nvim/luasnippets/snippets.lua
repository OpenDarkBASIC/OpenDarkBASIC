return {
    s({ filetype = "cpp", trig = "TEST", desc = "Add a unit test" }, {
        t("TEST_F(NAME, "),
        i(1),
        t(")"),
    }),
}
