/* Copyright © 2018-2019 N. Van Bossuyt.                                      */
/* This code is licensed under the MIT License.                               */
/* See: LICENSE.md                                                            */

#include <skift/io.h>

DEFN_SYSCALL1(sk_io_print, SYS_IO_PRINT, const char *);
DEFN_SYSCALL2(sk_io_read, SYS_IO_READ, char *, int);