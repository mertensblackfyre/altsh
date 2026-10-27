#ifndef HELPER_H
#define HELPER_H


#include "../include/parser.h"

int helper_count_substrings(char *s);
char *helper_backtrack_path(char *s, int count);
char *helper_trim_space(char *string);
void helper_find_program();
char *helper_strtrim(char *str);
void helper_skip_whitespace(scanner *s);
bool helper_ispace(scanner *s);
bool helper_check_quotes(char *str);

#endif
