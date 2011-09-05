/**************************************************************************
 * daemon.c                                                               *
 * 4096R/B7B720D6  "Kyle Isom <coder@kyleisom.net>"                       *
 * 2011-02-10                                                             *
 *                                                                        *
 * libdaemon: devio.us C library to facilitate daemonising                *
 *                                                                        *
 * this is a small C library providing a few functions to make creating   *
 * daemons under OpenBSD much easier. It is OpenBSD specific and will     *
 * not compile under Linux. See the header file for documentation.        *
 *                                                                        *
 **************************************************************************/

#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>

#include "daemon.h"

/* global vars */
static int daemon_logfd;                                /* logfile fd        */
static int daemon_pidfd;                                /* pidfile fd        */
static char pname[LIBDAEMON_D_NAME_MAX_LEN + 1];        /* process name      */

extern int errno;
                                            

/* function prototypes for internal-only functions */
static void dedaemonise(int);
static int  daemon_gen_pidfile(int);

/* begin function definitions */

int 
daemonise(char *d_name) 
{
    int fd;                     /* file descriptor when redirecting I/O      */
    struct stat vd_stat;        /* stat struct to test for directory         */
    char vardir[LIBDAEMON_FILENAME_MAX];

    /* truncating names may have unintended effects */
    if (strlen(d_name) > LIBDAEMON_D_NAME_MAX_LEN) 
        return EXIT_FAILURE;

    /* build strings */
    strlcpy(pname, d_name, LIBDAEMON_D_NAME_MAX_LEN);
    snprintf(vardir, LIBDAEMON_FILENAME_MAX, "%s/%s/", 
             LIBDAEMON_BASE_RUNDIR, pname);

    /* test to make sure we aren't already running */
    fd = daemon_gen_pidfile(LIBDAEMON_PIDF_TEST);
    if (EXIT_FAILURE == fd)
        return EXIT_FAILURE;

    /* drop privileges */
    if (0 != setreuid(LIBDAEMON_UID, LIBDAEMON_UID))
        return EXIT_FAILURE;
    else if (0 != setregid(LIBDAEMON_GID, LIBDAEMON_GID))
        return EXIT_FAILURE;

    /* exit if already a daemon */
    if (getppid() == 0) 
        return EXIT_FAILURE;

    /* set up rundir to store pidfile and lockfile in */
    if (0 != stat((const char *) vardir, &vd_stat))
        if (0 != mkdir(vardir, 0755))
            return EXIT_FAILURE;
            
    /* fork to background and kill off parent */
    if (fork() != 0) 
        exit(EXIT_SUCCESS);

    /* set session leader and grab pid */
    setsid();           /* grab the pid later, we're about to fork again */
    
    /* second fork prevents reacquisition of a controlling terminal */
    if (fork() != 0)
        exit(EXIT_SUCCESS);
        

    /* resetting umask for security */
    umask(0017);

    /* chdir to root */
    chdir("/");

     /* close all file descriptors */
    if (0 != closefrom(0)) 
        return EXIT_FAILURE;

    /* redirect I/O to /dev/null for security and stability */
    fd = open("/dev/null", O_RDWR);
    dup(fd);
    dup(fd);

    /* 
     * obtain a lock - not getting one means either system error or 
     * the daemon is already running. 
     */
    daemon_pidfd = daemon_gen_pidfile(NULL);

    if (-1 == daemon_pidfd) {
        syslog(LOG_INFO, "gen_pid failure");
        return EXIT_FAILURE; 
    }

    /* set up signal handlers */
    signal(SIGTTOU, SIG_IGN);           /* ignore stop process via bg write */
    signal(SIGTTIN, SIG_IGN);           /* ignore stop process via bg read  */
    signal(SIGTSTP, SIG_IGN);           /* ignore stop process */

    signal(LIBDAEMON_DEATH_KNOLL,       /* catch SIGUSR1 as a signal to     */
           dedaemonise);                /*      gracefully exit.            */

    syslog(LOG_INFO, "now alive!");

    /* passed the gauntlet */
    return EXIT_SUCCESS;
}


/* signal handler to gracefully exit */
static void 
dedaemonise(int sig) 
{
    char vardir[LIBDAEMON_FILENAME_MAX];
    char pidfile[LIBDAEMON_FILENAME_MAX];  

    /* make sure this isn't set to handle multiple signals */
    if (sig != LIBDAEMON_DEATH_KNOLL)
        return;
 
    snprintf(vardir, LIBDAEMON_FILENAME_MAX, "%s/%s/", 
             LIBDAEMON_BASE_RUNDIR, pname);
    snprintf(pidfile, LIBDAEMON_FILENAME_MAX, "%s%s.pid", vardir, pname);

    /* release lock */
    close(daemon_pidfd);
    flock(daemon_pidfd, LOCK_UN | LOCK_NB);

    /* close log before removing vardir in case log is in vardir */
    daemon_log(-1, "received SIGUSR1 - shutting down gracefully\n");
    close(daemon_logfd);

    /* clean up files */
    unlink(pidfile);
    rmdir(vardir);

    exit(EXIT_SUCCESS);
}

int 
daemon_set_logfile(char *filename)
{
    /* 
     * if a filename was passed in but is too long, the developer needs to fix
     * their code. trying to write to a truncated filename will cause confusion
     * to anyone using the code. 
     */
    if ((NULL != filename) && strlen(filename) > LIBDAEMON_FILENAME_MAX) {
        syslog(LOG_INFO, "filename too long!\n");
        return EXIT_FAILURE;
    }    

    /* NULL is passed in to indicate the daemon should log to syslog(3) */
    if (NULL == filename)
        daemon_logfd = -1;      /* use an invalid fd to indicate syslog(3)   */
    else                        /*      should be used.                      */
        daemon_logfd = open(filename, O_RDWR | O_CREAT, 0600);
    
    return EXIT_SUCCESS;
}


void
daemon_log(int log_level, char *msg)
{
    if (-1 == log_level)   
        log_level = LOG_INFO;

    if (-1 == daemon_logfd)     /* as mentioned in daemon_set_logfile(), -1   */
        syslog(log_level, msg); /*      indicates we should log to syslog(3). */
    else {
        lseek(daemon_logfd, 0, SEEK_END);
        write(daemon_logfd, msg, strlen(msg));
    }
}

void 
daemon_vlog(int log_level, char *msg, ...)
{
    va_list ap;
    char logmsg[LIBDAEMON_LOG_MAX];
    bzero(&logmsg, LIBDAEMON_LOG_MAX);

    va_start(ap, msg);
    vsnprintf(logmsg, LIBDAEMON_LOG_MAX, msg, ap);
    va_end(ap);

    if (-1 == log_level)
        log_level = LOG_INFO;

    if (-1 == daemon_logfd)
        syslog(log_level, logmsg);
    else
        write(daemon_logfd, logmsg, LIBDAEMON_LOG_MAX);

}

static int 
daemon_gen_pidfile(int flags)
{
    int fd;                                     /* descriptor for pidfile     */
    int oflags;                                 /* flags to open(2)           */
    char pidfile[LIBDAEMON_FILENAME_MAX];       /* filename for pidfile       */
    char pid[LIBDAEMON_PID_BUF];                /* pid->str                   */

    /* zero out buffers */
    bzero(&pidfile, LIBDAEMON_FILENAME_MAX);
    bzero(&pidfile, LIBDAEMON_PID_BUF);

    /* fill in buffers */
    snprintf(pidfile, LIBDAEMON_FILENAME_MAX, "%s/%s/%s.pid", 
             LIBDAEMON_BASE_RUNDIR, pname, pname);
    snprintf(pid, LIBDAEMON_PID_BUF, "%u\n", (unsigned int) getpid());

    if (flags & LIBDAEMON_PIDF_TEST)
        oflags = (O_RDONLY | O_EXCL);
    else
        oflags = (O_RDWR | O_TRUNC | O_CREAT);

    fd = open(pidfile, oflags, 0600);
    if ((flags & LIBDAEMON_PIDF_TEST) && (-1 == fd)) 
        return -1;

    if (flags & LIBDAEMON_PIDF_TEST) {
        close(fd);
        return EXIT_FAILURE;
    }

    if (EXIT_SUCCESS != flock(fd, LOCK_EX | LOCK_NB)) 
        return -1;

    if (-1 == ftruncate(fd, 0))
        return -1;

    if (-1 == write(fd, &pid, strlen(pid)))
        return -1;

    return fd;
}
