#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/builtin.h"
#include "../include/parser.h"

typedef struct {
    int fd;
    int saved;
    FILE *tmp;
} capture_ctx;

static capture_ctx capture_start(int fd) {
    fflush(stdout);
    fflush(stderr);
    capture_ctx c;
    c.fd = fd;
    c.saved = dup(fd);
    c.tmp = tmpfile();
    dup2(fileno(c.tmp), fd);
    return c;
}

static char *capture_end(capture_ctx *c) {
    fflush(stdout);
    fflush(stderr);
    dup2(c->saved, c->fd);
    close(c->saved);
    rewind(c->tmp);
    char *buf = calloc(4096, 1);
    size_t n = fread(buf, 1, 4095, c->tmp);
    buf[n] = '\0';
    fclose(c->tmp);
    return buf;
}

static parsed_input make_input(char **argv, int argc) {
    parsed_input p;
    p.argv = argv;
    p.cmd = argv[0];
    p.argc = argc;
    return p;
}

static void assert_cwd_is(const char *expected) {
    char cwd[PATH_MAX];
    assert(getcwd(cwd, sizeof(cwd)) != NULL);
    if (strcmp(cwd, expected) != 0)
        fprintf(stderr, "cwd mismatch: got [%s], expected [%s]\n", cwd, expected);
    assert(strcmp(cwd, expected) == 0);
}

/* ---------- pwd ---------- */

static void test_pwd_prints_cwd(void) {
    assert(chdir("/usr") == 0);
    capture_ctx c = capture_start(STDOUT_FILENO);
    builtin_pwd();
    char *out = capture_end(&c);
    assert(strcmp(out, "/usr\n") == 0);
    free(out);
}

static void test_pwd_via_check(void) {
    assert(chdir("/var/log") == 0);
    char line[] = "pwd";
    capture_ctx c = capture_start(STDOUT_FILENO);
    bool handled = builtin_check(line);
    char *out = capture_end(&c);
    assert(handled == true);
    assert(strcmp(out, "/var/log\n") == 0);
    free(out);
}

/* ---------- cd ---------- */

static void test_cd_absolute_path(void) {
    assert(chdir("/") == 0);
    char *argv[] = {"cd", "/usr/local/bin", NULL};
    parsed_input in = make_input(argv, 2);
    builtin_cd(&in);
    assert_cwd_is("/usr/local/bin");
}

static void test_cd_nonexistent_keeps_directory(void) {
    assert(chdir("/usr/local/bin") == 0);
    char *argv[] = {"cd", "/does_not_exist", NULL};
    parsed_input in = make_input(argv, 2);

    capture_ctx c = capture_start(STDERR_FILENO);
    builtin_cd(&in);
    char *err = capture_end(&c);
    assert(strcmp(err, "Error: directory specified in path does not exist\n") == 0);
    free(err);

    assert_cwd_is("/usr/local/bin");
}

static void test_cd_not_a_directory(void) {
    assert(chdir("/usr") == 0);
    char *argv[] = {"cd", "/etc/passwd", NULL};
    parsed_input in = make_input(argv, 2);

    capture_ctx c = capture_start(STDERR_FILENO);
    builtin_cd(&in);
    char *err = capture_end(&c);
    assert(strcmp(err, "Error: path is not a directory\n") == 0);
    free(err);

    assert_cwd_is("/usr");
}

static void test_cd_dot_relative(void) {
    assert(chdir("/usr") == 0);
    char *argv[] = {"cd", "./local/bin", NULL};
    parsed_input in = make_input(argv, 2);
    builtin_cd(&in);
    assert_cwd_is("/usr/local/bin");
}

static void test_cd_parent_two_levels(void) {
    assert(chdir("/usr/local/bin") == 0);
    char *argv[] = {"cd", "../../", NULL};
    parsed_input in = make_input(argv, 2);
    builtin_cd(&in);
    assert_cwd_is("/usr");
}

static void test_cd_plain_relative_name(void) {
    assert(chdir("/usr") == 0);
    char *argv[] = {"cd", "local", NULL};
    parsed_input in = make_input(argv, 2);
    builtin_cd(&in);
    assert_cwd_is("/usr/local");
}

static void test_cd_tilde(void) {
    setenv("HOME", "/usr", 1);
    assert(chdir("/var/log") == 0);
    char *argv[] = {"cd", "~", NULL};
    parsed_input in = make_input(argv, 2);
    builtin_cd(&in);
    assert_cwd_is("/usr");
}

static void test_cd_no_argument(void) {
    setenv("HOME", "/usr", 1);
    assert(chdir("/var/log") == 0);
    char *argv[] = {"cd", NULL};
    parsed_input in = make_input(argv, 1);
    builtin_cd(&in);
    assert_cwd_is("/usr");
}

static void test_cd_via_check(void) {
    assert(chdir("/") == 0);
    char line[] = "cd /usr/local/bin";
    bool handled = builtin_check(line);
    assert(handled == true);
    assert_cwd_is("/usr/local/bin");
}

/* ---------- echo ---------- */

static void test_echo_two_words(void) {
    char *argv[] = {"echo", "hello", "world", NULL};
    parsed_input in = make_input(argv, 3);
    capture_ctx c = capture_start(STDOUT_FILENO);
    builtin_echo(&in);
    char *out = capture_end(&c);
    assert(strcmp(out, "hello world\n") == 0);
    free(out);
}

static void test_echo_three_words(void) {
    char *argv[] = {"echo", "one", "two", "three", NULL};
    parsed_input in = make_input(argv, 4);
    capture_ctx c = capture_start(STDOUT_FILENO);
    builtin_echo(&in);
    char *out = capture_end(&c);
    assert(strcmp(out, "one two three\n") == 0);
    free(out);
}

static void test_echo_no_arguments(void) {
    char *argv[] = {"echo", NULL};
    parsed_input in = make_input(argv, 1);
    capture_ctx c = capture_start(STDOUT_FILENO);
    builtin_echo(&in);
    char *out = capture_end(&c);
    assert(strcmp(out, "\n") == 0);
    free(out);
}

static void test_echo_via_check(void) {
    char line[] = "echo hello world";
    capture_ctx c = capture_start(STDOUT_FILENO);
    bool handled = builtin_check(line);
    char *out = capture_end(&c);
    assert(handled == true);
    assert(strcmp(out, "hello world\n") == 0);
    free(out);
}

/* ---------- type ---------- */

static void test_type_returns_true_for_builtins(void) {
    assert(builtin_type("echo") == true);
    assert(builtin_type("type") == true);
    assert(builtin_type("exit") == true);
    assert(builtin_type("pwd") == true);
    assert(builtin_type("cd") == true);
}

static void test_type_returns_false_for_others(void) {
    assert(builtin_type("invalid_command") == false);
    assert(builtin_type("ech") == false);
    assert(builtin_type("echoo") == false);
    assert(builtin_type("ls") == false);
}

static void test_type_prints_builtin_message(void) {
    char line[] = "type echo";
    capture_ctx c = capture_start(STDOUT_FILENO);
    bool handled = builtin_check(line);
    char *out = capture_end(&c);
    assert(handled == true);
    assert(strcmp(out, "echo is a shell builtin\n") == 0);
    free(out);

    char line2[] = "type exit";
    c = capture_start(STDOUT_FILENO);
    handled = builtin_check(line2);
    out = capture_end(&c);
    assert(handled == true);
    assert(strcmp(out, "exit is a shell builtin\n") == 0);
    free(out);
}

static void test_type_prints_not_found(void) {
    char line[] = "type invalid_command";
    capture_ctx c = capture_start(STDOUT_FILENO);
    bool handled = builtin_check(line);
    char *out = capture_end(&c);
    assert(handled == true);
    assert(strcmp(out, "invalid_command: not found\n") == 0);
    free(out);
}

/* ---------- builtin_check ---------- */

static void test_check_rejects_non_builtins(void) {
    char line[] = "ls -l";
    assert(builtin_check(line) == false);

    char line2[] = "invalid_command";
    assert(builtin_check(line2) == false);
}

/* ---------- exit (run in a child so the runner survives) ---------- */

static int run_in_child(int mode) {
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        if (mode == 0) {
            builtin_exit(3);
        } else {
            char line[] = "exit 3";
            builtin_check(line);
        }
        _exit(99);   // only reached if exit did not happen
    }
    int status;
    waitpid(pid, &status, 0);
    assert(WIFEXITED(status));
    return WEXITSTATUS(status);
}

static void test_exit_uses_given_status(void) {
    assert(run_in_child(0) == 3);
}

static void test_exit_via_check_uses_argument(void) {
    assert(run_in_child(1) == 3);
}

/* ---------- runner ---------- */

static void run_builtin_tests(void) {
    test_pwd_prints_cwd();
    test_pwd_via_check();

    test_cd_absolute_path();
    test_cd_nonexistent_keeps_directory();
    test_cd_not_a_directory();
    test_cd_dot_relative();
    test_cd_parent_two_levels();
    test_cd_plain_relative_name();
    test_cd_tilde();
    test_cd_no_argument();
    test_cd_via_check();

    test_echo_two_words();
    test_echo_three_words();
    test_echo_no_arguments();
    test_echo_via_check();

    test_type_returns_true_for_builtins();
    test_type_returns_false_for_others();
    test_type_prints_builtin_message();
    test_type_prints_not_found();

    test_check_rejects_non_builtins();

    test_exit_uses_given_status();
    test_exit_via_check_uses_argument();

    printf("builtin tests passed\n");
}
