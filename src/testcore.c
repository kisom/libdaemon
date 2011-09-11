#include <stdio.h>
#include <stdlib.h>

#include "daemon.h"

static int      test_init(void);
static int      test_run(void);
static int      test_destroy(void);

int 
main()
{
        struct libdaemon_config *cfg;
        int retval;

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
        retval = test_run();

        /* test destroy works */
        retval = test_destroy();

        /* test reinit works */
        retval = test_init();

        /* test destroy works again */
        retval = test_destroy();

        return retval;
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
        printf("[+] destroy retval: %d\n\n", retval);

        return retval;
}

int
test_init()
{
        int retval;
        printf("[+] testing init_daemon(NULL, 0, 0)\n");
        retval = init_daemon(NULL, 0, 0);
        printf("[+] init retval: %d\n\n", retval);

        return retval;
}
