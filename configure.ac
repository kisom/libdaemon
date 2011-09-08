SYSTEM="$(uname -s)"
echo "SYSTEM: ${SYSTEM}"

AC_PREREQ([2.62])
AC_INIT([libdaemon], [2.1], [coder@kyleisom.net])
AC_CONFIG_SRCDIR([src/daemon.c])
AC_CHECK_HEADERS

if test "$(uname -s)" = "Linux" ; then
    AC_SUBST(LIBDAEMON_LINUX, [-D_LINUX_SOURCE])
else
    AC_SUBST(LIBDAEMON_LINUX, [-U_LINUX_SOURCE])
fi

AC_CONFIG_FILES(src/Makefile Makefile doc/Makefile)
AC_PROG_CC
AC_PROG_INSTALL
AC_OUTPUT
    
