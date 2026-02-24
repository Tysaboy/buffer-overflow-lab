// fixed_server.c - patched version with correct bounds checking
// Build: gcc -O2 -o fixed_server fixed_server.c
// Run:   ./fixed_server 22222

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef BUFSZ
#define BUFSZ 32
#endif

struct Globals {
    char buffer[BUFSZ];
    char password[16];
    char secret[64];
} globals;

static ssize_t read_line_safe(int fd, char *dst, size_t dstsz) {
    if (dstsz == 0) return -1;
    size_t i = 0;

    while (i + 1 < dstsz) {
        char c;
        ssize_t r = read(fd, &c, 1);
        if (r <= 0) break;
        dst[i++] = c;
        if (c == '\n') break;
    }
    dst[i] = '\0';
    return (ssize_t)i;
}

static void die(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 2;
    }

    snprintf(globals.password, sizeof(globals.password), "letmein");
    snprintf(globals.secret, sizeof(globals.secret), "FLAG{demo_secret_do_not_use_in_prod}");

    int port = atoi(argv[1]);
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) die("socket");

    int yes = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons((uint16_t)port);

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) die("bind");
    if (listen(s, 5) < 0) die("listen");

    printf("[fixed] listening on 127.0.0.1:%d (BUFSZ=%d)\n", port, BUFSZ);

    while (1) {
        int cfd = accept(s, NULL, NULL);
        if (cfd < 0) die("accept");

        char input[BUFSZ];
        dprintf(cfd, "Password: ");
        ssize_t n = read_line_safe(cfd, input, sizeof(input));
        if (n <= 0) {
            close(cfd);
            continue;
        }

        char *nl = strchr(input, '\n');
        if (nl) *nl = '\0';

        if (strcmp(input, globals.password) == 0) {
            dprintf(cfd, "The secret is: %s\n", globals.secret);
        } else {
            dprintf(cfd, "Wrong password.\n");
        }

        close(cfd);
    }
}
