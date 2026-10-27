#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../include/parser.h"


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

    token t1 = parser_scan_token(strlen(input),&s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = parser_scan_token(strlen(input),&s);
    assert(t2.type == TOKEN_ARGUMENTS);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "hello") == 0);
    free(c2);

    token t3 = parser_scan_token(strlen(input),&s);
    assert(t3.type == TOKEN_EOF);
}

static void test_double_quotes_preserve_spaces() {
    char input[] = "echo \"hello    world\"";
    scanner s = make_scanner(input);

    token t1 = parser_scan_token(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = parser_scan_token(strlen(input), &s);
    assert(t2.type == TOKEN_DOUBLE_QUOTE);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "hello    world") == 0);
    free(c2);
}

static void test_multiple_spaces() {
    char input[] = "echo    hello";
    scanner s = make_scanner(input);

    token t1 = parser_scan_token(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = parser_scan_token(strlen(input), &s);
    assert(t2.type == TOKEN_ARGUMENTS);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "hello") == 0);
    free(c2);
}

static void test_adjacent_double_quotes_concatenated() {
    char input[] = "echo \"hello\"\"world\"";
    scanner s = make_scanner(input);

    token t1 = parser_scan_token(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = parser_scan_token(strlen(input), &s);
    assert(t2.type == TOKEN_DOUBLE_QUOTE);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "helloworld") == 0);
    free(c2);
}

static void test_quoted_unquoted_concatenated() {
    char input[] = "echo \"hello\"world";
    scanner s = make_scanner(input);

    token t1 = parser_scan_token(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = parser_scan_token(strlen(input), &s);
    assert(t2.type == TOKEN_DOUBLE_QUOTE);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "helloworld") == 0);
    free(c2);
}

static void test_separate_double_quoted_args() {
    char input[] = "echo \"hello\" \"world\"";
    scanner s = make_scanner(input);

    token t1 = parser_scan_token(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = parser_scan_token(strlen(input), &s);
    assert(t2.type == TOKEN_DOUBLE_QUOTE);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "hello") == 0);
    free(c2);

    token t3 = parser_scan_token(strlen(input), &s);
    assert(t3.type == TOKEN_DOUBLE_QUOTE);
    char *c3 = get_token_content(t3);
    assert(strcmp(c3, "world") == 0);
    free(c3);
}
static void test_single_quotes_preserve_spaces() {
    char input[] = "echo 'shell hello'";
    scanner s = make_scanner(input);

    token t1 = parser_scan_token(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = parser_scan_token(strlen(input), &s);
    assert(t2.type == TOKEN_SINGLE_QUOTE);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "shell hello") == 0);
    free(c2);
}

static void test_single_quotes_multiple_args() {
    char input[] = "cat '/tmp/file name' '/tmp/file name with spaces'";
    scanner s = make_scanner(input);

    token t1 = parser_scan_token(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "cat") == 0);
    free(c1);

    token t2 = parser_scan_token(strlen(input), &s);
    assert(t2.type == TOKEN_SINGLE_QUOTE);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "/tmp/file name") == 0);
    free(c2);

    token t3 = parser_scan_token(strlen(input), &s);
    assert(t3.type == TOKEN_SINGLE_QUOTE);
    char *c3 = get_token_content(t3);
    assert(strcmp(c3, "/tmp/file name with spaces") == 0);
    free(c3);
}

static void test_single_quote_inside_double_quotes() {
    char input[] = "echo \"shell's test\"";
    scanner s = make_scanner(input);

    token t1 = parser_scan_token(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = parser_scan_token(strlen(input), &s);
    assert(t2.type == TOKEN_DOUBLE_QUOTE);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "shell's test") == 0);
    free(c2);
}

static void test_empty_input() {
    char input[] = "";
    scanner s = make_scanner(input);
    token t = parser_scan_token(strlen(input), &s);
    assert(t.type == TOKEN_EOF);
}


static void test_escaped_spaces_single_arg() {
    char input[] = "echo multiple\\ \\ \\ \\ spaces";
    scanner s = make_scanner(input);

    token t1 = parser_scan_token(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = parser_scan_token(strlen(input), &s);
    assert(t2.type == TOKEN_ARGUMENTS);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "multiple    spaces") == 0);
    free(c2);
}

static void test_escaped_quotes_literal() {
    char input[] = "echo \\'\\\"literal quotes\\\"\\'";
    scanner s = make_scanner(input);

    token t1 = parser_scan_token(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = parser_scan_token(strlen(input), &s);
    assert(t2.type == TOKEN_ARGUMENTS);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "'\"literal") == 0);
    free(c2);

    token t3 = parser_scan_token(strlen(input), &s);
    assert(t3.type == TOKEN_ARGUMENTS);
    char *c3 = get_token_content(t3);
    assert(strcmp(c3, "quotes\"'") == 0);
    free(c3);
}

static void test_backslash_ignored_before_normal_char() {
    char input[] = "echo ignore\\_backslash";
    scanner s = make_scanner(input);

    token t1 = parser_scan_token(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = parser_scan_token(strlen(input), &s);
    assert(t2.type == TOKEN_ARGUMENTS);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "ignore_backslash") == 0);
    free(c2);
}

static void test_backslash_in_paths() {
    char input[] = "cat /tmp/\\_ignored_1 /tmp/ignore_\\2 /tmp/just_one_\\\\_3";
    scanner s = make_scanner(input);

    token t1 = parser_scan_token(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "cat") == 0);
    free(c1);

    token t2 = parser_scan_token(strlen(input), &s);
    assert(t2.type == TOKEN_ARGUMENTS);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "/tmp/_ignored_1") == 0);
    free(c2);

    token t3 = parser_scan_token(strlen(input), &s);
    assert(t3.type == TOKEN_ARGUMENTS);
    char *c3 = get_token_content(t3);
    assert(strcmp(c3, "/tmp/ignore_2") == 0);
    free(c3);

    token t4 = parser_scan_token(strlen(input), &s);
    assert(t4.type == TOKEN_ARGUMENTS);
    char *c4 = get_token_content(t4);
    assert(strcmp(c4, "/tmp/just_one_\\_3") == 0);
    free(c4);
}

static void test_single_quotes_double_backslash() {
    char input[] = "echo 'multiple\\\\slashes'";
    scanner s = make_scanner(input);

    token t1 = parser_scan_token(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = parser_scan_token(strlen(input), &s);
    assert(t2.type == TOKEN_SINGLE_QUOTE);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "multiple\\\\slashes") == 0);
    free(c2);
}

static void test_single_quotes_backslash_before_double_quote() {
    char input[] = "echo 'every\\\"thing_is\\\"literal'";
    scanner s = make_scanner(input);

    token t1 = parser_scan_token(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = parser_scan_token(strlen(input), &s);
    assert(t2.type == TOKEN_SINGLE_QUOTE);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "every\\\"thing_is\\\"literal") == 0);
    free(c2);
}

static void test_single_quotes_in_paths() {
    char input[] = "cat /tmp/'no slash 1' /tmp/'one slash \\2' /tmp/'two slashes \\3\\'";
    scanner s = make_scanner(input);

    token t1 = parser_scan_token(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "cat") == 0);
    free(c1);

    token t2 = parser_scan_token(strlen(input), &s);
    assert(t2.type == TOKEN_ARGUMENTS);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "/tmp/no slash 1") == 0);
    free(c2);

    token t3 = parser_scan_token(strlen(input), &s);
    assert(t3.type == TOKEN_ARGUMENTS);
    char *c3 = get_token_content(t3);
    assert(strcmp(c3, "/tmp/one slash \\2") == 0);
    free(c3);

    token t4 = parser_scan_token(strlen(input), &s);
    assert(t4.type == TOKEN_ARGUMENTS);
    char *c4 = get_token_content(t4);
    assert(strcmp(c4, "/tmp/two slashes \\3\\") == 0);
    free(c4);
}

static void test_single_quotes_backslash_n_literal() {
    char input[] = "echo 'shell\\\\\\nscript'";
    scanner s = make_scanner(input);

    token t1 = parser_scan_token(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = parser_scan_token(strlen(input), &s);
    assert(t2.type == TOKEN_SINGLE_QUOTE);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "shell\\\\\\nscript") == 0);
    free(c2);
}

static void test_single_quotes_backslash_double_quote_example() {
    char input[] = "echo 'example\\\"test'";
    scanner s = make_scanner(input);

    token t1 = parser_scan_token(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = parser_scan_token(strlen(input), &s);
    assert(t2.type == TOKEN_SINGLE_QUOTE);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "example\\\"test") == 0);
    free(c2);
}

static void test_double_quotes_single_quotes_and_backslash_n() {
    char input[] = "echo \"just'one'\\\\n'backslash\"";
    scanner s = make_scanner(input);

    token t1 = parser_scan_token(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = parser_scan_token(strlen(input), &s);
    assert(t2.type == TOKEN_DOUBLE_QUOTE);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "just'one'\\n'backslash") == 0);
    free(c2);
}

static void test_double_quotes_escaped_quote_then_outside() {
    char input[] = "echo \"inside\\\"literal_quote.\"outside\\\"";
    scanner s = make_scanner(input);

    token t1 = parser_scan_token(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = parser_scan_token(strlen(input), &s);
    assert(t2.type == TOKEN_DOUBLE_QUOTE);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "inside\"literal_quote.outside\"") == 0);
    free(c2);
}

static void test_double_quotes_backslash_escapes_itself() {
    char input[] = "echo \"A \\\\ escapes itself\"";
    scanner s = make_scanner(input);

    token t1 = parser_scan_token(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = parser_scan_token(strlen(input), &s);
    assert(t2.type == TOKEN_DOUBLE_QUOTE);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "A \\ escapes itself") == 0);
    free(c2);
}

static void test_double_quotes_escaped_double_quote() {
    char input[] = "echo \"A \\\" inside double quotes\"";
    scanner s = make_scanner(input);

    token t1 = parser_scan_token(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = parser_scan_token(strlen(input), &s);
    assert(t2.type == TOKEN_DOUBLE_QUOTE);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "A \" inside double quotes") == 0);
    free(c2);
}

static void test_double_quotes_in_paths() {
    char input[] = "cat /tmp/\"number 1\" /tmp/\"doublequote \\\" 2\" /tmp/\"backslash \\\\ 3\"";
    scanner s = make_scanner(input);

    token t1 = parser_scan_token(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "cat") == 0);
    free(c1);

    token t2 = parser_scan_token(strlen(input), &s);
    assert(t2.type == TOKEN_ARGUMENTS);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "/tmp/number 1") == 0);
    free(c2);

    token t3 = parser_scan_token(strlen(input), &s);
    assert(t3.type == TOKEN_ARGUMENTS);
    char *c3 = get_token_content(t3);
    assert(strcmp(c3, "/tmp/doublequote \" 2") == 0);
    free(c3);

    token t4 = parser_scan_token(strlen(input), &s);
    assert(t4.type == TOKEN_ARGUMENTS);
    char *c4 = get_token_content(t4);
    assert(strcmp(c4, "/tmp/backslash \\ 3") == 0);
    free(c4);
}

static void test_double_quotes_backslash_kept_before_normal_char() {
    char input[] = "echo \"a\\qb\\nc\"";
    scanner s = make_scanner(input);

    token t1 = parser_scan_token(strlen(input), &s);
    assert(t1.type == TOKEN_COMMAND);
    char *c1 = get_token_content(t1);
    assert(strcmp(c1, "echo") == 0);
    free(c1);

    token t2 = parser_scan_token(strlen(input), &s);
    assert(t2.type == TOKEN_DOUBLE_QUOTE);
    char *c2 = get_token_content(t2);
    assert(strcmp(c2, "a\\qb\\nc") == 0);
    free(c2);
}


static void run_parser_tests(){

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

    test_double_quotes_single_quotes_and_backslash_n();
    test_double_quotes_escaped_quote_then_outside();
    test_double_quotes_backslash_escapes_itself();
    test_double_quotes_escaped_double_quote();
    test_double_quotes_in_paths();
    test_double_quotes_backslash_kept_before_normal_char();

    printf("parser tests passed\n");
}
