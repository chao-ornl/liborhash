AC_PREREQ([2.69])
AC_INIT([liborhash], [0.1], [vallegr@ornl.gov])

AM_INIT_AUTOMAKE([dist-bzip2 subdir-objects foreign tar-ustar])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MKDIR_P
AM_PROG_CC_C_O
m4_pattern_allow([AM_PROG_AR], [AM_PROG_AR])
LT_INIT

AC_ARG_WITH([mhash],
            [AS_HELP_STRING([--with-mhash=DIR],
            [use mhash available in DIR])])
if test -n "$with_mhash"; then
    AC_MSG_CHECKING([for mhash])
    if test -f "$with_mhash/lib/libmhash.a"; then
        LIBMHASH="-L$with_mhash/lib"
        LDFLAGS="$LDFLAGS $LIBMHASH"
    else
        with_mhash=no
    fi
    if test -f "$with_mhash/include/mhash.h"; then
        MHASH_INCL="-I$with_mhash/include"
        CFLAGS="$GFLAGS $MHASH_INCL"
    else
        with_mhash=no
    fi
else
    with_mhash=no
fi

if test x"$with_mhash" = x"no"; then
    AC_MSG_RESULT([no])
else
    AC_MSG_RESULT([yes])
fi

CPPFLAGS_save="$CPPFLAGS"
top_srcdir=`pwd`
AC_SUBST([CPPFLAGS],["-I$top_srcdir/include $CPPFLAGS_save"])

AC_CONFIG_FILES([Makefile src/Makefile test/Makefile])

AC_ENABLE_SHARED
AC_DISABLE_STATIC

AC_PROG_LIBTOOL(libtool)

AC_OUTPUT
