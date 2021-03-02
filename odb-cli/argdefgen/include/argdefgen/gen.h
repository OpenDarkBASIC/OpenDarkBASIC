#pragma once

#include <stdio.h>

struct adg_action;

void
adg_gen_write_action_struct_cpp(FILE* fp);

void
adg_gen_write_action_table_cpp(struct adg_action** action_table, FILE* fp);
