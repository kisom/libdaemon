AC_PREREQ([2.62])
AC_INIT([libdaemon], 
        [3.0], 
        [coder@kyleisom.net], 
        [], 
        [http://www.brokenlcd.net/code/libdaemon/])
AM_INIT_AUTOMAKE
AM_SILENT_RULES([yes])
AC_CONFIG_SRCDIR([src/core.c])
AC_CHECK_HEADERS
AC_TYPE_UID_T
AC_CANONICAL_HOST

dnl Set up include guard.
AH_TOP([#ifndef __LIBDAEMON_CONFIG_H])
AH_TOP([#define __LIBDAEMON_CONFIG_H])
AH_BOTTOM([#endif])

dnl default is not to build for lunix
if test "x${host_os%-gnu}" = xlinux; then
    AC_MSG_NOTICE([Linux system detected, configuring for Linux compatability.])
    AC_DEFINE([_LINUX_SOURCE], [], [build for linux host])
else
    AH_TOP([#undef _GNU_SOURCE    /* make sure this is unset for BSD systems */])
    AC_DEFINE([_BSD_SOURCE], [], [use BSD semantics])
    AC_MSG_NOTICE([Configuring for BSD compatibility.])
fi

AC_CONFIG_FILES(doc/Makefile src/Makefile Makefile)
AC_CONFIG_HEADERS(src/config.h)
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB
AM_PROG_CC_C_O

AC_OUTPUT

if test "x${host_os%-gnu}" = xlinux; then
	OSYS="Linux"
else
	OSYS="BSD"
fi

CONFIGMSG="
===========================================================================
COPYRIGHT / LICENSE:
$(cat LICENSE)

configuration results:
	host: ${host_os}
	library type: ${OSYS}
        build flag: ${LIBDAEMON_LINUX}

	installation prefix (\$prefix): $prefix
	library will be installed in $libdir/libdaemon.a
	daemon.h will be installed in ${includedir}
===========================================================================
"
echo "$CONFIGMSG"

