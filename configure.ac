dnl @configure_input@
dnl

AC_PREREQ([2.69])
AC_INIT([Hoedown],[3.0.3],
  [marco.maggi-ipsu@poste.it],
  [hoedown],[http://github.com/marcomaggi/hoedown/])
AC_CONFIG_SRCDIR([src/])
AC_CONFIG_MACRO_DIR([meta/autotools])
AC_CONFIG_AUX_DIR([meta/autotools])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([foreign dist-xz no-dist-gzip subdir-objects])
AM_MAINTAINER_MODE

AM_PROG_AR
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_MKDIR_P

LT_PREREQ([2.4])
LT_INIT

dnl page
#### libraries interface version

dnl This is the version stored in the pkg-config data file.
AC_SUBST([HOEDOWN_PKG_CONFIG_VERSION],[3.0.3])

MM_LIBTOOL_LIBRARY_VERSIONS([hoedown],3,0,0)

dnl page
#### basic system inspection

AC_LANG([C])
dnl AC_DEFINE([_GNU_SOURCE],[1],[enable everything GNU])
AC_PROG_CC_C99
AM_PROG_AS
dnl AC_PROG_CC
AM_PROG_CC_C_O
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([assert.h stdint.h strings.h errno.h stddef.h])
dnl AC_CHECK_SIZEOF(void *)
AC_C_CONST
AC_C_INLINE
AC_C_RESTRICT
dnl AC_TYPE_LONG_LONG_INT
dnl AC_TYPE_UNSIGNED_LONG_LONG_INT
dnl AC_TYPE_SIZE_T
dnl AC_TYPE_SSIZE_T
dnl AC_TYPE_INT8_T
dnl AC_TYPE_INT16_T
dnl AC_TYPE_INT32_T
dnl AC_TYPE_INT64_T
dnl AC_TYPE_UINT8_T
dnl AC_TYPE_UINT16_T
dnl AC_TYPE_UINT32_T
dnl AC_TYPE_UINT64_T

AC_CACHE_SAVE

dnl page
#### checks for library functions

AC_FUNC_MALLOC
AC_FUNC_MEMCMP
dnl AC_CHECK_FUNCS([memmove memset strerror])
dnl AC_CHECK_FUNCS([mmap munmap msync mremap madvise mlock munlock mlockall munlockall mprotect])
dnl AC_CHECK_FUNCS([getenv])

AC_CACHE_SAVE

dnl page
#### external libraries

dnl AX_PTHREAD(,[AC_MSG_ERROR([cannot find pthread library],[1])])

dnl PKG_CHECK_MODULES([GSL],[gsl >= 1.16],[gsl=yes],[gsl=no])
dnl GSL_CPPFLAGS=$(pkg-config gsl --cflags)
dnl GSL_LDFLAGS=$(pkg-config gsl --libs)

dnl AC_CHECK_HEADERS([gmp.h],,[AC_MSG_ERROR([Cannot find libgmp.])])
dnl AC_CHECK_LIB([gmp],[__gmpn_add],,[AC_MSG_ERROR([Cannot find libgmp.])])

dnl AC_CACHE_SAVE

dnl page
#### operating system specific configuration

dnl case "$target_os" in
dnl   *linux*)
dnl     ;;
dnl   *bsd*)
dnl     ;;
dnl   *cygwin*)
dnl     ;;
dnl   *darwin*)
dnl     hoedown_OS_DARWIN=yes
dnl     ;;
dnl esac
dnl AM_CONDITIONAL([WANT_LINUX], [test "x$hoedown_OS_LINUX"  = xyes])
dnl AM_CONDITIONAL([WANT_BSD],   [test "x$hoedown_OS_BSD"    = xyes])
dnl AM_CONDITIONAL([WANT_CYGWIN],[test "x$hoedown_OS_CYGWIN" = xyes])
dnl AM_CONDITIONAL([WANT_DARWIN],[test "x$hoedown_OS_DARWIN" = xyes])

dnl page
#### finish

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile]
   [meta/scripts/hoedown.pc])
AC_OUTPUT

### end of file
# Local Variables:
# mode: autoconf
# page-delimiter: "^dnl page"
# End:
