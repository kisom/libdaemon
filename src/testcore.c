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

#include <stdio.h>
#include <stdlib.h>

#include "daemon.h"

static int      test_init(void);
static int      test_run(void);
static int      test_destroy(void);
static void     test_battery(void);

extern char     *__progname;

int 
main(int argc, char **argv)
{
        size_t i = 0;
        int retval = EXIT_FAILURE;
        int tests  = 1;

        if (argc > 1)
            tests = atoi(argv[1]);

        for (i = 0; i < tests; ++i) {
            printf("test #%d out of %d\n-----------------\n\n", 
                   (int)i + 1, tests);
            test_battery();
        }

        return retval;
}

void
test_battery()
{
        int retval;
        struct libdaemon_config *cfg;

        /* ensure run before init fails */
        retval = test_run();

        /* ensure destroy before init fails */
        retval = test_destroy();

        /* ensure init works */
        retval = test_init();

        cfg = daemon_getconfig();
        if ((NULL != cfg) && (NULL != cfg->rundir))
            printf("[+] rundir: %s\n\n", cfg->rundir);

        /* ensure double init fails */
        retval = test_init();

        /* test run works */
//        retval = test_run();

        /* test destroy works */
        retval = test_destroy();

}

int 
test_run()
{
        int retval;

        printf("[+] run_daemon():\n");
        retval = run_daemon();
        printf("\trun retval: %d\n\n", retval);

        return retval;
}

int
test_destroy()
{
        int retval;
        printf("[+] destroy_daemon():\n");
        retval = destroy_daemon();
        printf("\t[+] destroy retval: %d\n\n", retval);

        return retval;
}

int
test_init()
{
        int retval;
        printf("[+] testing init_daemon(NULL, 0, 0)\n");
        retval = init_daemon(NULL, 0, 0);
        printf("\t[+] init retval: %d\n\n", retval);

        return retval;
}

void
usage()
{
    fprintf(stderr, "Usage: %s [number of tests to run]\n", __progname);

}
