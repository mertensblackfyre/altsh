#include "../tests/test_args.c"
#include <stdio.h>

int main() {
  test_simple_command_args();
  test_multiple_spaces();
  test_double_quotes_preserve_spaces();
  test_adjacent_double_quotes_concatenated();
  test_quoted_unquoted_concatenated();
  test_separate_double_quoted_args();
  test_single_quotes_preserve_spaces();
  test_single_quotes_multiple_args();
  test_single_quote_inside_double_quotes();
  test_empty_input();
  test_escaped_spaces_single_arg();
  test_escaped_quotes_literal();
  test_backslash_ignored_before_normal_char();
  test_backslash_in_paths();

  test_single_quotes_double_backslash();
  test_single_quotes_backslash_before_double_quote();
  test_single_quotes_in_paths();
  test_single_quotes_backslash_n_literal();
  test_single_quotes_backslash_double_quote_example();

  printf("All tests passed!\n");
  return 0;
}
