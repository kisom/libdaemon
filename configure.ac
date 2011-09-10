AC_PREREQ([2.62])
AC_INIT([libdaemon], [2.2alpha], [coder@kyleisom.net])
AM_INIT_AUTOMAKE([silent-rules])
AM_SILENT_RULES
AC_CONFIG_SRCDIR([src/daemon.c])
AC_CHECK_HEADERS
AC_MSG_CHECKING([canonical host])
AC_CANONICAL_HOST

dnl default is not to build for lunix
AC_SUBST(LIBDAEMON_LINUX, [-U_LINUX_SOURCE])

if test "x${host_os%-gnu}" = xlinux; then
    AC_MSG_NOTICE([Linux system detected, configuring for Linux compatability.])
    AC_SUBST(LIBDAEMON_LINUX, [-D_LINUX_SOURCE])
else
    AC_MSG_NOTICE([Configuring for BSD compatibility.])
fi

AC_CONFIG_FILES(src/Makefile Makefile)
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_OUTPUT

if test "x${LIBDAEMON_LINUX}" = "x-U_LINUX_SOURCE"; then
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

	installation prefix (\$prefix): $prefix
	exec prefix (\$exec_prefix): $exec_prefix
	library will be installed in $libdir/libdaemon.a
	daemon.h will be installed in ${includedir}
===========================================================================
"
echo "$CONFIGMSG"

