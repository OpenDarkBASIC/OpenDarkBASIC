#pragma once

char*
adg_str_dup(const char* src);

char*
adg_str_dup_range(const char* src, int start, int end);

void
adg_str_free(char* str);

/*!
 * @brief Appends the string s2 at the end of s1 delimited by delim.
 * Ownership of s1 is taken and returned. If the function fails then s1 will
 * be freed for you.
 */
char*
adg_str_join(char* s1, const char* s2, const char* delim);
