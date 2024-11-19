import sys
import os
import subprocess

odb_path = "./build-Debug/bin/x86_64/linux/bin"
fname = sys.argv[1]
line_num = int(sys.argv[2]) - 1
visual_mode = int(sys.argv[3])

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
            break
        line_num += 1
    raise RuntimeError("Failed to find start marker")

def find_end_marker(line_num):
    while line_num < len(lines):
        if "/* odb-asttool end */" in lines[line_num]:
            return line_num
        line_num += 1
    raise RuntimeError("Failed to find end marker")

def find_asttool_args(line_num):
    if "/* odb-asttool --format" not in lines[line_num]:
        raise RuntimeError("Failed to find asttool args")
    args = lines[line_num].strip("/* ").split(" ")[1:]
    while "*" in lines[line_num+1]:
        args += lines[line_num+1].strip("/* ").split(" ")
        line_num += 1
    return args


test_start = find_test_start(line_num)
suite = find_suite_name()
test = get_test_name(test_start)

# in "visual mode" the text we write to stdout becomes the text vim replaces
# with its current visual selection, so we don't have to generate the entire
# file
if visual_mode:
    result = subprocess.run(
        [f"{odb_path}/odb-tests --gtest_filter=\"{suite}.{test}\" --ast | \
         {odb_path}/odb-asttool --format gtest"], shell=True)
    sys.exit(result.returncode)

# In "non-visual mode" we generate the entire file, so the insertion has to
# be done manually
start_marker = find_start_marker(test_start + 1)
end_marker = find_end_marker(start_marker + 1)
asttool_args = find_asttool_args(start_marker)

odbtests = subprocess.Popen([
    os.path.join(odb_path, "odb-tests"),
    f"--gtest_filter={suite}.{test}",
    "--ast"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
odbtests_stdout, odbtests_stderr = odbtests.communicate()

asttool = subprocess.Popen([
    os.path.join(odb_path, "odb-asttool")] + asttool_args,
    stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
asttool_stdout, asttool_stderr = asttool.communicate(input=odbtests_stdout)
if asttool.returncode != 0:
    print(asttool_stdout, asttool_stderr)
    sys.exit(asttool.returncode)

asttool_lines = asttool_stdout.decode("utf8").split("\n")
lines[start_marker:end_marker+1] = asttool_lines

print("\n".join(lines).strip("\n"))

