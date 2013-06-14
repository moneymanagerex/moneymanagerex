dnl ---------------------------------------------------------------------------
dnl Author:          Francesco Montorsi
dnl                  (contributions by David Hart and Mac-testing by Bob McCown)
dnl Creation date:   13/9/2005
dnl RCS-ID:          $Id: wxpresets.m4,v 1.11 2006/03/10 20:03:13 frm Exp $
dnl Purpose:         Some M4 macros which makes *much* easier writing the
dnl                  configure.ac files for build system based on wxpresets
dnl ---------------------------------------------------------------------------


dnl ---------------------------------------------------------------------------
dnl Macros for wxWidgets build configuration detection.
dnl Typically used in configure.in/ac as:
dnl
dnl     # basic configure checks
dnl     ...
dnl
dnl     # required library checks
dnl     AM_WXPRESETS_FULL([2.6.1], [net,xrc,stc])
dnl     ...
dnl
dnl     # write the output files
dnl     AC_BAKEFILE([m4_incl ude(autoconf_inc.m4)])
dnl     AC_CONFIG_FILES([Makefile ...])
dnl     AC_OUTPUT
dnl
dnl     # end of the configure.in script
dnl     AM_WXPRESETS_MSG        # optional: just to show a message to the user
dnl
dnl ---------------------------------------------------------------------------


dnl =========================================================================================================
dnl Table of Contents of this macro file:
dnl
dnl SECTION A: generic utilities
dnl SECTION B: macros for configure options
dnl SECTION C: macros for options handling
dnl SECTION D: messages to the user
dnl =========================================================================================================






dnl =========================================================================================================
dnl                                       SECTION A: generic utilities
dnl =========================================================================================================


dnl ---------------------------------------------------------------------------
dnl AM_YESNO_OPTCHECK([name of the boolean variable to set],
dnl                   [name of the --enable-option variable with yes/no values],
dnl                   [name of the --enable option])
dnl
dnl Converts the $3 variable, suppose to contain a yes/no value to a 1/0
dnl boolean variable and saves the result into $1.
dnl Outputs also the standard checking-option message.
dnl ---------------------------------------------------------------------------
AC_DEFUN([AM_YESNO_OPTCHECK],
        [
        AC_MSG_CHECKING([for $3 option])
        if [[ "x$$2" = "xyes" ]]; then
            AC_MSG_RESULT([yes])
            $1=1
        else
            AC_MSG_RESULT([no])
            $1=0
        fi
    ])


dnl ---------------------------------------------------------------------------
dnl AM_BOOLOPT_SUMMARY([name of the boolean variable to show summary for],
dnl                   [what to print when var is 1],
dnl                   [what to print when var is 0])
dnl
dnl Prints $2 when variable $1 == 1 and prints $3 when variable $1 == 0.
dnl This macro mainly exists just to make configure.ac scripts more readable.
dnl
dnl NOTE: you need to use the [" my message"] syntax for 2nd and 3rd arguments
dnl       if you want that M4 avoid to throw away the spaces prefixed to the
dnl       argument value.
dnl ---------------------------------------------------------------------------
AC_DEFUN([AM_BOOLOPT_SUMMARY],
        [
        if [[ "x$$1" = "x1" ]]; then
            echo $2
        elif [[ "x$$1" = "x0" ]]; then
            echo $3
        else
            echo "$1 is $$1"
        fi
    ])


dnl ---------------------------------------------------------------------------
dnl AM_SAVE_COREVAR()
dnl
dnl Saves the CPPFLAGS, CXXFLAGS, CFLAGS and LIBS variables in some helper
dnl variables which are used to restore them later (see AM_RESTORE_COREVAR)
dnl ---------------------------------------------------------------------------
AC_DEFUN([AM_SAVE_COREVAR],
        [
        SAVED_CPPFLAGS="$CPPFLAGS"
        SAVED_CXXFLAGS="$CXXFLAGS"
        SAVED_CFLAGS="$CFLAGS"
        SAVED_LIBS="$LIBS"
    ])


dnl ---------------------------------------------------------------------------
dnl AM_RESTORE_COREVAR()
dnl
dnl Loads into the CPPFLAGS, CXXFLAGS, CFLAGS and LIBS variables the values
dnl of the helper variables set by AM_SAVE_COREVAR
dnl ---------------------------------------------------------------------------
AC_DEFUN([AM_RESTORE_COREVAR],
        [
        CPPFLAGS="$SAVED_CPPFLAGS"
        CXXFLAGS="$SAVED_CXXFLAGS"
        CFLAGS="$SAVED_CFLAGS"
        LIBS="$SAVED_LIBS"
    ])


dnl ---------------------------------------------------------------------------
dnl AM_PREPEND_WXLIKE_LIB()
dnl
dnl Prepends to WX_LIBS variable a library named with same wxWidgets rules.
dnl ---------------------------------------------------------------------------
AC_DEFUN([AM_PREPEND_WXLIKE_LIB],
        [
        dnl The piece '$WX_CONFIG_PATH $wx_config_args' was taken from wxwin.m4
        WX_LIBS="$($WX_CONFIG_PATH $wx_config_args $WXCFG_FLAGS $1,$WX_ADDITIONAL_LIBS --libs)"
    ])






dnl =========================================================================================================
dnl                                 SECTION B: macros for configure options
dnl =========================================================================================================

dnl ---------------------------------------------------------------------------
dnl WX_ARG_ENABLE/WX_ARG_WITH
dnl
dnl Two little custom macros which define the ENABLE/WITH configure arguments.
dnl Macro arguments:
dnl $1 = the name of the --enable / --with  feature
dnl $2 = the name of the variable associated
dnl $3 = the description of that feature
dnl $4 = the default value for that feature
dnl $5 = additional action to do in case option is given with "yes" value
dnl ---------------------------------------------------------------------------
AC_DEFUN([WX_ARG_ENABLE],
         [AC_ARG_ENABLE($1,
            AC_HELP_STRING([--enable-$1], [$3 (default is $4)]),
            [], [enableval="$4"])

            dnl Show a message to the user about this option
            AC_MSG_CHECKING([for the --enable-$1 option])
            if test "$enableval" = "yes" ; then
                AC_MSG_RESULT([yes])
                $2=1
                $5
            elif test "$enableval" = "no" ; then
                AC_MSG_RESULT([no])
                $2=0
            elif test "$enableval" = "auto" ; then
                AC_MSG_RESULT([will be automatically detected])
                $2="auto"
            else
                AC_MSG_ERROR([
    Unrecognized option value (allowed values: yes, no, auto) !
                ])
            fi
         ])

AC_DEFUN([WX_ARG_WITH],
         [AC_ARG_WITH($1,
            AC_HELP_STRING([--with-$1], [$3 (default is $4)]),
            [], [withval="$4"])

            dnl Show a message to the user about this option
            AC_MSG_CHECKING([for the --with-$1 option])
            if test "$withval" = "yes" ; then
                AC_MSG_RESULT([yes])
                $2=1
                $5
            dnl NB: by default we don't allow --with-$1=no option
            dnl     since it does not make much sense !
            elif test "$6" = "1" -a "$withval" = "no" ; then
                AC_MSG_RESULT([no])
                $2=0
            elif test "$withval" = "auto" ; then
                AC_MSG_RESULT([will be automatically detected])
                $2="auto"
            else
                AC_MSG_ERROR([
    Unrecognized option value (allowed values: yes, auto) !
                ])
            fi
         ])

dnl ---------------------------------------------------------------------------
dnl AM_OPTIONS_WXPRESETS
dnl
dnl Gives to the configure script the following options:
dnl   --enable-debug
dnl   --enable-unicode
dnl   --enable-shared
dnl   --enable-wxshared
dnl Then checks for their presence and eventually set the DEBUG,UNICODE.SHARED
dnl WXSHARED variables accordingly.
dnl Note that DEBUG != WX_DEBUG; the first is the value of the --enable-debug
dnl option (in boolean format) while the second indicates if wxWidgets was
dnl built in debug mode (and still is in boolean format).
dnl ---------------------------------------------------------------------------
AC_DEFUN([AM_OPTIONS_WXPRESETS],
        [

        dnl The default values (and also option names) are the same of wx's ones
        WX_ARG_ENABLE([debug], [DEBUG], [Builds in debug mode], [auto])
        WX_ARG_ENABLE([unicode], [UNICODE], [Builds in unicode mode], [auto])
        WX_ARG_ENABLE([shared], [SHARED], [Builds in shared mode], [auto])

        PORT="auto"
        WX_ARG_WITH([motif], [MOTIFPORT], [Uses the wxMotif port], [auto], [PORT="motif"])
        WX_ARG_WITH([gtk], [GTKPORT], [Uses the wxGTK port], [auto], [PORT="gtk"])
        WX_ARG_WITH([x11], [X11PORT], [Uses the wxX11 port], [auto], [PORT="x11"])
        WX_ARG_WITH([mac], [MACPORT], [Uses the wxMac port], [auto], [PORT="mac"])
        WX_ARG_WITH([cocoa], [MACPORT], [Uses the wxCocoa port], [auto], [PORT="cocoa"])
        WX_ARG_WITH([mgl], [MGLPORT], [Uses the wxMGL port], [auto], [PORT="mgl"])

        dnl In case we are on Cygwin !
        WX_ARG_WITH([msw], [MSWPORT], [Uses the wxMSW port], [auto], [PORT="msw"])

        dnl ****** IMPORTANT *******
        dnl   Unlike DEBUG and UNICODE settings, you can build your program in
        dnl   shared mode against a static build of wxWidgets. Thus we have the
        dnl   following option which allows these mixtures. E.g.
        dnl
        dnl      ./configure --disable-shared --with-wxshared
        dnl
        dnl   will build your library in static mode against the first available
        dnl   shared build of wxWidgets.
        dnl
        dnl   Note that's not possible to do the viceversa:
        dnl
        dnl      ./configure --enable-shared --without-wxshared
        dnl
        dnl   would try to build your library in shared mode against a static
        dnl   build of wxWidgets. This is not possible !
        dnl   A check for this combination of options is in AM_WXPRESETS_CHECK
        dnl   (where we know what 'auto' should be expanded to).
        dnl
        dnl   If you try to build something in ANSI mode against a UNICODE build
        dnl   of wxWidgets or in RELEASE mode against a DEBUG build of wxWidgets,
        dnl   then at best you'll get ton of linking errors !
        dnl ************************
        WX_ARG_WITH([wxshared], [WX_SHARED],
                    [Builds against a shared build of wxWidgets], [auto], [], [1])

        dnl FOR DEBUG ONLY
        if test "$DBG_CONFIGURE" = "1"; then
            echo "[[dbg]] SHARED: $SHARED, WX_SHARED: $WX_SHARED"
            echo "[[dbg]] DEBUG: $DEBUG, UNICODE: $UNICODE, PORT: $PORT"
            echo "[[dbg]] GTKPORT: $GTKPORT, X11PORT: $X11PORT, MACPORT: $MACPORT"
            echo "[[dbg]] MOTIFPORT: $MOTIFPORT, MGLPORT: $MGLPORT"
        fi
    ])







dnl =========================================================================================================
dnl                                 SECTION C: macros for configure options
dnl =========================================================================================================

dnl ---------------------------------------------------------------------------
dnl AM_WXFLAGS_CONTAIN / AM_WXSELECTEDCONFIG_CONTAIN ([RESULTVAR], [STRING])
dnl
dnl Sets to nonzero the variable named "WX_$RESULTVAR" if the wxWidgets
dnl additional CPP flags (taken from $WX_CPPFLAGS) contain the given STRING.
dnl Otherwise the variable will be set to 0.
dnl
dnl NOTE: 'expr match STRING REGEXP' cannot be used since on Mac it doesn't work;
dnl       we'll use 'expr STRING : REGEXP' instead
dnl ---------------------------------------------------------------------------
AC_DEFUN([AM_WXFLAGS_CONTAIN],
        [
        WX_$1=$(expr "$WX_CPPFLAGS" : ".*$2.*")
    ])

AC_DEFUN([AM_WXSELECTEDCONFIG_CONTAIN],
        [
        WX_$1=$(expr "$WX_SELECTEDCONFIG" : ".*$2.*")
    ])


dnl ---------------------------------------------------------------------------
dnl AM_WXFLAGS_CHECK([RESULTVAR], [STRING], [MSG] [, ACTION-IF-FOUND
dnl                                               [, ACTION-IF-NOT-FOUND]])
dnl
dnl Outputs the given MSG. Then searches the given STRING in the wxWidgets
dnl additional CPP flags and put the result of the search in WX_$RESULTVAR
dnl also adding the "yes" or "no" message result to MSG.
dnl ---------------------------------------------------------------------------
AC_DEFUN([AM_WXFLAGS_CHECK],
        [
        if test "$$1" = "auto" ; then

            dnl The user does not have particular preferences for this option;
            dnl so we will detect the wxWidgets relative build setting and use it
            AC_MSG_CHECKING([$3])
            AM_WXFLAGS_CONTAIN([$1], [$2])
            if test "$WX_$1" != "0"; then
                WX_$1=1
                AC_MSG_RESULT([yes])
                ifelse([$4], , :, [$4])
            else
                WX_$1=0
                AC_MSG_RESULT([no])
                ifelse([$5], , :, [$5])
            fi
        else

            dnl Use the setting given by the user
            WX_$1=$$1
        fi
    ])

dnl ---------------------------------------------------------------------------
dnl AM_WXSELECTEDCONFIG_CHECK([RESULTVAR], [STRING], [MSG] [, ACTION-IF-FOUND
dnl                                               [, ACTION-IF-NOT-FOUND]])
dnl
dnl Works like AM_WXFLAGS_CHECK but uses the wxWidgets-2.6 specific option
dnl of wx-config: --selected_config instead of grepping WX_CPPFLAGS var.
dnl ---------------------------------------------------------------------------
AC_DEFUN([AM_WXSELECTEDCONFIG_CHECK],
        [
        if test "$$1" = "auto" ; then

            dnl The user does not have particular preferences for this option;
            dnl so we will detect the wxWidgets relative build setting and use it
            AC_MSG_CHECKING([$3])
            AM_WXSELECTEDCONFIG_CONTAIN([$1], [$2])
            if test "$WX_$1" != "0"; then
                WX_$1=1
                AC_MSG_RESULT([yes])
                ifelse([$4], , :, [$4])
            else
                WX_$1=0
                AC_MSG_RESULT([no])
                ifelse([$5], , :, [$5])
            fi
        else

            dnl Use the setting given by the user
            WX_$1=$$1
        fi
    ])


dnl ---------------------------------------------------------------------------
dnl AM_WXPRESETS_CHECK
dnl
dnl Detects the values of the following variables:
dnl 1) WX_UNICODE
dnl 2) WX_DEBUG
dnl 3) WX_SHARED    (and also WX_STATIC)
dnl 4) WX_VERSION
dnl 5) WX_VERSION_MAJOR
dnl 6) WX_VERSION_MINOR
dnl 7) WX_*PORT (i.e. WX_MOTIFPORT, WX_MACPORT, WX_X11PORT, WX_MGLPORT, etc)
dnl 8) WX_PORT
dnl from the previously selected wxWidgets build; this macro in fact must be
dnl called *after* calling the AM_PATH_WXCONFIG macro.
dnl ---------------------------------------------------------------------------
AC_DEFUN([AM_WXPRESETS_CHECK],
        [
        dnl be sure that the WX_VERSION variable has the same format of the
        dnl WX_VERSION option which is used by wx_win32.bkl
        dnl (i.e. 25, 26, 27... instead of 2.5.x, 2.6.x, 2.7.x...)
        WX_VERSION_MAJOR=`echo $WX_VERSION | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
        WX_VERSION_MINOR=`echo $WX_VERSION | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
        WX_VERSION=$wx_config_major_version$wx_config_minor_version

        dnl these are required !
        AC_SUBST(WX_VERSION_MAJOR)
        AC_SUBST(WX_VERSION_MINOR)


        dnl FOR DEBUG ONLY
        if test "$DBG_CONFIGURE" = "1"; then
            echo "[[dbg]] WX_VERSION: $WX_VERSION"
        fi

        dnl we could test directly for WX_SHARED with a line like:
        dnl    AM_WXSELECTEDCONFIG_CHECK([SHARED], [shared],
        dnl                         [if wxWidgets was built in SHARED mode])
        dnl but wx-config --selected-config DOES NOT outputs the 'shared'
        dnl word when wx was built in shared mode; it just outputs the
        dnl 'static' word when built in static mode.
        if test $WX_SHARED = "1"; then
            STATIC=0
        elif test $WX_SHARED = "0"; then
            STATIC=1
        elif test $WX_SHARED = "auto"; then
            STATIC="auto"
        fi

        if test $WX_VERSION -gt 26 ; then

            dnl The wx-config we are using understands the "--selected_config"
            dnl option which returns an easy-parseable string !
            WX_SELECTEDCONFIG=$($WX_CONFIG_WITH_ARGS --selected_config)

            dnl FOR DEBUG ONLY
            if test "$DBG_CONFIGURE" = "1"; then
                echo "[[dbg]] Using wx-config --selected_config"
                echo "[[dbg]] WX_SELECTEDCONFIG: $WX_SELECTEDCONFIG"
            fi

            AM_WXSELECTEDCONFIG_CHECK([UNICODE], [unicode],
                                [if wxWidgets was built with UNICODE enabled])
            AM_WXSELECTEDCONFIG_CHECK([DEBUG], [debug],
                                [if wxWidgets was built in DEBUG mode])
            AM_WXSELECTEDCONFIG_CHECK([STATIC], [static],
                                [if wxWidgets was built in STATIC mode])

            dnl checks for the wx port used
            WX_PORT="unknown"
            AM_WXSELECTEDCONFIG_CHECK([GTKPORT], [gtk],
                                [if wxWidgets port is wxGTK], [WX_PORT="gtk"])
            AM_WXSELECTEDCONFIG_CHECK([MOTIFPORT], [motif],
                                [if wxWidgets port is wxMotif], [WX_PORT="motif"])
            AM_WXSELECTEDCONFIG_CHECK([MACPORT], [mac],
                                [if wxWidgets port is wxMac], [WX_PORT="mac"])
            AM_WXSELECTEDCONFIG_CHECK([MACPORT], [cocoa],
                                [if wxWidgets port is wxMac], [WX_PORT="cocoa"])
            AM_WXSELECTEDCONFIG_CHECK([X11PORT], [x11],
                                [if wxWidgets port is wxX11], [WX_PORT="x11"])
            AM_WXSELECTEDCONFIG_CHECK([MGLPORT], [mgl],
                                [if wxWidgets port is wxMGL], [WX_PORT="mgl"])
            AM_WXSELECTEDCONFIG_CHECK([MSWPORT], [msw],
                                [if wxWidgets port is wxMSW], [WX_PORT="msw"])

            dnl in case we are using wxGTK check if we are using GTK 1.x or GTK 2.x;
            dnl the user will be able to find the result of the check in the
            dnl $WX_GTKPORT_VERSION variable
            if test "$WX_PORT" = "gtk"; then
                TEMP=auto
                AM_WXSELECTEDCONFIG_CHECK([TEMP], [gtk2],
                                    [if wxGTK uses GTK 2.x (instead of GTK 1.x)],
                                    [WX_GTKPORT_VERSION=2], [WX_GTKPORT_VERSION=1])
            fi
        else

            dnl FOR DEBUG ONLY
            if test "$DBG_CONFIGURE" = "1"; then
                echo "Using WX_CPPFLAGS: $WX_CPPFLAGS"
            fi

            dnl Before wx2.6.2, wx-config did not have the "--selected_config"
            dnl option... thus we need to extract the info we need from $WX_CPPFLAGS

            AM_WXFLAGS_CHECK([UNICODE], [unicode],
                                [if wxWidgets was built with UNICODE enabled])
            AM_WXFLAGS_CHECK([DEBUG], [__WXDEBUG__],
                                [if wxWidgets was built in DEBUG mode])
            AM_WXFLAGS_CHECK([STATIC], [static],
                                [if wxWidgets was built in STATIC mode])

            dnl checks for the wx port used
            WX_PORT="unknown"
            AM_WXFLAGS_CHECK([GTKPORT], [__WXGTK__],
                                [if wxWidgets port is wxGTK], [WX_PORT="gtk"])
            AM_WXFLAGS_CHECK([MOTIFPORT], [__WXMOTIF__],
                                [if wxWidgets port is wxMotif], [WX_PORT="motif"])
            AM_WXFLAGS_CHECK([MACPORT], [__WXMAC__],
                                [if wxWidgets port is wxMac], [WX_PORT="mac"])
            AM_WXFLAGS_CHECK([MACPORT], [__WXCOCOA__],
                                [if wxWidgets port is wxMac], [WX_PORT="cocoa"])
            AM_WXFLAGS_CHECK([X11PORT], [__WXX11__],
                                [if wxWidgets port is wxX11], [WX_PORT="x11"])
            AM_WXFLAGS_CHECK([MGLPORT], [__WXMGL__],
                                [if wxWidgets port is wxMGL], [WX_PORT="mgl"])
            AM_WXFLAGS_CHECK([MSWPORT], [__WXMSW__],
                                [if wxWidgets port is wxMSW], [WX_PORT="msw"])

            dnl in case we are using wxGTK check if we are using GTK 1.x or GTK 2.x;
            dnl the user will be able to find the result of the check in the
            dnl $WX_GTKPORT_VERSION variable
            if test "$WX_PORT" = "gtk"; then
                TEMP=auto
                AM_WXFLAGS_CHECK([TEMP], [gtk2],
                                    [if wxGTK uses GTK 2.x (instead of GTK 1.x)],
                                    [WX_GTKPORT_VERSION=2], [WX_GTKPORT_VERSION=1])
            fi
        fi

        dnl init WX_SHARED from WX_STATIC
        if test "$WX_STATIC" != "0"; then
            WX_SHARED=0
        else
            WX_SHARED=1
        fi

        dnl FOR DEBUG ONLY
        if test "$DBG_CONFIGURE" = "1"; then
            echo "[[dbg]] WX_SHARED: $WX_SHARED"
            echo "[[dbg]] WX_DEBUG: $WX_DEBUG"
            echo "[[dbg]] WX_UNICODE: $WX_UNICODE"
            echo "[[dbg]] WX_GTKPORT: $WX_GTKPORT, WX_MOTIFPORT: $WX_MOTIFPORT"
            echo "[[dbg]] WX_MACPORT: $WX_MACPORT, WX_X11PORT: $WX_X11PORT"
            echo "[[dbg]] WX_MGLPORT: $WX_MGLPORT, WX_MSWPORT: $WX_MSWPORT"
            echo "[[dbg]] WX_PORT: $WX_PORT"
            echo "[[dbg]] WX_GTKPORT_VERSION: $WX_GTKPORT_VERSION"
        fi


        dnl nice way to check:
        dnl - that only one of the WX_*PORT variables has been set to 1
        dnl - at least one of the WX_*PORT has been set !

        if test "$WX_PORT" = "unknown" -a "$PORT" = "auto" ; then
            AC_MSG_ERROR([
                           Cannot detect the currently installed wxWidgets port !
                           Check your 'wx-config --cxxflags'...
                         ])
        fi

        checksum="$(expr $WX_GTKPORT + $WX_MOTIFPORT + $WX_MACPORT + \
                                $WX_X11PORT + $WX_MGLPORT + $WX_MSWPORT)"
        if test "$checksum" = "0" ; then
            AC_MSG_ERROR([
                           Cannot detect the currently installed wxWidgets port !
                           Check your 'wx-config --cxxflags'...
                         ])
        fi
        if test "$checksum" != "1" ; then
            AC_MSG_ERROR([
    Your 'wx-config --cxxflags' command seems to define more than one port symbol...
    check your wxWidgets installation (the 'wx-config --list' command may help).
                         ])
        fi
        AC_SUBST(WX_PORT)
        AC_SUBST(WX_GTKPORT_VERSION)


        dnl Avoid problem described in the AM_OPTIONS_WXPRESETS which happens when
        dnl the user gives the options:
        dnl      ./configure --enable-shared --without-wxshared
        dnl or just do
        dnl      ./configure --enable-shared
        dnl but there is only a static build of wxWidgets available.
        if test "$WX_SHARED" = "0" -a "$SHARED" = "1"; then
            AC_MSG_ERROR([
    Cannot build in shared mode against a static build of wxWidgets !
    This error happens because the wxWidgets build which was selected
    has been detected as STATIC while you asked to build $PACKAGE_NAME
    in SHARED mode and this is not possible.
    Use the '--disable-shared' option to build $PACKAGE_NAME
    as STATIC or '--with-wxshared' to use a SHARED wxWidgets build.
                         ])
        fi
    ])


dnl ---------------------------------------------------------------------------
dnl AM_CONVERT_WXPRESETS_OPTIONS_TO_WXCONFIG_FLAGS
dnl
dnl Sets the WXCFG_FLAGS string using the SHARED,DEBUG,UNICODE variable values
dnl which are different from "auto"
dnl ---------------------------------------------------------------------------
AC_DEFUN([AM_CONVERT_WXPRESETS_OPTIONS_TO_WXCONFIG_FLAGS],
        [

        dnl The space at the end of the WXCFG_FLAGS is important; do not remove !
        if test "$WX_SHARED" = "1" ; then
            WXCFG_FLAGS="--static=no "
        elif test "$WX_SHARED" = "0" ; then
            WXCFG_FLAGS="--static=yes "
        fi

        if test "$DEBUG" = "1" ; then
            WXCFG_FLAGS="$WXCFG_FLAGS""--debug=yes "
        elif test "$DEBUG" = "0" ; then
            WXCFG_FLAGS="$WXCFG_FLAGS""--debug=no "
        fi

        if test "$UNICODE" = "1" ; then
            WXCFG_FLAGS="$WXCFG_FLAGS""--unicode=yes "
        elif test "$UNICODE" = "0" ; then
            WXCFG_FLAGS="$WXCFG_FLAGS""--unicode=no "
        fi

        if test "$GTKPORT" = "1" ; then
            WXCFG_FLAGS="$WXCFG_FLAGS""--toolkit=gtk2 "
        elif test "$MOTIFPORT" = "1" ; then
            WXCFG_FLAGS="$WXCFG_FLAGS""--toolkit=motif "
        elif test "$X11PORT" = "1" ; then
            WXCFG_FLAGS="$WXCFG_FLAGS""--toolkit=x11 "
        elif test "$MACPORT" = "1" ; then
            WXCFG_FLAGS="$WXCFG_FLAGS""--toolkit=mac "
        elif test "$COCOAPORT" = "1" ; then
            WXCFG_FLAGS="$WXCFG_FLAGS""--toolkit=cocoa "
        elif test "$MGLPORT" = "1" ; then
            WXCFG_FLAGS="$WXCFG_FLAGS""--toolkit=mgl "
        elif test "$MSWPORT" = "1" ; then
            WXCFG_FLAGS="$WXCFG_FLAGS""--toolkit=msw "
        fi


        dnl FOR DEBUG ONLY
        if test "$DBG_CONFIGURE" = "1"; then
            echo "[[dbg]] WXCFG_FLAGS: $WXCFG_FLAGS"
        fi
    ])


dnl ---------------------------------------------------------------------------
dnl AM_GET_GTK_FLAGS
dnl
dnl When using the wxGTK port, this macro creates a GTKPKG_FLAGS variable that
dnl contains the c flags (-I and -D flags) required to compile a program which
dnl uses directly the GTK libraries (instead using the wx layer).
dnl ---------------------------------------------------------------------------
AC_DEFUN([AM_GET_GTK_FLAGS],
        [
            if test "$WX_PORT" = "gtk" ; then
                dnl The commands below were taken from usenet postings by
                dnl John Labenski
                dnl NB Trying to do GTKPKG_FLAGS=`gtk-config --cflags` fails,
                dnl    you have to use an intermediate var
                if test "$WX_GTKPORT_VERSION" = "1" ; then
                    _GTKPKG_FL="gtk-config --cflags"
                    GTKPKG_FLAGS=`$_GTKPKG_FL`
                else
                    _GTKPKG_FL="pkg-config --cflags gtk+-2.0"
                    GTKPKG_FLAGS=`$_GTKPKG_FL`
                fi
            fi
        ])



dnl ---------------------------------------------------------------------------
dnl AM_WXPRESETS([minimum-wx-version], [addwxlibraries], [addgtkflags])
dnl
dnl This is the CORE macro of this file: it uses the options previously
dnl defined by AM_OPTIONS_WXPRESETS (--enable-debug, --enable-unicode, etc)
dnl to create a string to give to wx-config in order to select a compatible
dnl wxWidgets build (through AM_CONVERT_WXPRESETS_OPTIONS_TO_WXCONFIG_FLAGS).
dnl
dnl Then it calls AM_WXPRESETS_CHECK to extract informations returned by
dnl wx-config and convert them into the WX_* variables.
dnl
dnl Thus this acts just as a convenience macro for
dnl    AM_PATH_WXCONFIG([minimum-wx-version],,, [addwxlibraries])
dnl    AM_WXPRESETS_CHECK
dnl    AM_GET_GTK_FLAGS                  (only when addgtkflags != '')
dnl macros.
dnl
dnl NOTE: "addwxlibraries" are those wxWidgets libraries (when wx is built
dnl       in multilib mode) required by the program to link (e.g. xml,xrc,net,
dnl       odbc,qa,etc); the core & base libraries are included by default.
dnl
dnl NOTE2: see AM_GET_GTK_FLAGS for "addgtkflags" parameter.
dnl ---------------------------------------------------------------------------
AC_DEFUN([AM_WXPRESETS],
         [
            dnl From --enable-* options set by AM_OPTIONS_WXPRESETS, create the
            dnl WXCFG_FLAGS string to give to wx-config
            AM_CONVERT_WXPRESETS_OPTIONS_TO_WXCONFIG_FLAGS

            dnl save the additional wx libraries required as they could be useful
            dnl later from AM_PREPEND_WXLIKE_LIB macro
            WX_ADDITIONAL_LIBS="$2,core,base"

            AM_PATH_WXCONFIG([$1], [wxWin=1], [wxWin=0],
                             [$WX_ADDITIONAL_LIBS], [$WXCFG_FLAGS])
            if test "$wxWin" != 1; then

                if test ! -z "$WXCFG_FLAGS"; then

                    MSG="
    The configuration you asked for $PACKAGE_NAME requires a wxWidgets
    build with the following settings:
        $WXCFG_FLAGS
    but such build is not available.

    Try to use the default build of wxWidgets
    (see the command 'wx-config --list') leaving
    the --enable-unicode, --enable-static, --enable-debug
    and --with-* options to their default values."
                fi

                AC_MSG_ERROR([
    The requested wxWidgets build couldn't be found.
    $MSG

    If you still get this error, then check that 'wx-config' is
    in path, the directory where wxWidgets libraries are installed
    (returned by 'wx-config --libs' command) is in LD_LIBRARY_PATH
    or equivalent variable and wxWidgets version is $1 or above.
                ])
            else
                dnl do not update CPPFLAGS nor CXXFLAGS doing something like:
                dnl       CPPFLAGS="$CPPFLAGS $WX_CPPFLAGS"
                dnl       CXXFLAGS="$CXXFLAGS $WX_CXXFLAGS"
                dnl otherwise we'll get the
                dnl same settings repetead three times since wxpresets generate
                dnl a Makefile.in which merges these two variables with
                dnl $WX_CPPFLAGS automatically...
                dnl CFLAGS="$CFLAGS $WX_CFLAGS_ONLY"
                dnl LIBS="$LIBS $WX_LIBS"
                true;       dnl no-operation used because empty branchs are not
                            dnl allowed: else fi; gives an error !
            fi

            dnl save the full wxWidgets version in WX_VERSION_FULL
            dnl (to be compatible with win32 bakefiles, AM_WXPRESETS_CHECK
            dnl overwrites WX_VERSION with only the major and minor digits
            dnl of wx-config --version without any dot).
            WX_VERSION_FULL="$WX_VERSION"
            AC_SUBST(WX_VERSION_FULL)

            dnl Synch our WX_DEBUG,WX_UNICODE,WX_SHARED,WX_VERSION,etc
            dnl variables with wx ones
            AM_WXPRESETS_CHECK

            dnl now we can finally update the DEBUG,UNICODE,SHARED options
            dnl to their final values if they were set to 'auto'
            if test "$DEBUG" = "auto"; then
                DEBUG=$WX_DEBUG

                dnl in case user wants a BUILD=debug/release var...
                if test "$DEBUG" = "1"; then
                    BUILD="debug"
                elif test "$DEBUG" = ""; then
                    BUILD="release"
                fi
            fi
            if test "$UNICODE" = "auto"; then
                UNICODE=$WX_UNICODE
            fi
            if test "$SHARED" = "auto"; then
                SHARED=$WX_SHARED
            fi

            dnl It doesn't matter what "gtkaddflags" parameter contains,
            dnl if it's non-empty it's true
            if test -n "$3" ; then
                dnl Add gtk-specific flags (useful when compiling wx sources
                dnl that directly use the GTK libraries)
                AM_GET_GTK_FLAGS

                dnl Update global flags
                CXXFLAGS="$CXXFLAGS $GTKPKG_FLAGS"
                CFLAGS="$CFLAGS $GTKPKG_FLAGS"
            fi
        ])



dnl ---------------------------------------------------------------------------
dnl AM_WXPRESETS_FULL([minimum-wx-version], [addwxlibraries], [addgtkflags])
dnl
dnl Like AM_WXPRESETS but this macro also does those standard checks
dnl to recognize the system and the installed utility programs
dnl ---------------------------------------------------------------------------
AC_DEFUN([AM_WXPRESETS_FULL],
        [
            dnl This allows us to use Bakefile, recognizing the system type
            dnl (and sets the AC_CANONICAL_BUILD, AC_CANONICAL_HOST and
            dnl AC_CANONICAL_TARGET variables)
            AC_CANONICAL_SYSTEM

            dnl we set these vars to avoid that the AC_PROG_C* macros add the
            dnl "-g -O2" flags; we will add them later, if needed
            if test -z "$CFLAGS"; then
                CFLAGS=
            fi
            if test -z "$CXXFLAGS"; then
                CXXFLAGS=
            fi
            if test -z "$CPPFLAGS"; then
                CPPFLAGS=
            fi

            dnl Checks for basic programs used to compile/install.
            AC_PROG_AWK
            AC_PROG_INSTALL
            AC_PROG_LN_S
            AC_PROG_RANLIB
            AC_PROG_CC
            AC_PROG_CXX
            AC_PROG_CXXCPP

            dnl check for wxWidgets library and initialization of WX_* variables
            AM_WXPRESETS([$1], [$2], [$3])

            dnl add the optimize/debug flags
            dnl NOTE1: these checks must be put after AM_WXPRESETS
            dnl        (which sets the $WX_* variables)...
            dnl NOTE2: the CXXFLAGS are merged together with the CPPFLAGS so we
            dnl        don't need to set them, too
            if [[ "$WX_DEBUG" = "1" ]]; then

                dnl NOTE: the -Wundef and -Wno-ctor-dtor-privacy are not enabled
                dnl       automatically by -Wall
                dnl NOTE2: the '-Wno-ctor-dtor-privacy' has sense only when compiling
                dnl        C++ source files and thus we must be careful to add it only
                dnl        to CXXFLAGS and not to CFLAGS. Remember that CPPFLAGS is
                dnl        reserved for both C and C++ compilers while CFLAGS is
                dnl        intended as flags for C compiler only and CXXFLAGS for
                dnl        C++ only !
                CXXFLAGS="$CXXFLAGS -g3 -O0 -Wall -Wundef -Wno-ctor-dtor-privacy"
                CFLAGS="$CFLAGS -g3 -O0 -Wall -Wundef"
            else
                CXXFLAGS="$CXXFLAGS -g0 -O2"
                CFLAGS="$CFLAGS -g0 -O2"
            fi
        ])






dnl =========================================================================================================
dnl                                 SECTION D: messages to the user
dnl =========================================================================================================

dnl ---------------------------------------------------------------------------
dnl AM_WXPRESETS_MSG
dnl
dnl Shows a summary message to the user about the WX_* variable contents.
dnl This macro is used typically at the end of the configure script.
dnl ---------------------------------------------------------------------------
AC_DEFUN([AM_WXPRESETS_MSG],
    [
        AM_BOOLOPT_SUMMARY([WX_DEBUG], ["  - DEBUG build"], ["  - RELEASE build"])
        AM_BOOLOPT_SUMMARY([WX_UNICODE], ["  - UNICODE mode"], ["  - ANSI mode"])
        AM_BOOLOPT_SUMMARY([WX_SHARED], ["  - SHARED mode"], ["  - STATIC mode"])
        echo "  - VERSION: $WX_VERSION_FULL"

        if test "$WX_PORT" = "gtk"; then
            if test "$WX_GTKPORT_VERSION" = "1"; then
                echo "  - PORT: gtk (with GTK+ 1.x)"
            elif test "$WX_GTKPORT_VERSION" = "2" ; then
                echo "  - PORT: gtk (with GTK+ 2.x)"
            else
                AC_MSG_ERROR([
   Error in this configure script ! Please contact $PACKAGE_BUGREPORT
                ])
            fi
        else
            echo "  - PORT: $WX_PORT"
        fi
    ])


dnl ---------------------------------------------------------------------------
dnl AM_WXPRESETS_MSG_BEGIN
dnl
dnl Like AM_WXPRESETS_MSG but this macro also gives info about the configuration
dnl of the package which used the wxpresets.
dnl ---------------------------------------------------------------------------
AC_DEFUN([AM_WXPRESETS_MSG_BEGIN],
    [
        echo
        echo " ----------------------------------------------------------------"
        echo "  Configuration for $PACKAGE_NAME $PACKAGE_VERSION successfully completed."
        echo "  Summary of main configuration settings for $PACKAGE_NAME:"
        AM_BOOLOPT_SUMMARY([DEBUG], ["  - DEBUG build"], ["  - RELEASE build"])
        AM_BOOLOPT_SUMMARY([UNICODE], ["  - UNICODE mode"], ["  - ANSI mode"])
        AM_BOOLOPT_SUMMARY([SHARED], ["  - SHARED mode"], ["  - STATIC mode"])
    ])


dnl ---------------------------------------------------------------------------
dnl AM_WXPRESETS_MSG_END
dnl
dnl Like AM_WXPRESETS_MSG but this macro also gives info about the configuration
dnl of the package which used the wxpresets.
dnl ---------------------------------------------------------------------------
AC_DEFUN([AM_WXPRESETS_MSG_END],
    [
        echo
        echo "  The wxWidgets build which will be used by $PACKAGE_NAME $PACKAGE_VERSION"
        echo "  has the following settings:"
        AM_WXPRESETS_MSG
        echo
        echo "  Now, just run make."
        echo " ----------------------------------------------------------------"
        echo
    ])


dnl ---------------------------------------------------------------------------
dnl AM_SET_WXBUILD_STR
dnl
dnl Sets the WXBUILD_STR string to a string like 'gtk2-unicode-release-2.7'
dnl which can be used to inform the user about the selected wxWidgets build
dnl in a brief form. This macro must be called after AM_WXPRESETS.
dnl ---------------------------------------------------------------------------
AC_DEFUN([AM_SET_WXBUILD_STR],
    [
        if test $WX_VERSION -gt 26 ; then
            WXBUILD_STR=$($WX_CONFIG_WITH_ARGS --selected_config)
        else
            dnl build the string ourselves...
            WXBUILD_STR="$WX_PORT"
            if test ! -z $WX_GTKPORT_VERSION; then
                WXBUILD_STR="$WXBUILD_STR""$WX_GTKPORT_VERSION"
            fi
            if test "$WX_UNICODE" = "1"; then
                WXBUILD_STR="$WXBUILD_STR""-unicode"
            else
                WXBUILD_STR="$WXBUILD_STR""-ansi"
            fi
            if test "$WX_DEBUG" = "1"; then
                WXBUILD_STR="$WXBUILD_STR""-debug"
            else
                WXBUILD_STR="$WXBUILD_STR""-release"
            fi
            if test "$WX_SHARED" = "1"; then
                WXBUILD_STR="$WXBUILD_STR""-shared"
            else
                WXBUILD_STR="$WXBUILD_STR""-static"
            fi
            WXBUILD_STR="$WXBUILD_STR""-$WX_VERSION_MAJOR.$WX_VERSION_MINOR"
        fi
    ])


