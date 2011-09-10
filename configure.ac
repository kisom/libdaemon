PACKAGE=[libdaemon]
VERSION=[2.1]
BUGSREPORT=[coder@kyleisom.net]

AC_PREREQ([2.62])
AC_CANONICAL_SYSTEM
AC_INIT(${PACKAGE}, ${VERSION}, ${BUGSREPORT})
AM_INIT_AUTOMAKE(silent-rules)
AC_CONFIG_SRCDIR([src/daemon.c])
AC_CHECK_HEADERS

case "${target_os}" in
  linux*)
    AC_SUBST(LIBDAEMON_LINUX, [-D_LINUX_SOURCE])
  openbsd*)
    AC_SUBST(LIBDAEMON_LINUX, [-U_LINUX_SOURCE])
  *)
    AC_MSG_WARN([
============================================================================
This code has not been tested with ${target_os}. Assuming a BSD install, but
you have been warned that this might not work as expected.
============================================================================])
)
    AC_SUBST(LIBDAEMON_LINUX, [-U_LINUX_SOURCE])
esac

AC_CONFIG_FILES(src/Makefile Makefile)
AC_PROG_CC
AC_PROG_INSTALL
AC_OUTPUT
    
echo "target os: ${target_os}"
