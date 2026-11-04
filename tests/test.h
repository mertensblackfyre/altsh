#ifndef CAPTURE_H
#define CAPTURE_H



#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>


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
    char *buf = (char *)calloc(4096, 1);
    size_t n = fread(buf, 1, 4095, c->tmp);
    buf[n] = '\0';
    fclose(c->tmp);
    return buf;
}

#endif // CAPTURE_H
