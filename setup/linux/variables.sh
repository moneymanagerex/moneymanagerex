#!/bin/bash

# Specify the build version of mmex
MMEX_VERSION="1.3.1"
MMEX_RELEASE_DATE="2017-01-05"
MMEX_EMAIL="moneymanagerex@moneymanagerex.org"
MMEX_HOMEPAGE="http://www.moneymanagerex.org"   
#${shlibs:Depends}, ${misc:Depends}, 
MMEX_DEB_DEPENDS="libc6 (>= 2.3.5-1), libstdc++6, libwxgtk3.0-0 (>= 3.0.0), libwxgtk-media3.0-0 (>=3.0.0), libwxgtk-webview3.0-0 (>= 3.0.0)"
MMEX_RPM_DEPENDS=""
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
