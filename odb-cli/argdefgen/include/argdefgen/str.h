#pragma once

char*
adg_str_dup(const char* src);

char*
adg_str_dup_range(const char* src, int start, int end);

void
adg_str_free(char* str);
