#include "../tests/test_parser.c"
#include "../tests/test_builtin.c"
#include <stdio.h>

int main() {

run_builtin_tests();
run_parser_tests();
  printf("All tests passed!\n");
  return 0;
}
