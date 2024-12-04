local ls = require("luasnip")
local extras = require("luasnip.extras")
local fmt = require("luasnip.extras.fmt").fmt
local s = ls.snippet
local sn = ls.snippet_node
local c = ls.choice_node
local i = ls.insert_node
local r = ls.restore_node
local t = ls.text_node
local f = ls.function_node
local k = require("luasnip.nodes.key_indexer").new_key
local d = ls.dynamic_node
local rep = extras.rep

local function var_to_node_name(value)
    local special_cases = {
        ["ident"] = "identifier",
        ["ass"] = "assignment",
        ["default_case"] = "case_",
        ["decl1"] = "var_decl1",
        ["decl2"] = "var_decl2",
        ["cmd"] = "command",
    }
    return special_cases[value] or value
end

ls.add_snippets("c", {
    s({ trig = "ast", desc = "Access AST node" },
        fmt("{}->nodes[{}].{}.{}", {
            c(1, { i(1, "ast"), i(2, "(*astp)") }),
            i(2, "node", { key = "varname" }),
            c(3, {
                f(function(values) return var_to_node_name(values[1][1]) end, k("varname")),
                i(2, "node"),
            }),
            i(4, "property"),
        })
    ),
    s({ trig = "asd", desc = "Access AST node" },
        c(1, {
            sn(1,
                fmt("{} = {}->nodes[{}].{}.{};", {
                    i(1, "node", { key = "varname1" }),
                    c(2, { i(1, "ast"), i(2, "(*astp)") }),
                    i(3, "parent"),
                    c(4, {
                        f(function(values) return var_to_node_name(values[1][1]) end, k("varname1")),
                        i(2, "node"),
                    }),
                    i(5, "property")
                })
            ),
            sn(2,
                fmt("ast_id {} = {}->nodes[{}].{}.{};", {
                    i(1, "node", { key = "varname2" }),
                    c(2, { i(1, "ast"), i(2, "(*astp)") }),
                    i(3, "parent"),
                    c(4, {
                        f(function(values) return var_to_node_name(values[1][1]) end, k("varname2")),
                        i(2, "node"),
                    }),
                    i(5, "property")
                })
            ),
        })
    ),
    s({ trig = "log", docstring = "Logging function" }, {
        c(1, { i(1, "log_err"), i(2, "log_warn"), i(3, "log_dbg"), i(4, "log_info") }),
        t('("'), i(2, ""), t('\\n"'),
        d(3, function(values)
            local fmt_string = values[1][1]
            local nodes = {}
            for _ in fmt_string:gmatch("%%[^%%]") do
                local idx = #nodes / 2 + 1
                table.insert(nodes, t(", "))
                table.insert(nodes, r(idx, "arg" .. idx, i(nil, "arg" .. idx)))
            end
            return sn(1, nodes)
        end, { 2 }),
        t(");"),
    }),
    s({ trig = "fprintf", docstring = "fprintf" }, {
        t("fprintf("),
        c(1, { i(1, "fp"), i(2, "stderr"), i(3, "stdout") }),
        t(', "'), i(2, ""), t('\\n"'),
        d(3, function(values)
            local fmt_string = values[1][1]
            local nodes = {}
            for _ in fmt_string:gmatch("%%[^%%]") do
                local idx = #nodes / 2 + 1
                table.insert(nodes, t(", "))
                table.insert(nodes, r(idx, "arg" .. idx, i(nil, "arg" .. idx)))
            end
            return sn(1, nodes)
        end, { 2 }),
        t(");"),
    }),
    s({ trig = "odba", docstring = "Assert" }, {
        t("ODBUTIL_DEBUG_ASSERT("),
        c(1, {
            i(1, "stmt"),
            sn(3, { i(1, "stmt"), t(" != NULL") }),
            sn(2, { i(1, "stmt"), t(" > -1") }),
        }),
        t(", "),
        c(2, {
            i(1, "(void)0"),
            sn(2, {
                t('log_err("'),
                i(1, ""), t('\\n"'),
                d(2, function(values)
                    local fmt_string = values[1][1]
                    local nodes = {}
                    for _ in fmt_string:gmatch("%%[^%%]") do
                        local idx = #nodes / 2 + 1
                        table.insert(nodes, t(", "))
                        table.insert(nodes, r(idx, "arg" .. idx, i(nil, "arg" .. idx)))
                    end
                    return sn(1, nodes)
                end, { 1 }),
                t(")"),
            }),
        }),
        t(");"),
    }),
    s({ trig = "asta", docstring = "Assert AST nodes" },
        fmt("ODBUTIL_DEBUG_ASSERT(\n    {}({}, {}) == {}{},\n    log_err({}));", {
            c(1, { t("ast_node_type"), t("ast_type_info"), }),
            c(2, { i(1, "ast"), i(2, "*astp") }),
            i(3, "node"),
            f(function(values)
                if values[1][1] == "ast_node_type" then return "AST_" else return "TYPE_" end
            end, { 1 }),
            i(4, "BLOCK"),
            f(function(values)
                local func = values[1][1]
                local ast = values[2][1]
                local node = values[3][1]
                return '"type: %d\\n", ' .. func .. "(" .. ast .. ", " .. node .. ")"
            end, { 1, 2, 3 }),
        })
    ),
    s({trig = "view", docstring = "Print utf8_view to std format string"}, {
        i(1, "view"),
        f(function(values)
            local name = values[1][1]
            return ".len, " .. name .. ".data + " .. name .. ".off"
        end, {1}),
    }),
}, { key = "OpenDarkBASIC-c" })

ls.add_snippets("cpp", {
    s({ trig = "ast", desc = "Access AST node" },
        fmt("{}->nodes[{}].{}.{};", {
            c(1, { i(1, "ast"), i(2, "(*astp)") }),
            i(2, "node", { key = "varname" }),
            c(3, {
                f(function(values) return var_to_node_name(values[1][1]) end, k("varname")),
                i(2, "node"),
            }),
            i(4, "property"),
        })
    ),
    s({ trig = "asd", desc = "Access AST node" },
        c(1, {
            sn(1,
                fmt("{} = {}->nodes[{}].{}.{};", {
                    i(1, "node", { key = "varname1" }),
                    c(2, { i(1, "ast"), i(2, "(*astp)") }),
                    i(3, "parent"),
                    c(4, {
                        f(function(values) return var_to_node_name(values[1][1]) end, k("varname1")),
                        i(2, "node"),
                    }),
                    i(5, "property")
                })
            ),
            sn(2,
                fmt("ast_id {} = {}->nodes[{}].{}.{};", {
                    i(1, "node", { key = "varname2" }),
                    c(2, { i(1, "ast"), i(2, "(*astp)") }),
                    i(3, "parent"),
                    c(4, {
                        f(function(values) return var_to_node_name(values[1][1]) end, k("varname2")),
                        i(2, "node"),
                    }),
                    i(5, "property")
                })
            ),
        })
    ),
    s({ trig = "odba", docstring = "Assert" }, {
        t("ODBUTIL_DEBUG_ASSERT("),
        c(1, {
            i(1, "stmt"),
            sn(3, { i(1, "stmt"), t(" != NULL") }),
            sn(2, { i(1, "stmt"), t(" > -1") }),
        }),
        t(", "),
        c(2, {
            i(1, "(void)0"),
            sn(2, {
                t('log_err("'),
                i(1, ""), t('\\n"'),
                d(2, function(values)
                    local fmt_string = values[1][1]
                    local nodes = {}
                    for _ in fmt_string:gmatch("%%[^%%]") do
                        local idx = #nodes / 2 + 1
                        table.insert(nodes, t(", "))
                        table.insert(nodes, r(idx, "arg" .. idx, i(nil, "arg" .. idx)))
                    end
                    return sn(1, nodes)
                end, { 1 }),
                t(")"),
            }),
        }),
        t(");"),
    }),
    s({ trig = "asta", docstring = "Assert AST nodes" },
        fmt("ODBUTIL_DEBUG_ASSERT(\n    {}({}, {}) == {}{},\n    log_err({}));", {
            c(1, { t("ast_node_type"), t("ast_type_info"), }),
            c(2, { i(1, "ast"), i(2, "*astp") }),
            i(3, "node"),
            f(function(values)
                if values[1][1] == "ast_node_type" then return "AST_" else return "TYPE_" end
            end, { 1 }),
            i(4, "BLOCK"),
            f(function(values)
                local func = values[1][1]
                local ast = values[2][1]
                local node = values[3][1]
                return '"type: %d\\n", ' .. func .. "(" .. ast .. ", " .. node .. ")"
            end, { 1, 2, 3 }),
        })
    ),
    s({ trig = "log", docstring = "Logging function" }, {
        c(1, { i(1, "log_err"), i(2, "log_warn"), i(3, "log_dbg"), i(4, "log_info") }),
        t('("'), i(2, ""), t('\\n"'),
        d(3, function(values)
            local fmt_string = values[1][1]
            local nodes = {}
            for _ in fmt_string:gmatch("%%[^%%]") do
                local idx = #nodes / 2 + 1
                table.insert(nodes, t(", "))
                table.insert(nodes, r(idx, "arg" .. idx, i(nil, "arg" .. idx)))
            end
            return sn(1, nodes)
        end, { 2 }),
        t(");"),
    }),
    s({trig = "view", docstring = "Print utf8_view to std format string"}, {
        i(1, "view"),
        f(function(values)
            local name = values[1][1]
            return ".len, " .. name .. ".data + " .. name .. ".off"
        end, {1}),
    }),
}, { key = "OpenDarkBASIC-cpp" })
