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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "daemon.h"
#include "rundir.h"

/* this is the primary configuration struct for the daemon */
static struct libdaemon_config  *cfg;
extern char                     *__progname;

/* private daemon functions */
static int                      dedaemonise;

int
init_daemon(char *rundir, uid_t run_uid, gid_t run_gid)
{
        struct stat      rdst;
        char            *testfile;          /* for access priv check */
        int              statres, free_rundir, testfd, retval;
        uid_t            suid;
        gid_t            sgid;

   
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
    retval = EXIT_SUCCESS;
    errno = 0;

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
        int retval;
        retval = EXIT_FAILURE;

        if (NULL == cfg) {
            fprintf(stderr, "[!] config struct not initalised!\n");
            fprintf(stderr, "\tinit_daemon needs to be called first!\n");
            goto run_exit;
        }

        if (EXIT_FAILURE == gen_pidfile(cfg->rundir)) {
            perror("gen_pidfile");
            goto run_exit;
        }

        retval = EXIT_SUCCESS;
run_exit:

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

        retval = EXIT_SUCCESS;
        errno = 0;

destroy_exit:
        
        return retval;
}



struct libdaemon_config 
*daemon_getconfig()
{
        return cfg;
}


