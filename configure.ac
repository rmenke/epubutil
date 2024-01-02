# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])

AC_COPYRIGHT([Copyright (C) 2024 by Rob Menke.  Please see the
accompanying LICENSE.txt file for details.])

AC_INIT([EPUB Utilities],[m4_esyscmd_s([perl git-semver])],[bug-AC_PACKAGE_TARNAME@the-wabe.com],[epubutil],[http://www.the-wabe.com/AC_PACKAGE_TARNAME/])

AC_SUBST([PACKAGE_DESCRIPTION],["EPUB CLI Utilities"])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIRS([m4])

AM_INIT_AUTOMAKE([-Wall foreign nostdinc subdir-objects])

AM_PROG_AR

LT_INIT
LT_LANG([C++])

AX_CXX_COMPILE_STDCXX([20],[noext],[mandatory])

AC_REQUIRE_AUX_FILE([tap-driver.sh])

AC_ARG_ENABLE([epubcheck],[AS_HELP_STRING([--enable-epubcheck@<:@=PROG@:>@],[use epubcheck (or PROG) for validation tests])],[],[enable_epubcheck=yes])

AS_IF([test "x$enable_epubcheck" != xno],[
    AS_IF([test "x$enable_epubcheck" != xyes],[EPUBCHECK="$enable_epubcheck"])
    AC_CHECK_PROGS([EPUBCHECK],[epubcheck])
    AS_IF([test "x$EPUBCHECK" != x],[
        AC_DEFINE_UNQUOTED([EPUBCHECK],["$EPUBCHECK"])
    ])
])

AX_CODE_COVERAGE

AC_CONFIG_FILES([Makefile test/Makefile Doxyfile])

AC_OUTPUT

