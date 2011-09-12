/****************************************************************************
 * file: rundir.c                                                           *
 * author: kyle isom <coder@kyleisom.net>                                   *
 * created: 2011-09-12                                                      *
 * modified: 2011-09-12                                                     *
 *                                                                          *
 * functions to deal with the run directory                                 *
 *                                                                          *
 * it is released under an ISC / public domain dual-license; see any of the *
 * header files or the file "LICENSE" (or COPYING) under the project root.  *
 ****************************************************************************/

#include <config.h>
#ifdef _LINUX_SOURCE
#include <sys/file.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rundir.h"

#define     MAX_PID_STR_SZ          0x20    /* string representation of pid */

static char *get_pidfile_name(char *);

extern char     *__progname;

/*
 * int get_default_rundir(char *rundir)
 *  returns: EXIT_SUCCESS | EXIT_FAILURE
 *  parameter: char *rundir
 *      rundir should be a NULL pointer that will be used to store the rundir
 *
 * You should test the return value before trusting the value of rundir after
 * the function exits. If retval is EXIT_SUCCESS, rundir has been allocated
 * (and therefore you will need to free it), and contains the best guess as to
 * where the daemon should be running.
 */
char *get_default_rundir()
{
        int retval = EXIT_FAILURE;      /* return value defaults to fail */
        char *rundir;

        rundir = NULL;

        rundir = calloc(PATH_MAX, sizeof(char));
        if (NULL != rundir) {
            /*
             * It is expected that root has privileges to access to access 
            * /var/run. Otherwise, the code looks for ${HOME} as the rundir,
            * and failing that, /tmp.
            */
            if (0 == getuid())
                snprintf(rundir, PATH_MAX, "/var/run/%s/", __progname);
            else
                if (NULL == getenv("HOME"))
                    snprintf(rundir, PATH_MAX, "/tmp/%s", __progname);
                else
                    snprintf(rundir, PATH_MAX, "%s/.%s", 
                             getenv("HOME"), __progname);

            retval = EXIT_SUCCESS;
        }
        else
            fprintf(stderr, "[!] get_default_rundir: could not calloc!\n");

        if (EXIT_FAILURE == retval) {
            free(rundir);
            rundir = NULL;
        }
        
        return rundir;
}


int
test_rundir_access(char *rundir)
{
        struct stat rdst;
        int retval, statres, testfd;
        char *testfile;

        retval = EXIT_FAILURE;
        testfile = NULL;

        /* This is a sanity check to make sure rundir isn't NULL. */
        if (NULL == rundir)
            return retval;

        statres = stat(rundir, &rdst);
        if (-1 == statres) {
            if ((ENOENT == errno) && (-1 == mkdir(rundir, 00700)))
                return retval;
            else
                if (! (rdst.st_mode & S_IFDIR))
                    return retval;
        }

        testfile = calloc(PATH_MAX, sizeof(char));
        if (NULL == testfile)
            return retval;

        snprintf(testfile, PATH_MAX, "%s/%s.testfile", rundir, __progname);

        testfd = open(testfile, O_CREAT | O_EXCL | O_WRONLY,
                      S_IRUSR | S_IWUSR);

        if (-1 == testfd) {
            free(testfile);
            testfile = NULL;
            return retval;
        }

        if ((0 == close(testfd)) && (0 == unlink(testfile)))
            retval = EXIT_SUCCESS;

        /* Perform all cleanup here.*/
        free(testfile);
        testfile = NULL;

        return retval;
}


int
gen_pidfile(char *rundir)
{
        ssize_t wrsz;
        pid_t pid;
        int retval, pidfd;
        char *pidfile, *pidstr;


        retval  = EXIT_FAILURE;
        pidfd   = -1;
        pidfile = NULL;
        pidstr  = NULL;

        /* This is a quick sanity check to make sure rundir exists. */
        if (NULL == rundir)
            return retval;

        pid = getpid();

        pidfile = get_pidfile_name(rundir);
        pidstr  = calloc(MAX_PID_STR_SZ, sizeof(char));
        if ((NULL == pidfile) || (NULL == pidstr))
            goto pid_exit;

        snprintf(pidfile, PATH_MAX, "%s/%s.pid", rundir, __progname);
        pidfd = open(pidfile, O_WRONLY | O_CREAT | O_EXCL, 
                     S_IRUSR | S_IWUSR);
        if (-1 == pidfd)
            goto pid_exit;

        if (-1 == flock(pidfd, LOCK_EX))
            goto pid_exit;

        snprintf(pidstr, MAX_PID_STR_SZ, "%u", (unsigned int)pid);
        wrsz = write(pidfd, pidstr, MAX_PID_STR_SZ);

        if ((wrsz <= 0) || (-1 == ftruncate(pidfd, (off_t)strlen(pidstr))))
            goto pid_exit;

        retval = EXIT_SUCCESS;


pid_exit:
        /* Now we do our cleanup. */
        free(pidfile);
        free(pidstr);
        pidfile = NULL;
        pidstr  = NULL;

        if (0 < pidfd)
            close(pidfd);

        return retval;
}

int
destroy_pidfile(char *rundir)
{
        int retval, syscallres;
        char *pidfile;

        pidfile = NULL;
        retval  = EXIT_FAILURE;

        /* Ensure the user hasn't passed in a NULL rundir. */
        if (NULL == rundir)
            goto destroy_exit;

        pidfile = get_pidfile_name(rundir);
        if (NULL == pidfile)
            goto destroy_exit;

        syscallres = unlink(pidfile);
        if (-1 == syscallres)
            goto destroy_exit;

        retval = EXIT_SUCCESS;

destroy_exit:
        free(pidfile);
        pidfile = NULL;

        return retval;
}

char
*get_pidfile_name(char *rundir)
{
        char *pidfile;

        pidfile = NULL;

        /* This is a quick sanity check to make sure rundir exists. */
        if (NULL == rundir)
            return pidfile;

        pidfile = calloc(PATH_MAX, sizeof(char));
        if (NULL == pidfile)
            return pidfile;

        snprintf(pidfile, PATH_MAX, "%s/%s.pid", rundir, __progname);
 
        return pidfile;
}
