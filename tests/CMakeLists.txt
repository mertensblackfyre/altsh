add_executable(test_args
    test_parser.c
    test_main.c
)

target_link_libraries(test_args altsh_lib)

add_test(NAME parser_scanner_tests COMMAND test_parser)
