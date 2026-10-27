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

// Restores the stream and returns what was printed. Caller frees it.
static char *capture_end(capture_ctx *c) {
    fflush(stdout);
    fflush(stderr);
    dup2(c->saved, c->fd);
    close(c->saved);
    rewind(c->tmp);
    char *buf = calloc(4096, 1);
    fread(buf, 1, 4095, c->tmp);
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
    assert(strcmp(cwd, expected) == 0);
}
static void test_cd_absolute_path(void) {
    char *argv[] = {"cd", "/usr/local/bin", NULL};
    parsed_input in = make_input(argv, 2);
    builtin_cd(&in);
    assert_cwd_is("/usr/local/bin");

    capture_ctx c = capture_start(STDOUT_FILENO);
    builtin_pwd();
    char *out = capture_end(&c);
    assert(strcmp(out, "/usr/local/bin\n") == 0);
    free(out);
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

static void test_cd_relative_dot_path(void) {
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

static void test_cd_tilde_goes_home(void) {
    setenv("HOME", "/usr", 1);
    assert(chdir("/var/log") == 0);
    char *argv[] = {"cd", "~", NULL};
    parsed_input in = make_input(argv, 2);
    builtin_cd(&in);
    assert_cwd_is("/usr");
}

static void test_cd_no_argument_goes_home(void) {
    setenv("HOME", "/usr", 1);
    assert(chdir("/var/log") == 0);
    char *argv[] = {"cd", NULL};
    parsed_input in = make_input(argv, 1);
    builtin_cd(&in);
    assert_cwd_is("/usr");
}

static void test_type_builtins_and_unknown() {
    capture_ctx c = capture_start(STDOUT_FILENO);
    builtin_type("echo");
    char *out = capture_end(&c);
    assert(strcmp(out, "echo is a shell builtin\n") == 0);
    free(out);

    c = capture_start(STDOUT_FILENO);
    builtin_type("exit");
    out = capture_end(&c);
    assert(strcmp(out, "exit is a shell builtin\n") == 0);
    free(out);

    c = capture_start(STDOUT_FILENO);
    builtin_type("invalid_command");
    out = capture_end(&c);
    assert(strcmp(out, "invalid_command: not found\n") == 0);
    free(out);
}

static void test_builtin_check() {
    assert(builtin_check("echo") == true);
    assert(builtin_check("exit") == true);
    assert(builtin_check("type") == true);
    assert(builtin_check("pwd") == true);
    assert(builtin_check("cd") == true);
    assert(builtin_check("invalid_command") == false);
}

static void test_echo_arguments() {
    char *a[] = {"echo", "hello", "world", NULL};
    parsed_input ia = make_input(a, 3);
    capture_ctx c = capture_start(STDOUT_FILENO);
    builtin_echo(&ia);
    char *out = capture_end(&c);
    assert(strcmp(out, "hello world\n") == 0);
    free(out);

    char *b[] = {"echo", "one", "two", "three", NULL};
    parsed_input ib = make_input(b, 4);
    c = capture_start(STDOUT_FILENO);
    builtin_echo(&ib);
    out = capture_end(&c);
    assert(strcmp(out, "one two three\n") == 0);
    free(out);
}

static void test_exit_terminates_process() {

    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        builtin_exit(3);
        _exit(99);             
    }
    int status;
    waitpid(pid, &status, 0);
    assert(WIFEXITED(status));
    assert(WEXITSTATUS(status) == 3);
}


static void run_builtin_tests(void) {
    test_cd_absolute_path();
    test_cd_nonexistent_keeps_directory();
    test_cd_tilde_goes_home();
    test_type_builtins_and_unknown();
    test_builtin_check();
    test_echo_arguments();
    test_exit_terminates_process();
    printf("builtin tests passed\n");
}
