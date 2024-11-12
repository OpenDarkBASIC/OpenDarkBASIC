local ls = require("luasnip")
local extras = require("luasnip.extras")
local fmt = require("luasnip.extras.fmt").fmt
local sn = ls.snippet
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
    }
    return special_cases[value:gsub("[%d_]+$", "")] or value
end

ls.add_snippets("c", {
    sn({ trig = "ast", desc = "Access AST node" },
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
    sn({ trig = "asd", desc = "Access AST node" },
        -- In C the variable is usually declared at the top of the function
        fmt("{} = {}->nodes[{}].{}.{};", {
            i(1, "node", { key = "varname" }),
            c(2, { i(1, "ast"), i(2, "(*astp)") }),
            i(3, "parent"),
            c(4, {
                f(function(values) return var_to_node_name(values[1][1]) end, k("varname")),
                i(2, "node"),
            }),
            i(5, "property")
        })
    ),
    sn({ trig = "log", docstring = "Logging function" }, {
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
            return sn({ docstring = "" }, nodes)
        end, { 2 }),
        t(");"),
    }),
    sn({ trig = "asta", docstring = "Assert AST nodes" }, {
        t("ODBUTIL_DEBUG_ASSERT("),
        c(1, {t("ast_node_type"), t("ast_type_info"),}),
        t("("), c(2, { i(1, "ast"), i(2, "*astp") }),
        t(", "), i(3, "node"), t(")"),
        t(" == "),
        f(function(values)
            if values[1][1] == "ast_node_type" then return "AST_" else return "TYPE_" end
        end, {1}),
        i(4, "BLOCK"),
        t(", log_err("),
        f(function(values)
            local func = values[1][1]
            local ast = values[2][1]
            local node = values[3][1]
            return '"type: %d\\n", ' .. func .. "(" .. ast .. ", " .. node .. ")"
        end, { 1, 2, 3 }),
        t("));"),
    }),
}, { key = "OpenDarkBASIC-c" })

ls.add_snippets("cpp", {
    sn({ trig = "ast", desc = "Access AST node" },
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
    sn({ trig = "asd", desc = "Define and access AST node" },
        -- In C++ the variable is usually declared at the same location
        fmt("ast_id {} = {}->nodes[{}].{}.{};", {
            i(1, "node"),
            c(2, { i(1, "ast"), i(2, "(*astp)") }),
            i(3, "node", { key = "varname" }),
            c(4, {
                f(function(values) return var_to_node_name(values[1][1]) end, k("varname")),
                i(2, "node"),
            }),
            i(5, "property")
        })
    ),
}, { key = "OpenDarkBASIC-cpp" })
