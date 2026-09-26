#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../include/args.h"


static scanner make_scanner(char *input) {
    scanner s;
    s.start = input;
    s.current = input;
    s.pos = 0;
    return s;
}

static char* get_token_content(token t) {
    char *buf = (char*)malloc(strlen(t.literal) + 1);
    strcpy(buf, t.literal);
    return buf;
}

// --- Test cases ---
static void test_simple_command_args() {
    char input[] = "echo hello";
    scanner s = make_scanner(input);

    token t1 = args_token_scan(strlen(input),&s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = args_token_scan(strlen(input),&s);
    assert(t2.type == TOKEN_ARGUMENTS);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "hello") == 0);
    free(c2);

    token t3 = args_token_scan(strlen(input),&s);
    assert(t3.type == TOKEN_EOF);
}

static void test_double_quotes_preserve_spaces() {
    char input[] = "echo \"hello    world\"";
    scanner s = make_scanner(input);

    token t1 = args_token_scan(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = args_token_scan(strlen(input), &s);
    assert(t2.type == TOKEN_DOUBLE_QUOTE);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "hello    world") == 0);
    free(c2);
}

static void test_multiple_spaces() {
    char input[] = "echo    hello";
    scanner s = make_scanner(input);

    token t1 = args_token_scan(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = args_token_scan(strlen(input), &s);
    assert(t2.type == TOKEN_ARGUMENTS);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "hello") == 0);
    free(c2);
}

static void test_adjacent_double_quotes_concatenated() {
    char input[] = "echo \"hello\"\"world\"";
    scanner s = make_scanner(input);

    token t1 = args_token_scan(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = args_token_scan(strlen(input), &s);
    assert(t2.type == TOKEN_DOUBLE_QUOTE);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "helloworld") == 0);
    free(c2);
}

static void test_quoted_unquoted_concatenated() {
    char input[] = "echo \"hello\"world";
    scanner s = make_scanner(input);

    token t1 = args_token_scan(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = args_token_scan(strlen(input), &s);
    assert(t2.type == TOKEN_DOUBLE_QUOTE);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "helloworld") == 0);
    free(c2);
}

static void test_separate_double_quoted_args() {
    char input[] = "echo \"hello\" \"world\"";
    scanner s = make_scanner(input);

    token t1 = args_token_scan(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = args_token_scan(strlen(input), &s);
    assert(t2.type == TOKEN_DOUBLE_QUOTE);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "hello") == 0);
    free(c2);

    token t3 = args_token_scan(strlen(input), &s);
    assert(t3.type == TOKEN_DOUBLE_QUOTE);
    char *c3 = get_token_content(t3);
    assert(strcmp(c3, "world") == 0);
    free(c3);
}
static void test_single_quotes_preserve_spaces() {
    char input[] = "echo 'shell hello'";
    scanner s = make_scanner(input);

    token t1 = args_token_scan(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = args_token_scan(strlen(input), &s);
    assert(t2.type == TOKEN_SINGLE_QUOTE);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "shell hello") == 0);
    free(c2);
}

static void test_single_quotes_multiple_args() {
    char input[] = "cat '/tmp/file name' '/tmp/file name with spaces'";
    scanner s = make_scanner(input);

    token t1 = args_token_scan(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "cat") == 0);
    free(c1);

    token t2 = args_token_scan(strlen(input), &s);
    assert(t2.type == TOKEN_SINGLE_QUOTE);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "/tmp/file name") == 0);
    free(c2);

    token t3 = args_token_scan(strlen(input), &s);
    assert(t3.type == TOKEN_SINGLE_QUOTE);
    char *c3 = get_token_content(t3);
    assert(strcmp(c3, "/tmp/file name with spaces") == 0);
    free(c3);
}

static void test_single_quote_inside_double_quotes() {
    char input[] = "echo \"shell's test\"";
    scanner s = make_scanner(input);

    token t1 = args_token_scan(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = args_token_scan(strlen(input), &s);
    assert(t2.type == TOKEN_DOUBLE_QUOTE);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "shell's test") == 0);
    free(c2);
}

static void test_empty_input() {
    char input[] = "";
    scanner s = make_scanner(input);
    token t = args_token_scan(strlen(input), &s);
    assert(t.type == TOKEN_EOF);
}
