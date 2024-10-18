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

def find_parser_source(current_line):
    while current_line > 1:
        if "TEST(" in lines[current_line - 1] or "TEST_F(" in lines[current_line -1]:
            break
        current_line -= 1
    while current_line < len(lines):
        if "parse(" in lines[current_line] or "const char*" in lines[current_line]:
            break
        current_line += 1
    while current_line < len(lines):
        if '"' in lines[current_line]:
            break;
        current_line += 1
    source = str()
    while '"' in lines[current_line]:
        source += lines[current_line].split('"')[1]
        current_line += 1
    return "\n".join(source.split("\\n"))


source = find_parser_source(line_num)
odb_paths = "./build-debug/bin/x86_64/linux/bin"
subprocess.run([f"./odb-cli -b --dba --ast{ast_type} | dot -Tx11"], shell=True, input=source.encode("utf8"), cwd=odb_path)

#suite = find_suite_name()
#test = find_test_name(line_num)
#ast_path = "./build-debug/bin/x86_64/linux/bin/ast"
#dotfile = f"{os.path.join(ast_path, f'{suite}__{test}')}{ast_type}.dot"
#
#subprocess.run(["dot", "-Tx11", dotfile])

