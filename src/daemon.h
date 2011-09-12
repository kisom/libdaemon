/****************************************************************************
 * file: daemon.h                                                           *
 * author: kyle isom <coder@kyleisom.net>                                   *
 * created: 2011-09-11                                                      *
 * modified: 2011-09-11                                                     *
 *                                                                          *
 * public interface for libdaemon                                           *
 *                                                                          *
 ****************************************************************************
 * the ISC license:                                                         *
 * Copyright (c) 2011 Kyle Isom <coder@kyleisom.net>                        *
 *                                                                          *
 * Permission to use, copy, modify, and distribute this software for any    *
 * purpose with or without fee is hereby granted, provided that the above   *
 * copyright notice and this permission notice appear in all copies.        *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES *
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF         *
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR  *
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES   *
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN    *
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF  *
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.           *
 ****************************************************************************
 * you may choose to follow this license or public domain. my intent with   *
 * dual-licensing this code is to afford you, the end user, maximum freedom *
 * with the software. if public domain affords you more freedom, use it.    *
 ****************************************************************************/

#ifndef __LIBDAEMON_DAEMON_H
#define __LIBDAEMON_DAEMON_H

#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>


#define LIBDAEMON_DEATH_KNOLL       SIGUSR1
#define LIBDAEMON_DO_NOT_DESTROY    EXIT_FAILURE + 1

static int      libdaemon_do_kill;  /* set to 1 if libdaemon should die */

struct libdaemon_config {
        char    *rundir;
        char    *pidfile;
        char    *logfile;
        int      logfd;

        uid_t   run_uid;
        gid_t   run_gid;

};

int                      init_daemon(char *, uid_t, gid_t);
int                      run_daemon(void);
int                      destroy_daemon(void);
int                      daemon_setlog(char *logfile);
int                      daemon_log(int, char *);
int                      daemon_vlog(int, char *, ...);
struct libdaemon_config *daemon_getconfig(void);


#endif
