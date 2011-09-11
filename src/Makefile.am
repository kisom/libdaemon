lib_LIBRARIES = libdaemon.a
include_HEADERS = daemon.h
check_PROGRAMS = testd 
libdaemon_a_SOURCES = core.o 
testd_SOURCES = testd.c
testd_LDADD = libdaemon.a

CLEANFILES = *.o *.a

