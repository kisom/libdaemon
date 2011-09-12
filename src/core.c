/****************************************************************************
 * file: core.c                                                             *
 * author: kyle isom <coder@kyleisom.net>                                   *
 * created: 2011-09-11                                                      *
 * modified: 2011-09-11                                                     *
 *                                                                          *
 * core and public functionality of libdaemon, a small C library to         *
 * facilitate daemonising.                                                  *
 *                                                                          *
 * it is released under an ISC / public domain dual-license; see any of the *
 * header files or the file "LICENSE" (or COPYING) under the project root.  *
 ****************************************************************************/
#include <config.h>

#include <sys/types.h>
#include <sys/stat.h>
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
static struct libdaemon_config  *cfg;
extern char                     *__progname;

/* private daemon functions */
static void                      dedaemonise(int);

/* external functions */
extern char    *get_default_rundir(void);
extern int      test_rundir_access(char *);
extern int      gen_pidfile(char *);
extern int      destroy_pidfile(char *);


int
init_daemon(char *rundir, uid_t run_uid, gid_t run_gid)
{
        char            *testfile;          /* for access priv check */
        int              free_rundir, retval;
   
        retval = EXIT_FAILURE;
        testfile = NULL;
        free_rundir = 0;

        if (NULL != cfg) {
            fprintf(stderr, "[!] daemon already initalised!\n");
            fprintf(stderr, "\tcall destroy_daemon before reinitialising!\n");
            goto init_exit;
        }

        /* if run_uid is 0, set it to the current uid. */
        if (0 == run_uid)
            run_uid = getuid();

        if (0 == run_gid)
            run_gid = getgid();

        if (NULL == rundir) 
            rundir = get_default_rundir();
            if (NULL != rundir)
                free_rundir = 1;
            else {
                fprintf(stderr, "[!] rundir returned NULL!\n");
                goto init_exit;
            }

        if (-1 == test_rundir_access(rundir))
            goto init_exit;

    /*
     * We wait until all the tests and sanity checks have passed before 
     * saving any of this to the configuration struct so as to ensure the
     * user doesn't accidentally try to daemonise with an invalid setup.
     */
    cfg = calloc(1, sizeof(struct libdaemon_config));
    if (NULL == cfg)
        goto init_exit;

    cfg->rundir     = calloc(PATH_MAX, sizeof(char));
    cfg->pidfile    = calloc(PATH_MAX, sizeof(char));
    cfg->logfile    = NULL;        /* set via daemon_setlog */
    cfg->logfd      = -1;          /* set via daemon_setlog */
    cfg->run_uid    = run_uid;
    cfg->run_gid    = run_gid;

    snprintf(cfg->rundir, PATH_MAX, "%s", rundir);
    libdaemon_do_kill = 0;
    retval = EXIT_SUCCESS;

init_exit:
        perror(__progname);
        if (1 == free_rundir) {
            free(rundir);
            rundir = NULL;
        }
        free(testfile);
        testfile = NULL;
        return retval;
}


int
run_daemon(void)
{
        int fd, retval;
        retval = EXIT_FAILURE;

        if (NULL == cfg) {
            fprintf(stderr, "[!] config struct not initalised!\n");
            fprintf(stderr, "\tinit_daemon needs to be called first!\n");
            goto run_exit;
        }

        /* ensure we aren't running already */
        if (EXIT_FAILURE == gen_pidfile(cfg->rundir)) {
            perror("gen_pidfile");
            goto run_exit;
        }
        else
            if (EXIT_FAILURE == destroy_pidfile(cfg->rundir)) {
                perror("destroy_pidfile");
                goto run_exit;
            }

        syslog(LOG_INFO, "attempting to daemonise as %u:%u...",
               (unsigned int)cfg->run_uid, (unsigned int)cfg->run_gid);

        /* attempt to drop privileges if required */
        if ((cfg->run_uid != getuid()) || 
            (0 != setreuid(cfg->run_uid, cfg->run_uid)))
            goto run_exit;
        if ((cfg->run_gid != getgid()) || 
            (0 != setregid(cfg->run_gid, cfg->run_gid)))
            goto run_exit;

        /* If we are already daemonised, fail to redaemonise. */
        if (0 == getppid()) {
            retval = LIBDAEMON_DO_NOT_DESTROY;    
            goto run_exit;
        }

        /* Fork to background and kill off the parent. */
        if (0 != fork()) {
            retval = LIBDAEMON_DO_NOT_DESTROY;    
            goto run_exit;
        }

        /* Set session leader. */
        setsid();

        /* Fork again to prevent reacquisition of a controlling terminal. */
        if (0 != fork()) {
            retval = LIBDAEMON_DO_NOT_DESTROY;
            goto run_exit;
        }

        /* Write the final pid to a file. */
        if (EXIT_FAILURE == gen_pidfile(cfg->rundir)) {
            perror("gen_pidfile");
            goto run_exit;
        }
 
        /* Reset the umask to more secure permissions. */
        umask((mode_t)0027);

        /* Change the working directory to filesystem root. */
        if (-1 == chdir("/"))
            goto run_exit;

        /* Close all file descriptors. */
        #ifdef _LINUX_SOURCE
        if ((0 != close(0)) || (0 != close(1)) || (0 != close(2)))
        #else
        if (0 != closefrom(0))
        #endif
            goto run_exit;

        /* We should redirect I/O to /dev/null for security and stability. */
        fd = open("/dev/null", O_RDWR);
        if (-1 == fd)
            goto run_exit;
        if ((-1 == dup(fd)) || (-1 == dup(fd)))
            goto run_exit;
        
        /* Ignore certain signals and setup death knoll signal handler.  */
        signal(SIGTTOU, SIG_IGN);   /* Ignore stop process via bg write. */
        signal(SIGTTIN, SIG_IGN);   /* Ignore stop process via bg read.  */
        signal(SIGTSTP, SIG_IGN);   /* Ignore stop process.              */

        /* Catch death knoll as a signal to exit gracefully. */
        signal(LIBDAEMON_DEATH_KNOLL, dedaemonise);


        syslog(LOG_INFO, "daemonised!");

        retval = EXIT_SUCCESS;

run_exit:
        if (EXIT_FAILURE == retval)
            perror("run_daemon");

        return retval;
}

int
destroy_daemon(void)
{
        int retval;

        retval = EXIT_FAILURE;

        if (NULL == cfg) {
            fprintf(stderr, "[!] daemon not initialised!\n");
            fprintf(stderr, "\tyou should not call destroy_daemon before");
            fprintf(stderr, "calling init_daemon.\n");
            goto destroy_exit;
        }

        if (-1 == destroy_pidfile(cfg->rundir)) 
            perror("attempting to unlink pidfile");

        free(cfg->rundir);
        free(cfg->pidfile);
        free(cfg->logfile);

        cfg->rundir = NULL;
        cfg->pidfile = NULL;
        cfg->logfile = NULL;

        if ((cfg->logfd > 0) && (-1 == close(cfg->logfd)) && (EBADF != errno))
            goto destroy_exit;

        free(cfg);
        cfg = NULL;

        /* Restore stdin, stdout, and stderr. */
        #ifdef _LINUX_SOURCE
        if ((0 != close(0)) || (0 != close(1)) || (0 != close(2)))
        #else
        if (0 != closefrom(0))
        #endif
            goto destroy_exit;
        
        if ((-1 == open("/dev/stdin",  O_RDONLY))   ||
            (-1 == open("/dev/stdout", O_WRONLY))   ||
            (-1 == open("/dev/stdout", O_WRONLY)))
            goto destroy_exit;

        retval = EXIT_SUCCESS;
        syslog(LOG_INFO, "successfully daedaemonised!");
destroy_exit:
        
        return retval;
}



struct libdaemon_config 
*daemon_getconfig()
{
        return cfg;
}

void
dedaemonise(int flags)
{
    flags = 0;
    syslog(LOG_INFO, "destroying daemon: %d", destroy_daemon());
    exit(EXIT_SUCCESS);
}
