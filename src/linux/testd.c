/***********************************************
 * testd.c                                     *
 * Kyle Isom <coder@kyleisom.net>              *
 *                                             *
 * test daemon code to show usage of libdaemon *
 *                                             *
 * released under an ISC license.              *
 ***********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include "daemon.h"

#define LOAD_WARN_THRESH                1.5

#ifdef USE_SYSLOG
#define LOGFILE                         NULL
#else
#define LOGFILE                         "/var/log/testd.log"
#endif

int
main(int argc, char **argv)
{
    double lavg[1];         /* load average */
    int result;

    result = EXIT_FAILURE;

    /* daemonise the program */
    if (EXIT_SUCCESS != daemonise("testd")) {
        syslog(LOG_INFO, "error daemonising!\n");
        return EXIT_FAILURE;
    }

    /* set up logging */
    if (EXIT_SUCCESS == daemon_set_logfile(LOGFILE))
        daemon_log(-1, "running!");

    else
        return result;

    /* main run loop */
    while (1) {
        /* get one minute load average */
        if (-1 == getloadavg(lavg, 1))
            daemon_log(LOG_INFO, "error retrieving load average!");
        else if (lavg[0] > LOAD_WARN_THRESH)
            daemon_vlog(LOG_WARNING, "load average exceeded %f: "
                        "is %f!\n", LOAD_WARN_THRESH, lavg[0]);
        else
            daemon_log(LOG_INFO, "wakes up...\n");
        sleep(60);
    }

    /* should not end up here - all shutdowns should be via signal */
    return EXIT_SUCCESS;
}

