/****************************************************************************
 * file: core.c                                                             *
 * author: kyle isom <coder@kyleisom.net>                                   *
 * created: 2011-09-11                                                      *
 * modified: 2011-09-11                                                     *
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

/* this is the primary configuration struct for the daemon */
static struct libdaemon_config  *cfg;
extern char                     *__progname;

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
            run_gid = getgid();

        if (NULL == rundir) {
            rundir = calloc(PATH_MAX, sizeof(char));
            if (NULL == rundir)
                goto init_exit;
            else
                free_rundir = 1;
            
            /* 
             * It is expected that root has privileges to access /var/run.
             * Otherwise, the code looks for ${HOME} as the rundir.
             */
            if (0 == run_uid)
                snprintf(rundir, PATH_MAX, "/var/run/%s", __progname);
            else {      /* braces not needed but clears up the code */
                if (NULL == getenv("HOME"))
                    snprintf(rundir, PATH_MAX, "/tmp/%s", __progname);
                else
                    snprintf(rundir, PATH_MAX, "%s/.%s", 
                             getenv("HOME"), __progname);
            }
        }

        testfile = calloc(PATH_MAX, sizeof(char));
        if (NULL == testfile)
            goto init_exit;

        snprintf(testfile, PATH_MAX, "%s/%s.testfile", rundir, __progname);

        if ((-1 == setreuid(run_uid, run_uid)) ||
            (-1 == setregid(run_gid, run_gid)))
            goto init_exit;

        statres = stat(rundir, &rdst);
        if (-1 == statres)
            if (ENOENT == errno)
                /* 
                 * The top level for rundir is the only component of the path
                 * that cannot exist, i.e. dirname rundir must exist. This 
                 * follows from the behaviour /var/run/__progname/ : /var/run
                 * is assumed to exist.
                 */
                 if (-1 == mkdir(rundir, 00700))
                    goto init_exit;
            else
                goto init_exit;

        testfd = open(testfile, O_WRONLY | O_CREAT | O_EXCL, 
                      S_IRUSR | S_IWUSR);
        if (-1 == testfd)
            goto init_exit;
        else {
            close(testfd);
            unlink(testfile);
        }
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

        free(cfg->rundir);
        free(cfg->pidfile);
        free(cfg->logfile);

        cfg->rundir = NULL;
        cfg->pidfile = NULL;
        cfg->logfile = NULL;

        if ((-1 == close(cfg->logfd)) && (EBADF != errno))
            goto destroy_exit;

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

