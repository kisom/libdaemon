AC_PREREQ([2.62])
AC_INIT([libdaemon], [2.2alpha], [coder@kyleisom.net])
AM_INIT_AUTOMAKE(silent-rules)
AC_CONFIG_SRCDIR([src/daemon.c])
AC_CHECK_HEADERS
AC_CANONICAL_HOST

dnl default is not to build for lunix
AC_SUBST(LIBDAEMON_LINUX, [-U_LINUX_SOURCE])

echo "*** target os: ${host_os}"
if test "x${host_os%-gnu}" = xlinux; then
    AC_SUBST(LIBDAEMON_LINUX, [-D_LINUX_SOURCE])
elif test "x${host_os%[[:digit:]]\.[[:digit:]]}" = xopenbsd; then
    echo ""
else
    AC_MSG_WARN([
============================================================================
This code has not been tested with ${target_os}. Assuming a BSD install, but
you have been warned that this might not work as expected.
============================================================================])
fi

AC_CONFIG_FILES(src/Makefile Makefile)
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_OUTPUT
    
