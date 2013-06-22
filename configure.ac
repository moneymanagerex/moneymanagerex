#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
##########################################################################

AC_PREREQ(2.53)
AC_INIT([MoneyManagerEx], [0.9.9.2], [madhan@sourceforge.net])

##########################################################################

AC_CONFIG_SRCDIR([src/mmex.cpp])
AC_CONFIG_AUX_DIR([auxd])
AC_CONFIG_MACRO_DIR([m4])

##########################################################################

# Standard Checks

AC_CANONICAL_SYSTEM

##########################################################################

# Checks for programs

AC_PROG_CXX
AC_PROG_CC
AC_PROG_CXXCPP
AC_PROG_CPP
AC_PROG_RANLIB
AC_PROG_INSTALL
#AC_PROG_LIBTOOL

##########################################################################

# Checks for compilation model

AC_SYS_LARGEFILE

# Checks C++11
AX_CXX_COMPILE_STDCXX_11([noext],[mandatory])

##########################################################################

# Add wxConfig Options to Configure

AM_OPTIONS_WXCONFIG
AM_OPTIONS_WXPRESETS

AM_WXPRESETS_FULL([2.9.4], [base, core, html, xml, net, aui, adv, qa])

##########################################################################

# Process standard bakefile
AC_BAKEFILE([m4_include(m4/autoconf_inc.m4)])

AC_CONFIG_FILES([Makefile  src/tests/Makefile  ${srcdir}/po/Makefile  ${srcdir}/setup/win/Makefile])

AC_OUTPUT

##########################################################################

AM_WXPRESETS_MSG_BEGIN
AM_WXPRESETS_MSG
AM_WXPRESETS_MSG_END

##########################################################################
