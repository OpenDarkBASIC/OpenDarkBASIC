#pragma once

#include <stdio.h>

struct adg_action;

void
adg_gen_cpp_write_typedefs(FILE* fp);

void
adg_gen_cpp_write_entry_function_forward_decl(FILE* fp);

void
adg_gen_cpp_write_action_struct_def(FILE* fp);

void
adg_gen_cpp_write_action_table(struct adg_action** action_table, FILE* fp);

void
adg_gen_cpp_write_argparse_preamble(FILE* fp);

void
adg_gen_cpp_write_argparse_postamble(FILE* fp);

void
adg_gen_cpp_write_helpers_forward_decl(FILE* fp);

void
adg_gen_cpp_write_helpers_impl(FILE* fp);
