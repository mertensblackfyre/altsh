add_executable(test_altsh
    test_parser.c
    test_redirect.c
    test_main.c
)

target_link_libraries(test_altsh altsh_lib)

add_test(NAME parser_tests COMMAND test_parser)
