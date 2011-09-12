AM_CFLAGS = -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align \
            -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations   \
            -Wredundant-decls -Wnested-externs -Winline -Wno-long-long    \
            -Wconversion -Wstrict-prototypes -g -O0
lib_LIBRARIES = libdaemon.a
include_HEADERS = daemon.h
check_PROGRAMS = testcore testd
libdaemon_a_SOURCES = core.c rundir.c
testcore_SOURCES = testcore.c
testcore_LDADD = libdaemon.a
testcore_CFLAGS = -O0
testd_SOURCES = testd.c
testd_LDADD = libdaemon.a

