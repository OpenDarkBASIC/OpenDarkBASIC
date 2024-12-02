local dap = require("dap")
local last_dba = vim.fn.getcwd() .. '/dba-sources/'

dap.set_log_level("TRACE")

dap.adapters.lldb = {
    type = "executable",
    command = "/usr/bin/lldb-dap",
    name = "lldb"
}

dap.configurations.cpp = {
    {
        name        = "Unit Tests",
        type        = "lldb",
        request     = "launch",
        program     = "${workspaceFolder}/build-Debug/bin/x86_64/linux/bin/odb-tests",
        cwd         = "${workspaceFolder}/build-Debug/bin/x86_64/linux/bin",
        stopOnEntry = false,
    },
    {
        name        = "Unit Tests current Suite",
        type        = "lldb",
        request     = "launch",
        program     = "${workspaceFolder}/build-Debug/bin/x86_64/linux/bin/odb-tests",
        cwd         = "${workspaceFolder}/build-Debug/bin/x86_64/linux/bin",
        stopOnEntry = false,
        args        = function()
            local buf = vim.api.nvim_get_current_buf()
            local cursor = vim.api.nvim_win_get_cursor(0)
            local row = cursor[1]

            for i = row, 1, -1 do
                local line = vim.api.nvim_buf_get_lines(buf, i - 1, i, false)[1]
                local suite = line:match("#define%s+NAME%s+(%S+)")
                if suite then
                    arg = "--gtest_filter=" .. suite .. ".*"
                    print("Running with " .. arg)
                    return { arg }
                end
            end
            error("No test found")
        end,
    },
    {
        name        = "Unit Test under Cursor",
        type        = "lldb",
        request     = "launch",
        program     = "${workspaceFolder}/build-Debug/bin/x86_64/linux/bin/odb-tests",
        cwd         = "${workspaceFolder}/build-Debug/bin/x86_64/linux/bin",
        stopOnEntry = false,
        args        = function()
            local buf = vim.api.nvim_get_current_buf()
            local cursor = vim.api.nvim_win_get_cursor(0)
            local row = cursor[1]

            local test = nil
            for i = row, 1, -1 do
                -- TEST or TEST_F
                local line = vim.api.nvim_buf_get_lines(buf, i - 1, i, false)[1]
                test = line:match("TEST%s*%(%s*NAME%s*,%s*(%S+)%s*%)")
                if test then
                    break
                end
                test = line:match("TEST_F%s*%(%s*NAME%s*,%s*(%S+)%s*%)")
                if test then
                    break
                end
            end

            local suite = nil
            for i = row, 1, -1 do
                local line = vim.api.nvim_buf_get_lines(buf, i - 1, i, false)[1]
                suite = line:match("#define%s+NAME%s+(%S+)")
                if suite then
                    break
                end
            end

            if suite and test then
                arg = "--gtest_filter=" .. suite .. "." .. test
                print("Running with " .. arg)
                --dap.set_breakpoint()
                return { arg }
            end

            error("No test found")
        end,
    },
    {
        name        = "Run DBA",
        type        = "lldb",
        request     = "launch",
        program     = "${workspaceFolder}/build-Debug/bin/x86_64/linux/bin/odb-cli",
        cwd         = "${workspaceFolder}/build-Debug/bin",
        stopOnEntry = false,
        args        = function()
            local current_fname = vim.api.nvim_buf_get_name(0)
            if current_fname:match(".dba$") then
                last_dba = current_fname
            end
            local dba_file = vim.fn.input('Path to DBA: ', last_dba, 'file')
            local output = "${workspaceFolder}/build-Debug/bin/" .. vim.fs.basename(dba_file) .. ".exe"
            return { "-b", "-c", "--dba", dba_file, "--output", output, "--exec" }
        end,
    },
    {
        name        = "Run playground DBA",
        type        = "lldb",
        request     = "launch",
        program     = "${workspaceFolder}/build-Debug/bin/x86_64/linux/bin/odb-cli",
        cwd         = "${workspaceFolder}/build-Debug/bin/playground",
        stopOnEntry = false,
        args        = function()
            local extra_args = vim.fn.input('Extra args: ')
            local args = {
                "-b",
                "--dba",
                "${workspaceFolder}/build-Debug/bin/playground/playground.dba",
                "--output",
                "${workspaceFolder}/build-Debug/bin/playground/playground",
                "--exec",
            }
            if #extra_args > 0 then
                vim.list_extend(args, vim.split(extra_args, "%s+"))
            end
            return args
        end,
    },
    {
        name = "CLI Gen",
        type = "lldb",
        request = "launch",
        program = "${workspaceFolder}/build-Debug/bin/hosttools/bin/odb-cligen",
        cwd = "${workspaceFolder}",
        stopOnEntry = false,
        args = { "-i", "${workspaceFolder}/odb-cli/src/args.cli" },
    },
    {
        name = "asttool",
        type = "lldb",
        request = "launch",
        program = "${workspaceFolder}/build-Debug/bin/x86_64/linux/bin/odb-asttool",
        cwd = "${workspaceFolder}/build-Debug/bin/x86_64/linux/bin",
        stopOnEntry = false,
        args = { "-i", "playground.dba.ast", "--types" },
    },
}

dap.configurations.c = dap.configurations.cpp

dap.configurations.basic = {
    {
        name        = "Run DBA",
        type        = "lldb",
        request     = "launch",
        program     = "${workspaceFolder}/build-Debug/bin/x86_64/linux/bin/odb-cli",
        cwd         = "${workspaceFolder}/build-Debug/bin",
        stopOnEntry = false,
        args        = function()
            local current_fname = vim.api.nvim_buf_get_name(0)
            if current_fname:match(".dba$") then
                last_dba = current_fname
            end
            local dba_file = vim.fn.input('Path to DBA: ', last_dba, 'file')
            local output = "${workspaceFolder}/build-Debug/bin/" .. vim.fs.basename(dba_file) .. ".exe"
            return { "-b", "-c", "--dba", dba_file, "--output", output, "--exec" }
        end,
    },
}
