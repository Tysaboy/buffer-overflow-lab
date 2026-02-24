// vuln_server.c - intentionally vulnerable (for education)
// Build: gcc -O0 -fno-stack-protector -z execstack -no-pie -o vuln_server vuln_server.c
// Run:   ./vuln_server 22222
// NOTE: This is a clean-room demo. Do not deploy.

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

// Intentionally packed together to make overwrite behavior clear.
struct Globals {
    char buffer[BUFSZ];
    char password[16];
    char secret[64];
} globals;

// BAD: wrong bounds check (uses address math unrelated to buffer length)
static ssize_t read_line_vuln(int fd) {
    char *p = globals.buffer;
    while (1) {
        char c;
        ssize_t r = read(fd, &c, 1);
        if (r <= 0) return r;

        *p++ = c;

        // ❌ Bug: stops only when reaching "just before secret", not buffer end.
        if (p == globals.secret - 1) break;

        if (c == '\n') break;
    }
    *p = '\0';
    return (ssize_t)(p - globals.buffer);
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

    // Initialize sensitive data (demo values)
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
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // localhost only
    addr.sin_port = htons((uint16_t)port);

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) die("bind");
    if (listen(s, 5) < 0) die("listen");

    printf("[vuln] listening on 127.0.0.1:%d (BUFSZ=%d)\n", port, BUFSZ);

    while (1) {
        int cfd = accept(s, NULL, NULL);
        if (cfd < 0) die("accept");

        dprintf(cfd, "Password: ");
        ssize_t n = read_line_vuln(cfd);
        if (n <= 0) {
            close(cfd);
            continue;
        }

        // strip newline
        char *nl = strchr(globals.buffer, '\n');
        if (nl) *nl = '\0';

        if (strcmp(globals.buffer, globals.password) == 0) {
            dprintf(cfd, "The secret is: %s\n", globals.secret);
        } else {
            dprintf(cfd, "Wrong password.\n");
        }

        close(cfd);
    }
}
