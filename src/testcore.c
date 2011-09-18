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
