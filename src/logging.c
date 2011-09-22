/****************************************************************************
 * file: logging.c                                                          *
 * author: kyle isom <coder@kyleisom.net>                                   *
 * created: 2011-09-22                                                      *
 * modified: 2011-09-22                                                     *
 *                                                                          *
 * logging functions for libdaemon                                          *
 *                                                                          *
 * it is released under an ISC / public domain dual-license; see any of the *
 * header files or the file "LICENSE" (or COPYING) under the project root.  *
 ****************************************************************************/
#include <config.h>

#ifdef _LINUX_SOURCE
#include <sys/file.h>
#endif
#include <sys/types.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "daemon.h"

int
daemon_setlog(char *logfile)
{
        int retval = EXIT_FAILURE;
        struct libdaemon_config *daemon_cfg;

        daemon_cfg = daemon_getconfig();

        if (daemon_cfg->logfd > 2)
            close(daemon_cfg->logfd);

        if (NULL != daemon_cfg->logfile) {
            free(daemon_cfg->logfile);
            daemon_cfg->logfile = NULL;
        }

        if (NULL == logfile) {
            daemon_cfg->logfd = -1;
            retval = EXIT_SUCCESS;
        }
        else {
            daemon_cfg->logfd = open(logfile, O_RDWR | O_APPEND | O_CREAT,
                                     0600);
            if (-1 != daemon_cfg->logfd) {
                daemon_cfg->logfile = strdup(logfile);
                if (NULL != daemon_cfg->logfile)
                    retval = EXIT_SUCCESS;
            }
        }

        return retval;
}

int
daemon_log(int priority, const char *message)
{
        int retval = EXIT_FAILURE;
        ssize_t write_sz, buf_sz;
        struct libdaemon_config *daemon_cfg;

        buf_sz = (ssize_t)strlen(message);

        daemon_cfg = daemon_getconfig();

        if (NULL == daemon_cfg->logfile) {
            syslog(priority, message, NULL);
            retval = EXIT_SUCCESS;
        } else {
            write_sz = write(daemon_cfg->logfd, message, buf_sz);
            if (write_sz == buf_sz)
                retval = EXIT_SUCCESS;
            write_sz = write(daemon_cfg->logfd, "\n", 1);
        }
        
        return retval;
}

int
daemon_vlog(int priority, const char *message, ...)
{
        va_list ap;
        ssize_t write_sz, msg_sz;
        struct libdaemon_config *daemon_cfg;

        daemon_cfg = daemon_getconfig();
        msg_sz = (ssize_t)strlen(message);

        va_start(ap, msg);
        
        if (NULL == daemon_cfg->logfile)
            vsyslog(priority, message, va_arg(ap, type));
        else {
            write_sz = write(daemon_cfg->logfd, message, ap);
        }

}
