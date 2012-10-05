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

        rundir = calloc((size_t)PATH_MAX, sizeof(char));
        if (NULL != rundir) {
            /*
             * It is expected that root has privileges to access to access 
            * /var/run. Otherwise, the code looks for ${HOME} as the rundir,
            * and failing that, /tmp.
            */
            if (0 == getuid())
                snprintf(rundir, (size_t)PATH_MAX, "/var/run/%s/", __progname);
            else
                if (NULL == getenv("HOME"))
                    snprintf(rundir, (size_t)PATH_MAX, "/tmp/%s", __progname);
                else
                    snprintf(rundir, (size_t)PATH_MAX, "%s/.%s", 
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
            if ((ENOENT == errno) && (-1 == mkdir(rundir, (mode_t)00700)))
                return retval;
            else
                if (! (rdst.st_mode & S_IFDIR))
                    return retval;
        }

        testfile = calloc((size_t)PATH_MAX, sizeof(char));
        if (NULL == testfile)
            return retval;

        snprintf(testfile, (size_t)PATH_MAX, "%s/%s.testfile", rundir, __progname);

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
        pidstr  = calloc((size_t)MAX_PID_STR_SZ, sizeof(char));
        if ((NULL == pidfile) || (NULL == pidstr))
            goto pid_exit;

        snprintf(pidfile, (size_t)PATH_MAX, "%s/%s.pid", rundir, __progname);
        pidfd = open(pidfile, O_WRONLY | O_CREAT | O_EXCL, 
                     S_IRUSR | S_IWUSR);
        if (-1 == pidfd)
            goto pid_exit;

        if (-1 == flock(pidfd, LOCK_EX))
            goto pid_exit;

        snprintf(pidstr, (size_t)MAX_PID_STR_SZ, "%u", (unsigned int)pid);
        wrsz = write(pidfd, pidstr, (size_t)MAX_PID_STR_SZ);

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

        pidfile = calloc((size_t)PATH_MAX, sizeof(char));
        if (NULL == pidfile)
            return pidfile;

        snprintf(pidfile, (size_t)PATH_MAX, "%s/%s.pid", rundir, __progname);
 
        return pidfile;
}
