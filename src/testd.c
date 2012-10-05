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

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#include "daemon.h"

#define LOAD_WARN_THRESH                1.5

int
main(void)
{
    double lavg[1];         /* load average */
    int dres, result;

    result = EXIT_FAILURE;

    /* daemonise the program */
    dres = init_daemon(NULL, 0, 0);
    if (EXIT_SUCCESS != dres) {
        syslog(LOG_INFO, "error daemonising!");
        if (LIBDAEMON_DO_NOT_DESTROY != dres) 
            destroy_daemon();
        return EXIT_FAILURE;
    }

    else {
        dres = run_daemon();
        if (EXIT_SUCCESS != dres) {
            syslog(LOG_CRIT, "run_daemon failed!");
            if (LIBDAEMON_DO_NOT_DESTROY != dres)
                destroy_daemon();
            return EXIT_FAILURE;
        }
    }

    while (1) {
        syslog(LOG_INFO, "still running...");
        /* get one minute load average */
        if (-1 == getloadavg(lavg, 1))
            syslog(LOG_INFO, "error retrieving load average!");
        else if (lavg[0] > LOAD_WARN_THRESH)
            syslog(LOG_WARNING, "load average exceeded %f: "
                        "is %f!\n", LOAD_WARN_THRESH, lavg[0]);
        else
            syslog(LOG_INFO, "wakes up...\n");

        if (1 == libdaemon_do_kill) {
            syslog(LOG_INFO, "received death knoll...");
            destroy_daemon();
            exit( EXIT_SUCCESS );
        }
        sleep(60);
    }

    /* should not end up here - all shutdowns should be via signal */
    return EXIT_SUCCESS;
}

