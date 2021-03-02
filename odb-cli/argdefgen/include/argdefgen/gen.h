#pragma once

#include <stdio.h>

struct adg_action;

void
adg_gen_cpp_write_typedefs(FILE* fp);

void
adg_gen_cpp_write_action_struct_def(FILE* fp);

void
adg_gen_cpp_write_action_table(struct adg_action** action_table, FILE* fp);
