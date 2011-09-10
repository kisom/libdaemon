AC_PREREQ([2.62])
AC_INIT([libdaemon], [2.1], [coder@kyleisom.net])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/daemon.c])
AC_CONFIG_FILES([src/Makefile Makefile])
AC_CHECK_HEADERS
AC_SUBST(LIBDAEMON_LINUX, [-U_LINUX_SOURCE]) 

AC_MSG_CHECKING([checking operating system])
target_os="$(uname -s)"
if test "x${target_os}" = xLinux ; then
    AC_MSG_RESULT([Lunix])
    AC_SUBST(LIBDAEMON_LINUX, [-D_LINUX_SOURCE])
elif test "x${target_os%[[:digit:]].[[:digit:]]}" = xOpenBSD ; then
    AC_MSG_RESULT([OpenBSD])
else
    AC_MSG_RESULT([unknown])
    AC_MSG_WARN([
============================================================================
This code has not been tested with ${target_os}. Assuming a BSD install, but
you have been warned that this might not work as expected.
============================================================================])
fi
AC_PROG_RANLIB
AC_PROG_CC
AC_PROG_INSTALL
AC_OUTPUT

