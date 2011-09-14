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
main(void)
{
    double lavg[1];         /* load average */
    int dres, result;

    result = EXIT_FAILURE;

    /* daemonise the program */
    dres = init_daemon(NULL, 0, 0);
    if ((EXIT_SUCCESS != dres) || (EXIT_SUCCESS != daemon_setlog(NULL))) {
        syslog(LOG_INFO, "error daemonising!");
        if (LIBDAEMON_DO_NOT_DESTROY != dres) 
            destroy_daemon();
        return EXIT_FAILURE;
    }

    else {
        dres = run_daemon();
        if (EXIT_SUCCESS != dres) {
            daemon_log(DAEMON_INFO, "run_daemon failed!");
            if (LIBDAEMON_DO_NOT_DESTROY != dres)
                destroy_daemon();
            return EXIT_FAILURE;
        }
    }
    /* set up logging */
    /*
    if (EXIT_SUCCESS == daemon_set_logfile(LOGFILE))
        daemon_log(-1, "running!");

    else
        return result;
    */
    /* main run loop */
    while (1) {
        daemon_log(DAEMON_INFO, "still running...");
        /* get one minute load average */
        if (-1 == getloadavg(lavg, 1))
            daemon_log(DAEMON_INFO, "error retrieving load average!");
        else if (lavg[0] > LOAD_WARN_THRESH)
            daemon_vlog(DAEMON_WARNING, "load average exceeded %f: "
                        "is %f!\n", LOAD_WARN_THRESH, lavg[0]);
        else
            daemon_log(DAEMON_INFO, "wakes up...\n");

        if (1 == libdaemon_do_kill) {
            daemon_log(DAEMON_INFO, "received death knoll...");
            destroy_daemon();
            exit( EXIT_SUCCESS );
        }
        sleep(60);
    }

    /* should not end up here - all shutdowns should be via signal */
    return EXIT_SUCCESS;
}

