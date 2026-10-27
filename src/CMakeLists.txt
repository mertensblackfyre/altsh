add_library(altsh_lib
    builtin.c
    helper.c
    utils.c
    parser.c
)

add_executable(altsh main.c)

target_link_libraries(altsh altsh_lib)
