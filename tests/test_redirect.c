#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../include/helper.h"
#include "../tests/test.h"

static char test_dir[] = "/tmp/altsh_XXXXXX";
static void redirect_setup(void) {
    assert(mkdtemp(test_dir) != NULL);
}

static void redirect_cleanup(const char *file) {
    unlink(file);
}

static void make_path(char *out, size_t size, const char *name) {
    snprintf(out, size, "%s/%s", test_dir, name);
}

// Returns the whole file as a heap string, or NULL if it can't be opened.
static char *read_file(const char *path) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
        return NULL;
    char *buf = calloc(4096, 1);
    size_t n = fread(buf, 1, 4095, fp);
    buf[n] = '\0';
    fclose(fp);
    return buf;
}

// ls /tmp/baz > baz.md
static void test_redirect_creates_file_with_content(void) {
    char path[256];
    make_path(path, sizeof(path), "baz.md");

    helper_write(path, "apple\nblueberry\n", "w");

    char *out = read_file(path);
    assert(out != NULL);
    assert(strcmp(out, "apple\nblueberry\n") == 0);
    free(out);
    redirect_cleanup(path);
}

// echo Hello James 1> foo.md
static void test_redirect_single_line(void) {
    char path[256];
    make_path(path, sizeof(path), "foo.md");

    helper_write(path, "Hello James\n", "w");

    char *out = read_file(path);
    assert(out != NULL);
    assert(strcmp(out, "Hello James\n") == 0);
    free(out);
    redirect_cleanup(path);
}

// cat baz/blueberry nonexistent 1> quz.md  (only stdout goes in the file)
static void test_redirect_only_stdout_content(void) {
    char path[256];
    make_path(path, sizeof(path), "quz.md");

    helper_write(path, "blueberry\n", "w");   // the stderr line is never passed in

    char *out = read_file(path);
    assert(out != NULL);
    assert(strcmp(out, "blueberry\n") == 0);
    assert(strstr(out, "nonexistent") == NULL);
    free(out);
    redirect_cleanup(path);
}

// cat nonexistent > output.txt  (command prints nothing, file still created and empty)
static void test_redirect_empty_output_creates_empty_file(void) {
    char path[256];
    make_path(path, sizeof(path), "output.txt");

    helper_write(path, "", "w");

    char *out = read_file(path);
    assert(out != NULL);
    assert(strcmp(out, "") == 0);
    free(out);
    redirect_cleanup(path);
}

// > replaces what was there before
static void test_redirect_truncates_existing_file(void) {
    char path[256];
    make_path(path, sizeof(path), "trunc.txt");

    helper_write(path, "old old old\n", "w");
    helper_write(path, "new\n", "w");

    char *out = read_file(path);
    assert(out != NULL);
    assert(strcmp(out, "new\n") == 0);
    free(out);
    redirect_cleanup(path);
}

// >> keeps the old content and adds to it
static void test_redirect_append_mode(void) {
    char path[256];
    make_path(path, sizeof(path), "append.txt");

    helper_write(path, "first\n", "w");
    helper_write(path, "second\n", "a");

    char *out = read_file(path);
    assert(out != NULL);
    assert(strcmp(out, "first\nsecond\n") == 0);
    free(out);
    redirect_cleanup(path);
}

// target folder doesn't exist: error on stderr, no file created
static void test_redirect_missing_directory(void) {
    char path[256];
    make_path(path, sizeof(path), "no_such_dir/out.txt");

    capture_ctx c = capture_start(STDERR_FILENO);
    helper_write(path, "data\n", "w");
    char *err = capture_end(&c);

    char expected[512];
    snprintf(expected, sizeof(expected),
             "altsh: %s: No such file or directory\n", path);
    assert(strcmp(err, expected) == 0);
    free(err);

    assert(access(path, F_OK) != 0);
}

static void run_redirect_tests(void) {
    redirect_setup();
    test_redirect_creates_file_with_content();
    test_redirect_single_line();
    test_redirect_only_stdout_content();
    test_redirect_empty_output_creates_empty_file();
    test_redirect_truncates_existing_file();
    test_redirect_append_mode();
    test_redirect_missing_directory();
    rmdir(test_dir);
    printf("redirect tests passed\n");
}
