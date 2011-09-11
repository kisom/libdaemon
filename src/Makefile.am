lib_LIBRARIES = libdaemon.a
include_HEADERS = daemon.h
check_PROGRAMS = testd 
libdaemon_a_SOURCES = daemon.c daemon.h
libdaemon_a_CFLAGS = @LIBDAEMON_LINUX@
testd_SOURCES = testd.c
testd_LDADD = libdaemon.a

