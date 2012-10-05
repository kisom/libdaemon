/*
 * the ISC license:                                                         
 * Copyright (c) 2011 Kyle Isom <coder@kyleisom.net>                        
 *                                                                          
 * Permission to use, copy, modify, and distribute this software for any    
 * purpose with or without fee is hereby granted, provided that the above   
 * copyright notice and this permission notice appear in all copies.        
 *                                                                          
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES 
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF         
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR  
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES   
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN    
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF  
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.           
 *
 * you may choose to follow this license or public domain. my intent with   
 * dual-licensing this code is to afford you, the end user, maximum freedom 
 * with the software. if public domain affords you more freedom, use it.    
 */

#include <config.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#include "daemon.h"
#include "rundir.h"

/* this is the primary configuration struct for the daemon */
static struct libdaemon_config  *daemon_cfg;
extern char *__progname;

static void                      dedaemonise(int);


/*
 * initialise the daemon and prepare for daemonisation
 */
int
init_daemon(char *rundir, uid_t run_uid, gid_t run_gid)
{
        char            *testfile;          /* for access priv check */
        int              free_rundir, retval;

        retval = EXIT_FAILURE;
        testfile = NULL;
        free_rundir = 0;

        if (NULL != daemon_cfg) {
                warnx("[!] daemon already initalised!");
                warnx("\tcall destroy_daemon before reinitialising!");
                goto init_exit;
        }

        if (0 == run_uid)
                run_uid = getuid();
        if (0 == run_gid)
                run_gid = getgid();

        if (NULL == rundir) 
                rundir = get_default_rundir();

        if (NULL != rundir) {
                free_rundir = 1;
        } else {
                warnx("[!] rundir returned NULL!");
                goto init_exit;
        }

        if (-1 == test_rundir_access(rundir))
                goto init_exit;

        daemon_cfg = calloc((size_t)1, sizeof(struct libdaemon_config));
        if (NULL == daemon_cfg)
                goto init_exit;

        daemon_cfg->rundir     = calloc((size_t)PATH_MAX, sizeof(char));
        daemon_cfg->pidfile    = calloc((size_t)PATH_MAX, sizeof(char));
        daemon_cfg->run_uid    = run_uid;
        daemon_cfg->run_gid    = run_gid;

        snprintf(daemon_cfg->rundir, (size_t)PATH_MAX, "%s", rundir);
        libdaemon_do_kill = 0;
        retval = EXIT_SUCCESS;

init_exit:
        if (EXIT_FAILURE == retval)
                perror(__progname);
        if (1 == free_rundir) {
                free(rundir);
                rundir = NULL;
        }
        free(testfile);
        testfile = NULL;
        return retval;
}


/*
 * run_daemon actually daemonises the program
 */
int
run_daemon(void)
{
        int fd, retval;
        retval = EXIT_FAILURE;

        if (NULL == daemon_cfg) {
                warnx("[!] config struct not initalised!");
                warnx("\tinit_daemon needs to be called first!");
                goto run_exit;
        }

        if (EXIT_FAILURE == gen_pidfile(daemon_cfg->rundir)) {
                perror("gen_pidfile");
                goto run_exit;
        } else {
                if (EXIT_FAILURE == destroy_pidfile(daemon_cfg->rundir)) {
                        perror("destroy_pidfile");
                        goto run_exit;
                }
        }

        syslog(LOG_INFO, "attempting to daemonise as %u:%u...",
            (unsigned int)daemon_cfg->run_uid, 
            (unsigned int)daemon_cfg->run_gid);

        if ((daemon_cfg->run_uid != getuid()) || 
            (0 != setreuid(daemon_cfg->run_uid, daemon_cfg->run_uid)))
                goto run_exit;
        if ((daemon_cfg->run_gid != getgid()) || 
            (0 != setregid(daemon_cfg->run_gid, daemon_cfg->run_gid)))
                goto run_exit;

        if (0 == getppid()) {
                retval = LIBDAEMON_DO_NOT_DESTROY;    
                goto run_exit;
        } else if (0 != fork()) {
                retval = LIBDAEMON_DO_NOT_DESTROY;    
                goto run_exit;
        }

        setsid();
        if (0 != fork()) {
                retval = LIBDAEMON_DO_NOT_DESTROY;
                goto run_exit;
        } else if (EXIT_FAILURE == gen_pidfile(daemon_cfg->rundir)) {
                warn("failed to write pid file");
                goto run_exit;
        }

        umask((mode_t)0027);
        if (-1 == chdir("/"))
                goto run_exit;
        else if ((0 != close(0)) || (0 != close(1)) || (0 != close(2)))
                goto run_exit;

        fd = open("/dev/null", O_RDWR);
        if (-1 == fd)
                goto run_exit;
        else if ((-1 == dup(fd)) || (-1 == dup(fd)))
                goto run_exit;

        signal(SIGTTOU, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);

        signal(LIBDAEMON_DEATH_KNOLL, dedaemonise);
        daemon_cfg->pidfile = get_pidfile_name(daemon_cfg->rundir);
        syslog(LOG_INFO, "daemonised!");

        retval = EXIT_SUCCESS;

run_exit:
        if (EXIT_FAILURE == retval)
                warn("failed to daemonise");

        return retval;
}


/*
 * destroy_daemon frees all the daemon information and prepares for
 * a clean exit.
 */
int
destroy_daemon(void)
{
        int retval = EXIT_FAILURE;

        if (NULL == daemon_cfg) {
                warnx("daemon not initialised");
                goto destroy_exit;
        }

        if (-1 == destroy_pidfile(daemon_cfg->rundir)) 
                warn("attempting to unlink pidfile");

        free(daemon_cfg->rundir);
        free(daemon_cfg->pidfile);
        syslog(LOG_INFO, "config pidfile: %s", daemon_cfg->pidfile);

        daemon_cfg->rundir = NULL;
        daemon_cfg->pidfile = NULL;

        free(daemon_cfg);
        daemon_cfg = NULL;

        if ((0 != close(0)) || (0 != close(1)) || (0 != close(2)))
                goto destroy_exit;
        else if ((-1 == open("/dev/stdin",  O_RDONLY))   ||
                 (-1 == open("/dev/stdout", O_WRONLY))   ||
                 (-1 == open("/dev/stdout", O_WRONLY)))
                goto destroy_exit;

        retval = EXIT_SUCCESS;
        syslog(LOG_INFO, "successfully daedaemonised!");

destroy_exit:
        return retval;
}


/*
 * return the dameon configuration for use by the client
 */
struct libdaemon_config 
*daemon_getconfig()
{
        return daemon_cfg;
}


/*
 * signal handler to tear down the daemon.
 */
void
dedaemonise(int flags)
{
        flags = 0;
        syslog(LOG_INFO, "destroying daemon: %d", destroy_daemon());
        exit(EXIT_SUCCESS);
}
