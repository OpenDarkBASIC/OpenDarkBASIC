local dap = require("dap")

dap.set_log_level("TRACE")

dap.adapters.lldb = {
  type = "executable",
  command = "/usr/bin/lldb-dap-18",
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
    args        = { "-b", "-c", "--dba", "${workspaceFolder}/build-debug/bin/test/test.dba", "--output", "${workspaceFolder}/build-debug/bin/test/test", "--exec" }
  },
  {
    name        = "Compile test.dba AST2",
    type        = "lldb",
    request     = "launch",
    program     = "${workspaceFolder}/build-debug/bin/x86_64/linux/bin/odb-cli",
    cwd         = "${workspaceFolder}",
    stopOnEntry = false,
    args        = { "-b", "-c", "--dba", "${workspaceFolder}/build-debug/bin/test/test.dba", "--ast2" }
  },
  {
    name        = "Compile test.dba IR",
    type        = "lldb",
    request     = "launch",
    program     = "${workspaceFolder}/build-debug/bin/x86_64/linux/bin/odb-cli",
    cwd         = "${workspaceFolder}",
    stopOnEntry = false,
    args        = { "-b", "-c", "--dba", "${workspaceFolder}/build-debug/bin/test/test.dba", "--output", "${workspaceFolder}/build-debug/bin/test/test", "--ir" }
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
  {
    name        = "Unit Tests",
    type        = "lldb",
    request     = "launch",
    program     = "${workspaceFolder}/build-debug/bin/x86_64/linux/bin/odb-tests",
    cwd         = "${workspaceFolder}/build-debug/bin/x86_64/linux/bin",
    stopOnEntry = false,
    args = { "--gtest_filter=odbutil*", }
  },
  {
    name        = "Unit Test under Cursor",
    type        = "lldb",
    request     = "launch",
    program     = "${workspaceFolder}/build-debug/bin/x86_64/linux/bin/odb-tests",
    cwd         = "${workspaceFolder}/build-debug/bin/x86_64/linux/bin",
    stopOnEntry = false,
    args        = function()
      local buf = vim.api.nvim_get_current_buf()
      local cursor = vim.api.nvim_win_get_cursor(0)
      local row = cursor[1]

      local suite = nil
      local test = nil
      for i = row, 1, -1 do
        local line = vim.api.nvim_buf_get_lines(buf, i - 1, i, false)[1]

        local suite_match = line:match("#define%s+NAME%s+(%S+)")
        if suite_match then
          suite = suite_match
        end

        -- TEST or TEST_F
        local test_match = line:match("TEST%s*%(%s*NAME%s*,%s*(%S+)%s*%)")
        local test_f_match = line:match("TEST_F%s*%(%s*NAME%s*,%s*(%S+)%s*%)")
        if test_match then
          test = test_match
        elseif test_f_match then
          test = test_f_match
        end

        if suite and test then
          arg = "--gtest_filter=" .. suite .. "." .. test
          print("Running with " .. arg)
          dap.set_breakpoint()
          return { arg }
        end
      end
      error("No test found")
    end,
  },
}

dap.configurations.c = dap.configurations.cpp
