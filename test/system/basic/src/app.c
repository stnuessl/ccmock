/*
 * Copyright (C) 2022   Steffen Nuessle
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

void app_print(const char *fmt, ...);

int app_main(int argc, char *argv[])
{
    char *buf;
    int fd;
    size_t n, bufsize = 64;

    (void) argc;
    (void) argv;

    buf = malloc(bufsize);
    if (unlikely(!buf)) {
        fprintf(stderr, "error: failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }

    memset(buf, 0xff, bufsize);

    fd = open("/dev/null", O_WRONLY);
    if (unlikely(fd < 0)) {
        fprintf(stderr, "error, failed to open file\n");
        exit(EXIT_FAILURE);
    }

    n = 0;

    do {
        ssize_t m = write(fd, buf + n, bufsize - n);
        if (unlikely(m < 0)) {
            if (errno == EINTR)
                continue;

            fprintf(stderr, "error: failed to write data\n");
            exit(EXIT_FAILURE);
        }
        n += m;
    } while (n < bufsize);

    free(buf);

    app_print("Ok\n");

    return EXIT_SUCCESS;
}
