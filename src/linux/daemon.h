/**************************************************************************
 * daemon.h                                                               *
 * 4096R/B7B720D6  "Kyle Isom <coder@kyleisom.net>"                       *
 * 2011-02-10                                                             *
 *                                                                        *
 * libdaemon: devio.us C library to facilitate daemonising                *
 *                                                                        *
 * this is a small C library providing a few functions to make creating   *
 * daemons under Linux much easier. It is Linux specific and will         *
 * not compile under OpenBSD.                                             *
 *                                                                        *
 **************************************************************************/

#ifndef __LIBDAEMON_H
#define __LIBDAEMON_H

#define DEBUG                             1

/* permission defines */
#define LIBDAEMON_UID                     0             /* run as root        */
#define LIBDAEMON_GID                     1             /* run as daemon      */

/* string length defines */
#define LIBDAEMON_LOG_MAX               128             /* max log msg len    */
#define LIBDAEMON_PID_BUF                 9             /* size of PID buffer */
#define LIBDAEMON_FILENAME_MAX          128             /* max filename len   */
#define LIBDAEMON_D_NAME_MAX_LEN         32             /* max daemon name len*/

/* static strings */
#define LIBDAEMON_BASE_RUNDIR           "/var/run"      /* dir storing pid    *
                                                         *      lockfile      */

/* misc defines */
#define LIBDAEMON_DEATH_KNOLL           SIGUSR1         /* signal to shutdown *
                                                         *      on.           */
#define LIBDAEMON_PIDF_TEST              0x01           /* flag to test for   *
                                                         * presence of pid    *
                                                         * file               */

/* 
 * daemonise: daemonise a program
 *      arguments: a char buffer that is the name of the daemon
 *      returns: EXIT_SUCCESS if the program was successfully daemonised,
 *               EXIT_FAILURE if the program could not be daemonised
 *
 *      by default, the daemonised process may be gracefully shutdown by
 *      sending the process the signal SIGUSR1.
 */
int daemonise(char *);

/*
 * daemon_set_logfile: set the daemon's logfile
 *      arguments: a char buffer with the path to the logfile or NULL if the 
 *                 daemon should log to syslog
 *      returns: EXIT_SUCCESS if the logfile was opened successfully
 *               EXIT_FAILURE is the logfile could not be opened
 */
int daemon_set_logfile(char *);

/* 
 * daemon_log: log a string in the daemon's logfile
 *      arguments: an int indicating the log level (can be any value if not 
 *                 logging to syslog) with -1 defaulting to LOG_INFO, and
 *                 a char buffer with a maximum length of LIBDAEMON_LOG_MAX
 */
void daemon_log(int, char *);

/*
 * daemon_vlog: send a format string to the logfile
 *      arguments: an int indicating the log level (can be any value if not
 *                 logging to syslog) with -1 defaulting to LOG_INFO, a char
 *                 buffer with a maximum length of LIBDAEMON_LOG_MAX holding
 *                 the format string, and a variable number of arguments 
 *                 conforming to the format string. all the standard warnings
 *                 about unsanitised format strings apply.
 */
void daemon_vlog(int, char *, ...);

#endif
