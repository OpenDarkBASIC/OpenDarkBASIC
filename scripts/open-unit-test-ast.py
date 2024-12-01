import sys
import os
import subprocess

odb_path = "./build-Debug/bin/x86_64/linux/bin"
fname = sys.argv[1]
line_num = int(sys.argv[2])
ast_type = int(sys.argv[3])

lines = open(fname, "rb").read().decode("utf8").split("\n")

def find_suite_name():
    for line in lines:
        if "#define NAME" in line:
            return line.split(" ")[2]
    raise RuntimeError("Failed to find suite name")

def find_test_start(line_num):
    while line_num >= 1:
        if "TEST(" in lines[line_num] or "TEST_F(" in lines[line_num]:
            return line_num
        line_num -= 1
    raise RuntimeError("Failed to find test name")

def get_test_name(line_num):
    if "TEST(" not in lines[line_num] and \
        "TEST_F(" not in lines[line_num]:
        raise RuntimeError("Failed to find test name")
    return lines[line_num].split(",")[1].strip(") ")

def find_start_marker(line_num):
    while line_num < len(lines):
        if "/* odb-asttool --format" in lines[line_num]:
            return line_num
        if "TEST(" in lines[line_num] or "TEST_F(" in lines[line_num]:
            return None
        line_num += 1
    return None

def find_parser_source(line_num):
    while line_num < len(lines):
        if "parse(" in lines[line_num] or "const char*" in lines[line_num]:
            break
        line_num += 1
    while line_num < len(lines):
        if '"' in lines[line_num]:
            break;
        line_num += 1
    source = str()
    while '"' in lines[line_num]:
        source += lines[line_num].split('"')[1]
        line_num += 1
    return "\n".join(source.split("\\n"))

def find_asttool_args(line_num):
    if "/* odb-asttool --format" not in lines[line_num]:
        raise RuntimeError("Failed to find asttool args")
    args = lines[line_num]\
            .strip("/* ")\
            .replace("--format gtest", "")\
            .split(" ")[1:]
    while "*" in lines[line_num+1]:
        args += lines[line_num+1].strip("/* ").split(" ")
        line_num += 1
    return args

test_start = find_test_start(line_num)
start_marker = find_start_marker(test_start + 1)
asttool_args = find_asttool_args(start_marker) if start_marker else ""

if ast_type >= 1 and ast_type <= 2:
    source = find_parser_source(test_start)
    subprocess.run([
        f"./odb-cli -b --dba --ast{ast_type} | \
        ./odb-asttool {' '.join(asttool_args)} | \
        dot -Tx11"],
        shell=True,
        input=source.encode("utf8"),
        cwd=odb_path)
elif ast_type == 3:
    suite = find_suite_name()
    test = get_test_name(test_start)
    subprocess.run([
        f"./odb-asttool -i ast/{suite}__{test}.ast {' '.join(asttool_args)} | \
        dot -Tx11"],
        shell=True,
        cwd=odb_path)
else:
    subprocess.run([
        f"./odb-asttool -i test.ast {' '.join(asttool_args)} | \
        dot -Tx11"],
        shell=True,
        cwd=odb_path)

