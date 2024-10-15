import sys
import os
import subprocess

fname = sys.argv[1]
line_num = int(sys.argv[2])
ast_type = sys.argv[3]

lines = open(fname, "rb").read().decode("utf8").split("\n")

def find_suite_name():
    for line in lines:
        if "#define NAME" in line:
            return line.split(" ")[2]

def find_test_name(current_line):
    while current_line > 1:
        if "TEST(" in lines[current_line - 1] or "TEST_F(" in lines[current_line -1]:
            return lines[current_line - 1].split(",")[1].strip(") ")
        current_line -= 1

suite = find_suite_name()
test = find_test_name(line_num)
ast_path = "./build-debug/bin/x86_64/linux/bin/ast"
dotfile = f"{os.path.join(ast_path, f'{suite}__{test}')}{ast_type}.dot"

subprocess.run(["dot", "-Tx11", dotfile])

