import sys
import os
import subprocess

odb_path = "./build-Debug/bin/x86_64/linux/bin"
asttool_args = "--scopes --types"
fname = sys.argv[1]
line_num = int(sys.argv[2])
ast_type = int(sys.argv[3])

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


if ast_type >= 1 and ast_type <= 2:
    suite = find_suite_name()
    test = find_test_name(line_num)
    dotfile = f"{os.path.join(odb_path, 'ast', f'{suite}__{test}')}{ast_type}.ast"
    subprocess.run([f"{odb_path}/odb-asttool -i {dotfile} {asttool_args} | dot -Tx11"], shell=True)
elif ast_type >= 3 and ast_type <= 4:
    source = find_parser_source(line_num)
    subprocess.run([f"./odb-cli -b --dba --ast{ast_type - 2} | ./odb-asttool {asttool_args} | dot -Tx11"], shell=True, input=source.encode("utf8"), cwd=odb_path)
else:
    dotfile = os.path.join(odb_path, "test.ast")
    subprocess.run([f"{odb_path}/odb-asttool -i {dotfile} {asttool_args} | dot -Tx11"], shell=True)

