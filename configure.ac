AC_PREREQ([2.58])
AC_INIT(orc,0.4.14)
dnl don't forget to update libversion

AS_NANO(ORC_CVS=no,ORC_CVS=yes)

AC_CANONICAL_HOST([])
AM_INIT_AUTOMAKE(1.6)

ORC_MAJORMINOR=0.4
AC_SUBST(ORC_MAJORMINOR)

AM_CONFIG_HEADER(config.h)

dnl CURRENT, REVISION, AGE
dnl - library source changed -> increment REVISION
dnl - interfaces added/removed/changed -> increment CURRENT, REVISION = 0
dnl - interfaces added -> increment AGE
dnl - interfaces removed -> AGE = 0
ORC_LIBVERSION="14:0:14"
AC_SUBST(ORC_LIBVERSION)
AC_LIBTOOL_WIN32_DLL
AC_LIBTOOL_TAGS([])
AC_PROG_LIBTOOL
AM_PROG_CC_C_O
AM_PROG_AS
AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_SRCDIR([orc/orc.h])

ACLOCAL_AMFLAGS="-I m4 $ACLOCAL_AMFLAGS"
AC_SUBST(ACLOCAL_AMFLAGS)

AM_CONDITIONAL(CROSS_COMPILING, test "x$cross_compiling" = "xyes")
AC_CHECK_FUNCS([posix_memalign])

##################################################
# Check for gtk-doc.
##################################################

if test "x$cross_compiling" = "xyes" ; then
  enable_gtk_doc=no
fi
GTK_DOC_CHECK([1.0])


AS_HOST_DEFINES()

AC_C_BIGENDIAN

AC_FUNC_MMAP()

AC_CHECK_HEADERS([inttypes.h])
AC_CHECK_HEADERS([sys/time.h])
AC_CHECK_HEADERS([unistd.h])

AC_CHECK_FUNCS([gettimeofday])
AC_CHECK_FUNCS([sigaction])
AC_CHECK_FUNCS([sigsetjmp])

AC_CHECK_LIBM
AC_SUBST(LIBM)

AC_CHECK_LIB(rt, clock_gettime,
   AC_DEFINE(HAVE_CLOCK_GETTIME, 1, [Defined if we have clock_gettime()])
   LIBRT=-lrt
   )
AC_SUBST(LIBRT)

AC_CACHE_CHECK(for monotonic clocks,
    orc_cv_monotonic_clock,AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
#include <time.h>
#include <unistd.h>
int main() {
#if !(defined(_POSIX_MONOTONIC_CLOCK) && _POSIX_MONOTONIC_CLOCK >= 0 && defined(CLOCK_MONOTONIC))
        #error No monotonic clock
#endif
        return 0;
}
]])],orc_cv_monotonic_clock=yes,orc_cv_monotonic_clock=no))
if test "$orc_cv_monotonic_clock" = "yes"; then
  AC_DEFINE(HAVE_MONOTONIC_CLOCK,1,[Defined if we have a monotonic clock])
fi

AC_CHECK_HEADERS([valgrind/valgrind.h])

AS_COMPILER_FLAG(-Wall, ORC_CFLAGS="$ORC_CFLAGS -Wall")
if test "x$ORC_CVS" = "xyes"
then
  AS_COMPILER_FLAG(-Werror, ORC_CFLAGS="$ORC_CFLAGS -Werror")
fi

case "${host_os}" in
  nobody_is_using_this_currently)
    AC_DEFINE(HAVE_CODEMEM_MALLOC, 1, [Use malloc to allocate code for execution])
    ;;
  mingw*|pw32*|cygwin*)
    AC_DEFINE(HAVE_CODEMEM_VIRTUALALLOC, 1, [Use VirtualAlloc to allocate code for execution])
    ;;
  linux*|darwin*|solaris*|netbsd*|freebsd*|openbsd*|kfreebsd*|dragonfly*|gnu*)
    AC_DEFINE(HAVE_CODEMEM_MMAP, 1, [Use mmap to allocate code for execution])
    ;;
  *)
    AC_ERROR([no code allocation backend])
    ;;
esac

case "${host_os}" in
  mingw*|pw32*|cygwin*)
    AC_DEFINE(HAVE_THREAD_WIN32, 1, [Use Windows thread code])
    ;;
  *)
    AC_DEFINE(HAVE_THREAD_PTHREAD, 1, [Use pthread thread code])
    ;;
esac

AC_ARG_ENABLE(backend,
  AC_HELP_STRING([--enable-backend],[sse,mmx,neon,arm,all (default all)]),
    [], [enable_backend=all])
case "${enable_backend}" in
  sse)
    ENABLE_BACKEND_SSE=yes
    AC_DEFINE(ENABLE_BACKEND_SSE, 1, [Enable SSE backend])
    ;;
  mmx)
    ENABLE_BACKEND_MMX=yes
    AC_DEFINE(ENABLE_BACKEND_MMX, 1, [Enable MMX backend])
    ;;
  altivec)
    ENABLE_BACKEND_ALTIVEC=yes
    AC_DEFINE(ENABLE_BACKEND_ALTIVEC, 1, [Enable Altivec backend])
    ;;
  neon)
    ENABLE_BACKEND_NEON=yes
    AC_DEFINE(ENABLE_BACKEND_NEON, 1, [Enable NEON backend])
    ;;
  arm)
    ENABLE_BACKEND_ARM=yes
    AC_DEFINE(ENABLE_BACKEND_ARM, 1, [Enable Arm backend])
    ;;
  c64x)
    ENABLE_BACKEND_C64X=yes
    AC_DEFINE(ENABLE_BACKEND_C64X, 1, [Enable c64x backend])
    ;;
  all|auto)
    ENABLE_BACKEND_SSE=yes
    AC_DEFINE(ENABLE_BACKEND_SSE, 1, [Enable SSE backend])
    ENABLE_BACKEND_MMX=yes
    AC_DEFINE(ENABLE_BACKEND_MMX, 1, [Enable MMX backend])
    ENABLE_BACKEND_ALTIVEC=yes
    AC_DEFINE(ENABLE_BACKEND_ALTIVEC, 1, [Enable Altivec backend])
    ENABLE_BACKEND_NEON=yes
    AC_DEFINE(ENABLE_BACKEND_NEON, 1, [Enable NEON backend])
    ENABLE_BACKEND_ARM=yes
    AC_DEFINE(ENABLE_BACKEND_ARM, 1, [Enable Arm backend])
    ENABLE_BACKEND_C64X=yes
    AC_DEFINE(ENABLE_BACKEND_C64X, 1, [Enable c64x backend])
    ;;
esac
AM_CONDITIONAL(ENABLE_BACKEND_SSE, test "x$ENABLE_BACKEND_SSE" = "xyes")
AM_CONDITIONAL(ENABLE_BACKEND_MMX, test "x$ENABLE_BACKEND_MMX" = "xyes")
AM_CONDITIONAL(ENABLE_BACKEND_ALTIVEC, test "x$ENABLE_BACKEND_ALTIVEC" = "xyes")
AM_CONDITIONAL(ENABLE_BACKEND_NEON, test "x$ENABLE_BACKEND_NEON" = "xyes")
AM_CONDITIONAL(ENABLE_BACKEND_ARM, test "x$ENABLE_BACKEND_ARM" = "xyes")
AM_CONDITIONAL(ENABLE_BACKEND_C64X, test "x$ENABLE_BACKEND_C64X" = "xyes")


AC_DEFINE(ORC_EXPORTS, 1, [Defined for compiling internal code])

ORC_CFLAGS="$ORC_CFLAGS -I\$(top_srcdir)"
AC_SUBST(ORC_CFLAGS)

ORC_LIBS="\$(top_builddir)/orc/liborc-$ORC_MAJORMINOR.la $LIBM $LIBRT"
AC_SUBST(ORC_LIBS)

#CFLAGS=`echo $CFLAGS | sed s/O2/O0/`

pkgconfigdir="\$(libdir)/pkgconfig"
AC_SUBST(pkgconfigdir)


AC_CONFIG_FILES([
Makefile 
doc/Makefile
doc/version.entities
orc/Makefile
orc-test/Makefile
testsuite/Makefile
testsuite/orcc/Makefile
testsuite/benchmorc/Makefile
examples/Makefile
tools/Makefile
orc-uninstalled.pc
orc.pc
])
AC_OUTPUT



