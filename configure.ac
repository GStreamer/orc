AC_PREREQ([2.68])
AC_INIT([orc], [0.4.27], [http://bugzilla.gnome.org/enter_bug.cgi?product=GStreamer&component=orc],[orc])
dnl don't forget to update libversion

AS_NANO(ORC_GIT=no,ORC_GIT=yes)

AC_CANONICAL_HOST()
AM_INIT_AUTOMAKE([-Wno-portability 1.11 no-dist-gzip dist-xz tar-ustar subdir-objects])

dnl use pretty build output with automake >= 1.11
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])],
  [AM_DEFAULT_VERBOSITY=1
   AC_SUBST(AM_DEFAULT_VERBOSITY)])

ORC_MAJORMINOR=0.4
AC_SUBST(ORC_MAJORMINOR)

AC_CONFIG_HEADER(config.h)

dnl CURRENT, REVISION, AGE
dnl - library source changed -> increment REVISION
dnl - interfaces added/removed/changed -> increment CURRENT, REVISION = 0
dnl - interfaces added -> increment AGE
dnl - interfaces removed -> AGE = 0
ORC_LIBVERSION="27:0:27"
AC_SUBST(ORC_LIBVERSION)
LT_PREREQ([2.2.6])
LT_INIT([dlopen win32-dll disable-static])

AC_PROG_CC
AC_PROG_CC_STDC
AM_PROG_CC_C_O
AM_PROG_AS

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_SRCDIR([orc/orc.h])

AM_MAINTAINER_MODE([enable])

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
GTK_DOC_CHECK([1.12])

AS_HOST_DEFINES()

AC_C_BIGENDIAN

AC_FUNC_MMAP()

AC_HEADER_STDC
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

dnl Android liblog
case "${host_os}" in
  *android*)
    AC_CHECK_LIB(log, __android_log_print,
        AC_DEFINE(HAVE_ANDROID_LIBLOG, 1, [Defined if we have __android_log_print()])
        LIBLOG=-llog
    )
    ;;
  *)
    ;;
esac
AC_SUBST(LIBLOG)

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
if test "x$ORC_GIT" = "xyes"
then
  AS_COMPILER_FLAG(-Werror, ERROR_CFLAGS="-Werror")
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
  *android*)
    AC_DEFINE(HAVE_THREAD_PTHREAD, 1, [Use pthread thread code])
    PTHREAD_CFLAGS=
    PTHREAD_LIBS=
    ;;
  *)
    AC_DEFINE(HAVE_THREAD_PTHREAD, 1, [Use pthread thread code])
    PTHREAD_CFLAGS=
    PTHREAD_LIBS=-lpthread
    ;;
esac
AC_SUBST(PTHREAD_CFLAGS)
AC_SUBST(PTHREAD_LIBS)

AC_ARG_ENABLE(backend,
  AC_HELP_STRING([--enable-backend],[sse,mmx,neon,mips,all (default all)]),
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
### ARM backend is disabled until it has decent coverage
#  arm)
#    ENABLE_BACKEND_ARM=yes
#    AC_DEFINE(ENABLE_BACKEND_ARM, 1, [Enable Arm backend])
#    ;;
  c64x)
    ENABLE_BACKEND_C64X=yes
    AC_DEFINE(ENABLE_BACKEND_C64X, 1, [Enable c64x backend])
    ;;
  mips)
    ENABLE_BACKEND_MIPS=yes
    AC_DEFINE(ENABLE_BACKEND_MIPS, 1, [Enable MIPS backend])
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
#    ENABLE_BACKEND_ARM=yes
#    AC_DEFINE(ENABLE_BACKEND_ARM, 1, [Enable Arm backend])
    ENABLE_BACKEND_C64X=yes
    AC_DEFINE(ENABLE_BACKEND_C64X, 1, [Enable c64x backend])
    ENABLE_BACKEND_MIPS=yes
    AC_DEFINE(ENABLE_BACKEND_MIPS, 1, [Enable MIPS backend])
    ;;
esac
AM_CONDITIONAL(ENABLE_BACKEND_SSE, test "x$ENABLE_BACKEND_SSE" = "xyes")
AM_CONDITIONAL(ENABLE_BACKEND_MMX, test "x$ENABLE_BACKEND_MMX" = "xyes")
AM_CONDITIONAL(ENABLE_BACKEND_ALTIVEC, test "x$ENABLE_BACKEND_ALTIVEC" = "xyes")
AM_CONDITIONAL(ENABLE_BACKEND_NEON, test "x$ENABLE_BACKEND_NEON" = "xyes")
AM_CONDITIONAL(ENABLE_BACKEND_ARM, test "x$ENABLE_BACKEND_ARM" = "xyes")
AM_CONDITIONAL(ENABLE_BACKEND_C64X, test "x$ENABLE_BACKEND_C64X" = "xyes")
AM_CONDITIONAL(ENABLE_BACKEND_MIPS, test "x$ENABLE_BACKEND_MIPS" = "xyes")

dnl Check for -Bsymbolic-functions linker flag used to avoid
dnl intra-library PLT jumps, if available.
AC_ARG_ENABLE(Bsymbolic,
              [AS_HELP_STRING([--disable-Bsymbolic],[avoid linking with -Bsymbolic])],,
              [SAVED_LDFLAGS="${LDFLAGS}" SAVED_LIBS="${LIBS}"
               AC_MSG_CHECKING([for -Bsymbolic-functions linker flag])
               LDFLAGS=-Wl,-Bsymbolic-functions
               LIBS=
               AC_TRY_LINK([], [return 0],
                           AC_MSG_RESULT(yes)
                           enable_Bsymbolic=yes,
                           AC_MSG_RESULT(no)
                           enable_Bsymbolic=no)
               LDFLAGS="${SAVED_LDFLAGS}" LIBS="${SAVED_LIBS}"])

AC_DEFINE(ORC_EXPORTS, 1, [Defined for compiling internal code])

ORC_CFLAGS="$ORC_CFLAGS \$(ERROR_CFLAGS) -I\$(top_srcdir) -D_GNU_SOURCE"
AC_SUBST(ERROR_CFLAGS)
AC_SUBST(ORC_CFLAGS)

ORC_LIBS="\$(top_builddir)/orc/liborc-$ORC_MAJORMINOR.la $LIBM $LIBRT"
AC_SUBST(ORC_LIBS)

#CFLAGS=`echo $CFLAGS | sed s/O2/O0/`

ORC_LDFLAGS="-version-info \$(ORC_LIBVERSION) -no-undefined -export-symbols-regex \^orc_"
if test "x${enable_Bsymbolic}" = "xyes"; then
  ORC_LDFLAGS="$ORC_LDFLAGS -Wl,-Bsymbolic-functions"
fi
AC_SUBST(ORC_LDFLAGS)

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

