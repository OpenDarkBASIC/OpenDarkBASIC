local dap = require("dap")

dap.set_log_level("TRACE")

dap.adapters.lldb = {
    type = "executable",
    command = "/usr/bin/lldb-dap",
    name = "lldb"
}

dap.configurations.cpp = {
    {
        name        = "Compile DBA",
        type        = "lldb",
        request     = "launch",
        program     = "${workspaceFolder}/build-debug/bin/x86_64/linux/bin/odb-cli",
        cwd         = "${workspaceFolder}",
        stopOnEntry = false,
        args        = function()
            local dba_file = vim.fn.input('Path to DBA: ', vim.fn.getcwd() .. '/', 'file')
            local output_dir = "${workspaceFolder}/build-debug/bin/" .. dba_file .. "/program"
            return { "-b", "-c", "--dba", dba_file, "--output", output_dir, "--exec" }
        end,
    },
    {
        name        = "Compile test.dba",
        type        = "lldb",
        request     = "launch",
        program     = "${workspaceFolder}/build-debug/bin/x86_64/linux/bin/odb-cli",
        cwd         = "${workspaceFolder}",
        stopOnEntry = false,
        args        = { "-b", "-c", "--dba", "${workspaceFolder}/build-debug/bin/test/test2.dba", "--output", "${workspaceFolder}/build-debug/bin/test/test", "--exec" }
    },
    {
        name        = "Compile test.dba AST2",
        type        = "lldb",
        request     = "launch",
        program     = "${workspaceFolder}/build-debug/bin/x86_64/linux/bin/odb-cli",
        cwd         = "${workspaceFolder}",
        stopOnEntry = false,
        args        = { "-b", "-c", "--dba", "${workspaceFolder}/build-debug/bin/test/test2.dba", "--ast2" }
    },
    {
        name        = "Compile test.dba IR",
        type        = "lldb",
        request     = "launch",
        program     = "${workspaceFolder}/build-debug/bin/x86_64/linux/bin/odb-cli",
        cwd         = "${workspaceFolder}",
        stopOnEntry = false,
        args        = { "-b", "-c", "--dba", "${workspaceFolder}/build-debug/bin/test/test2.dba", "--output", "${workspaceFolder}/build-debug/bin/test/test", "--ir" }
    },
    {
        name = "CLI Gen",
        type = "lldb",
        request = "launch",
        program = "${workspaceFolder}/build-debug/bin/hosttools/bin/odb-cligen",
        cwd = "${workspaceFolder}",
        stopOnEntry = false,
        args = { "-i", "${workspaceFolder}/odb-cli/src/args.cli" },
    },
}

dap.configurations.c = dap.configurations.cpp
