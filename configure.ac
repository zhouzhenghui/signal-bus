#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.62])
AC_INIT([signalbus], [0.1], [zhouzhenghui@gmail.com])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/continuation.c])
AC_CONFIG_HEADERS([src/config.h])
AM_INIT_AUTOMAKE(1.10 gnu -Wall no-define)
AC_PROG_RANLIB
AC_GNU_SOURCE
AC_USE_SYSTEM_EXTENSIONS
AM_PROG_AR

AH_TOP([#define HAVE_CONFIGURED 1])

#### Determine host OS ####

os_is_linux=0
os_is_win32=0
os_is_darwin=0

AC_MSG_CHECKING([host operating system])
case "$host_os" in
    linux*)
        AC_MSG_RESULT([linux])
        os_is_linux=1
    ;;
    darwin*)
        AC_MSG_RESULT([darwin])
        os_is_darwin=1
        AC_DEFINE([OS_IS_DARWIN], 1, [Build target is Darwin.])
    ;;
    mingw*)
        AC_MSG_RESULT([win32])
        os_is_win32=1
        AC_DEFINE([OS_IS_WIN32], 1, [Build target is Windows.])
    ;;
    *)
        AC_MSG_RESULT([unknown])
    ;;
esac

AM_CONDITIONAL(OS_IS_DARWIN, test "x$os_is_darwin" = "x1")
AM_CONDITIONAL(OS_IS_WIN32, test "x$os_is_win32" = "x1")
AC_SUBST([OS_IS_WIN32], [$os_is_win32])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
m4_pattern_allow([AM_PROG_AR], [AM_PROG_AR])

# Checks for libraries.

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([inttypes.h limits.h malloc.h memory.h stddef.h stdint.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC

# check pthread support
AX_PTHREAD([have_pthread=yes], [have_pthread=no])
AS_IF([test "x$have_pthread" = "xyes"], [AC_DEFINE([HAVE_PTHREAD], 1)])
AM_CONDITIONAL([HAVE_PTHREAD], [test "x$have_pthread" = "xyes"])
AM_CONDITIONAL([HAVE_SELECT], [test "x$have_pthread" = "xyes" -a "x$ac_cv_func_select" = "xyes"])

# gcc arch flag
AX_GCC_ARCHFLAG([no])

if test "x$GCC" != "xyes"; then
  AC_MSG_CHECKING([for __builtin_return_address])
  AC_TRY_LINK([], [
          __builtin_return_address(0);
      ], [
          AC_MSG_RESULT(yes)
          AC_DEFINE(HAVE_BUILTIN_RETURN_ADDRESS, 1,
                  [Whether we have __builtin_return_address])
          AC_MSG_RESULT(no)
    ])
fi

AC_SUBST([PICFLAG])

AC_CONFIG_FILES([Makefile src/Makefile src/continuation/Makefile tests/Makefile \
        pkgconfig/signalbus.pc pkgconfig/signalbus-uninstalled.pc
  ])

AC_OUTPUT
