#!/bin/bash

# Specify the build version of mmex
MMEX_VERSION="1.3.0"
MMEX_RELEASE_DATE="2015-02-09"
MMEX_EMAIL="moneymanagerex@moneymanagerex.org"
MMEX_HOMEPAGE="http://www.moneymanagerex.org"   
#${shlibs:Depends}, ${misc:Depends}, 
MMEX_DEB_DEPENDS="libc6 (>= 2.3.5-1), libwxgtk3.0-0 (>= 3.0.0)"
MMEX_RPM_DEPENDS=""
MMEX_DEB_BUILD_DEPENDS="libwxgtk3.0-dev (>= 3.0.0), libwebkitgtk-dev (>= 3.0.0), debhelper (>= 8.0.0), autotools-dev, python-dev (>= 2.7.3), libreadline-dev (>=6.2), gcc-4.7-base (>=4.7)"
MMEX_RPM_BUILD_DEPENDS="bakefile, automake, gcc, gcc-c++, gettext, desktop-file-utils, wxGTK3-devel >= 3.0.0"
MMEX_SUMMARY="Simple to use financial management software"
MMEX_DESCRIPTION="Simple to use financial management software
 Money Manager Ex (MMEX) is a free, open-source,
 cross-platform, easy-to-use personal finance software.
 It primarily helps organize one's finances and keeps
 track of where, when and how the money goes.
 MMEX includes all the basic features that 90% of users
 would want to see in a personal finance application.
 The design goals are to concentrate on simplicity
 and user friendliness - something one can use everyday."
