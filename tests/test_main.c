#include "../tests/test_parser.c"
#include "../tests/test_redirect.c"
#include <stdio.h>

int main() {


    run_redirect_tests();
//run_builtin_tests();
run_parser_tests();
  printf("All tests passed!\n");
  return 0;
}
