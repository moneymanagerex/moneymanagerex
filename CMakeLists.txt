CMAKE_MINIMUM_REQUIRED(VERSION 3.1 FATAL_ERROR)
SET(InMMEX TRUE)

FIND_PACKAGE(Git REQUIRED)
# Get the latest version tag
EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} describe --match v* --tags --abbrev=0
    WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY}
    OUTPUT_VARIABLE GIT_TAG_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    TIMEOUT 4 ERROR_QUIET)
IF(NOT GIT_TAG_VERSION MATCHES "^v([0-9]+\\.[0-9]+\\.[0-9]+)(-([^ ]+))*")
    MESSAGE(FATAL_ERROR "Unable to determine MMEX version tag from sources!")
ENDIF()
# ---------------------------------------------------------------------------
# MMEX uses semantic versioning Ref: http://semver.org
#
# PROJECT VERSION - Format: MAJOR.MINOR.PATCH-UNSTABLE
# Update the version definition for the PROJECT as follows:
# 1. MAJOR version when incompatible API changes are made.
# 2. MINOR version adding functionality in a backwards-compatible manner.
# 3. PATCH version makeing backwards-compatible bug fixes.
# 4. UNSTABLE = alpha, alpha.1, beta, beta.4, rc, rc.3
#    - used as additional version suffix, must be unset for stable version
# ---------------------------------------------------------------------------
# By setting the application version here, CMake will transfer the variable
# values to the corresponding variable names in the following files:
# ./src/versions.h
# ./resources/mmex.rc
# ---------------------------------------------------------------------------
SET(MMEX_VERSION ${CMAKE_MATCH_1})
SET(MMEX_VERSION_UNSTABLE ${CMAKE_MATCH_3})
SET(MMEX_VERSION_FULL ${CMAKE_MATCH_1}${CMAKE_MATCH_2})

PROJECT(MMEX VERSION ${MMEX_VERSION})
OPTION(MMEX_PORTABLE_INSTALL "Include an empty mmexini.db3 file in the Windows installation" ON)
OPTION(MMEX_ENCRYPTION_OPTIONAL "Build even if encryption is not supported by wxsqlite library" OFF)
OPTION(MMEX_WEBSERVER "Build MMEX with webserver function" ON)

# Name of the resulted executable binary
SET(MMEX_EXE mmex)

# Find Git commit details to put into versions.h later
# Get the current working branch
EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} symbolic-ref --short -q HEAD
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    TIMEOUT 4 ERROR_QUIET)
# Get the latest abbreviated commit hash of the working branch
EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} log -1 --format=%h
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    TIMEOUT 4 ERROR_QUIET)
# Get the latest commit date of the working branch
EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} log -1 --format=%cd --date=short
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_DATE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    TIMEOUT 4 ERROR_QUIET)

IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # define shortcut
    SET(LINUX TRUE)

    # Detect Linux distribution (if possible)
    FIND_PROGRAM(LSB_RELEASE lsb_release
        DOC "tool to read linux distribution name")
    IF(LSB_RELEASE)
        EXECUTE_PROCESS(COMMAND lsb_release -is
            OUTPUT_VARIABLE LINUX_DISTRO
            OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
        EXECUTE_PROCESS(COMMAND lsb_release -cs
            OUTPUT_VARIABLE LINUX_DISTRO_CODENAME
            OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
        EXECUTE_PROCESS(COMMAND lsb_release -rs
            OUTPUT_VARIABLE LINUX_DISTRO_REL
            OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    ELSE()
        IF(EXISTS /etc/slackware-version)
            FILE(STRINGS /etc/slackware-version LINUX_DISTRO LIMIT_COUNT 1)
            IF(LINUX_DISTRO MATCHES "^([^.]+) +([0-9.]+)")
                SET(LINUX_DISTRO "${CMAKE_MATCH_1}")
                SET(LINUX_DISTRO_REL ${CMAKE_MATCH_2})
            ENDIF()
        ELSE()
            MESSAGE(SEND_ERROR "lsb_release tool not found.")
        ENDIF()
    ENDIF()
    IF(NOT LINUX_DISTRO OR NOT LINUX_DISTRO_REL)
        MESSAGE(SEND_ERROR "Could not find a GNU/Linux distribution info")
    ENDIF()
    SET(LINUX_DISTRO_STRING "${LINUX_DISTRO} ${LINUX_DISTRO_REL}")
    IF(LINUX_DISTRO_CODENAME STREQUAL "n/a")
        IF(LINUX_DISTRO STREQUAL "Debian")
            EXECUTE_PROCESS(COMMAND lsb_release -ds
                OUTPUT_VARIABLE LINUX_DISTRO_CODENAME
                OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
            IF(LINUX_DISTRO_CODENAME MATCHES " ([a-z]+)/sid$")
                SET(LINUX_DISTRO_CODENAME ${CMAKE_MATCH_1})
            ELSE()
                UNSET(LINUX_DISTRO_CODENAME)
            ENDIF()
        ELSE()
            UNSET(LINUX_DISTRO_CODENAME)
        ENDIF()
    ENDIF()
    IF(LINUX_DISTRO_CODENAME)
        SET(LINUX_DISTRO_STRING "${LINUX_DISTRO_STRING} '${LINUX_DISTRO_CODENAME}'")
    ENDIF()
ENDIF()

GET_FILENAME_COMPONENT(CMAKE_MAKE_NAME "${CMAKE_MAKE_PROGRAM}" NAME_WE)
IF(CMAKE_MAKE_NAME STREQUAL "MSBuild")
    EXECUTE_PROCESS(COMMAND "${CMAKE_MAKE_PROGRAM}" /nologo /version
        OUTPUT_VARIABLE CMAKE_MAKE_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    SET(CMAKE_MAKE_VERSION "MSBuild ${CMAKE_MAKE_VERSION}")
ELSEIF(CMAKE_MAKE_NAME STREQUAL "ninja")
    EXECUTE_PROCESS(COMMAND "${CMAKE_MAKE_PROGRAM}" --version
        OUTPUT_VARIABLE CMAKE_MAKE_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    SET(CMAKE_MAKE_VERSION "Ninja ${CMAKE_MAKE_VERSION}")
ELSEIF(CMAKE_MAKE_NAME STREQUAL "xcodebuild")
    EXECUTE_PROCESS(COMMAND "${CMAKE_MAKE_PROGRAM}" -version
        OUTPUT_VARIABLE CMAKE_MAKE_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    STRING(REGEX REPLACE "\n.*" "" CMAKE_MAKE_VERSION "${CMAKE_MAKE_VERSION}")
ELSE()
    EXECUTE_PROCESS(COMMAND "${CMAKE_MAKE_PROGRAM}" --version
        OUTPUT_VARIABLE CMAKE_MAKE_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    STRING(REGEX REPLACE "\n.*" "" CMAKE_MAKE_VERSION "${CMAKE_MAKE_VERSION}")
ENDIF()
IF(DEFINED ENV{VisualStudioVersion})
    SET(VS_VERSION $ENV{VisualStudioVersion})
ENDIF()


# --------- Set compiler parameters ---------
IF(MSVC)
    # using Visual Studio C++
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL AppleClang)
    # force libc++, not libstdc++ for Xcode 10
    # https://developer.apple.com/documentation/xcode_release_notes/xcode_10_release_notes#3035631
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
ENDIF()

# try to use clcache/ccache to speed up builds
IF(NOT CCACHE_PROGRAM)
    IF(MSVC)
        FIND_PROGRAM(CCACHE_PROGRAM clcache)
        SET(_PARAM --help)
    ELSE()
        FIND_PROGRAM(CCACHE_PROGRAM ccache)
        SET(_PARAM --version)
    ENDIF()
    IF(CCACHE_PROGRAM)
        EXECUTE_PROCESS(COMMAND "${CCACHE_PROGRAM}" ${_PARAM}
            OUTPUT_VARIABLE CCACHE_VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
        STRING(REGEX REPLACE "\n.*" "" CCACHE_VERSION "${CCACHE_VERSION}")
        STRING(REGEX REPLACE ".* " "" CCACHE_VERSION "${CCACHE_VERSION}")
        STRING(REGEX REPLACE "^v" "" CCACHE_VERSION "${CCACHE_VERSION}")
        IF(CCACHE_VERSION)
            MESSAGE(STATUS "Found compile cache tool: ${CCACHE_PROGRAM} (found version \"${CCACHE_VERSION}\")")
        ELSE()
            MESSAGE(STATUS "Found compile cache tool: ${CCACHE_PROGRAM}")
        ENDIF()
    ELSE()
        MESSAGE(STATUS "Could NOT find compile cache tool (ccache or clcache)")
    ENDIF()
    UNSET(_PARAM)
ENDIF()
IF(CCACHE_PROGRAM)
    IF(MSVC)
        FUNCTION(any_target_hook)
            SET(NON_COMPILE_TARGETS INTERFACE IMPORTED UNKNOWN ALIAS) 
            LIST(FIND NON_COMPILE_TARGETS "${ARGV1}" found)
            IF(${found} GREATER -1)
                RETURN()
            ENDIF()
                
            SET_TARGET_PROPERTIES(${ARGV0} PROPERTIES VS_GLOBAL_CLToolExe       clcache.exe)
            SET_TARGET_PROPERTIES(${ARGV0} PROPERTIES VS_GLOBAL_CLToolPath      "${CLCACHE_DIR}")
            SET_TARGET_PROPERTIES(${ARGV0} PROPERTIES VS_GLOBAL_TrackFileAccess false)
        ENDFUNCTION()

        FUNCTION(add_library)
            _add_library(${ARGN})
            any_target_hook(${ARGN})
        ENDFUNCTION()

        FUNCTION(add_executable)
            _add_executable(${ARGN})
            any_target_hook(${ARGN})
        ENDFUNCTION()
    ELSE()
        IF(NOT CMAKE_VERSION VERSION_LESS 3.4)
            SET(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
            SET(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
        ELSE()
            SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
            SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_PROGRAM}")
        ENDIF()
    ENDIF()
ENDIF()


# --------- cmake --grapgviz settings ---------
FILE(WRITE "${CMAKE_BINARY_DIR}/CMakeGraphVizOptions.cmake" "
    SET(GRAPHVIZ_GRAPH_NAME \"MMEX build dependency graph\")
    SET(GRAPHVIZ_GENERATE_PER_TARGET FALSE)
    SET(GRAPHVIZ_GENERATE_DEPENDERS FALSE)
")

# --------- Set various location ---------
IF(APPLE)
    # Preparing for configure icons
    SET(MACOSX_APP_ICON_NAME mmex.icns)
    # Setting path to icon
    SET(MACOSX_APP_ICON_FILE "${PROJECT_SOURCE_DIR}/resources/${MACOSX_APP_ICON_NAME}")
    SET(MMEX_DOC_DIR ${MMEX_EXE}.app/Contents/SharedSupport/doc)
    SET(MMEX_RES_DIR ${MMEX_EXE}.app/Contents/Resources)
ELSEIF(WIN32)
    SET(MMEX_DOC_DIR .)
    SET(MMEX_RES_DIR res)
ELSE()
    SET(MMEX_DOC_DIR share/doc/mmex)
    SET(MMEX_RES_DIR share/mmex/res)
ENDIF()


# --------- wxWidgets dependency ---------
IF(CMAKE_VERSION VERSION_LESS 3.14)
    # FindwxWidgets.cmake module will be fixed in CMake 3.14.0
    LIST(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/util")
ENDIF()
SET(wxWidgets_USE_REL_AND_DBG ON)
FIND_PACKAGE(wxWidgets 2.9.2 REQUIRED
    COMPONENTS core qa html xml aui adv stc webview base
    OPTIONAL_COMPONENTS scintilla)
IF(APPLE AND wxWidgets_LIBRARIES MATCHES "${CMAKE_STATIC_LIBRARY_PREFIX}wx_baseu-[0-9\\.]+${CMAKE_STATIC_LIBRARY_SUFFIX}")
    # use static libs in place of dynamic if using static wxMac
    FOREACH(deplib png jpeg)
        FIND_LIBRARY(${deplib}path ${CMAKE_STATIC_LIBRARY_PREFIX}${deplib}${CMAKE_STATIC_LIBRARY_SUFFIX})
        IF(${deplib}path)
            STRING(REPLACE "-l${deplib}" "${${deplib}path}"
                wxWidgets_LIBRARIES "${wxWidgets_LIBRARIES}")
        ENDIF()
    ENDFOREACH()
    UNSET(deplib)
    UNSET(${deplib}path)
ENDIF()
ADD_LIBRARY(wxWidgets INTERFACE)
TARGET_INCLUDE_DIRECTORIES(wxWidgets SYSTEM INTERFACE ${wxWidgets_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(wxWidgets INTERFACE ${wxWidgets_LIBRARIES})
TARGET_COMPILE_OPTIONS(wxWidgets INTERFACE ${wxWidgets_CXX_FLAGS})
TARGET_COMPILE_DEFINITIONS(wxWidgets INTERFACE ${wxWidgets_DEFINITIONS}
    wxNO_UNSAFE_WXSTRING_CONV=1
    $<$<CONFIG:Debug>:${wxWidgets_DEFINITIONS_DEBUG}>)


# --------- curl dependency ---------
# curl version >=7.57 can have config files
FIND_PACKAGE(CURL QUIET COMPONENTS libcurl CONFIG)
IF(NOT CURL_FOUND AND NOT CURL_VERSION VERSION_LESS 7.62)
    # curl 7.62 removed support for COMPONENTS
    FIND_PACKAGE(CURL QUIET CONFIG)
ENDIF()
IF(CURL_FOUND)
    MESSAGE(STATUS "Found CURL: ${CURL_LIBRARIES} (found version \"${CURL_VERSION}\")")
    GET_TARGET_PROPERTY(LIBCURL_TYPE CURL::libcurl TYPE)
    # CURL_STATICLIB is missing for curl versions <7.61.1
    IF(CURL_VERSION VERSION_LESS 7.61.1 AND LIBCURL_TYPE STREQUAL STATIC_LIBRARY)
        SET_TARGET_PROPERTIES(CURL::libcurl PROPERTIES
            INTERFACE_COMPILE_DEFINITIONS CURL_STATICLIB)
    ENDIF()
ELSE()
    SET(CMAKE_FIND_LIBRARY_PREFIXES ${CMAKE_FIND_LIBRARY_PREFIXES} lib)
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES} _imp.lib -d.lib -d_imp.lib)
    FIND_PACKAGE(PkgConfig)
    IF(PkgConfig_FOUND)
        SET(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)
        PKG_SEARCH_MODULE(CURL libcurl)
        IF(CURL_FOUND)
            # CMake <3.12 workaround
            IF(NOT CURL_LINK_LIBRARIES)
                FIND_LIBRARY(CURL_LINK_LIBRARIES NAMES ${CURL_LIBRARIES}
                    HINTS "${CURL_LIBDIR}" NO_DEFAULT_PATH)
                IF(NOT CURL_LINK_LIBRARIES)
                    MESSAGE(WARNING "CURL library file cannot be found!")
                ENDIF()
            ENDIF()
            IF(NOT CURL_LINK_LIBRARIES MATCHES "_imp.lib$|${CMAKE_SHARED_LIBRARY_SUFFIX}$")
                LIST(REMOVE_ITEM CURL_STATIC_LIBRARIES ${CURL_LIBRARIES})
                ADD_LIBRARY(CURL::libcurl STATIC IMPORTED)
                SET_TARGET_PROPERTIES(CURL::libcurl PROPERTIES
                    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${CURL_STATIC_INCLUDE_DIRS}"
                    INTERFACE_COMPILE_DEFINITIONS CURL_STATICLIB
                    INTERFACE_LINK_LIBRARIES "${CURL_STATIC_LIBRARIES}"
                    IMPORTED_LINK_INTERFACE_LANGUAGES C
                    IMPORTED_LOCATION "${CURL_LINK_LIBRARIES}")
                LINK_DIRECTORIES(CURL_STATIC_LIBRARY_DIRS)
            ELSE()
                ADD_LIBRARY(CURL::libcurl SHARED IMPORTED)
                SET_TARGET_PROPERTIES(CURL::libcurl PROPERTIES
                    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIRS}")
                IF(WIN32)
                    SET_TARGET_PROPERTIES(CURL::libcurl PROPERTIES
                        IMPORTED_IMPLIB "${CURL_LINK_LIBRARIES}")
                ELSE()
                    SET_TARGET_PROPERTIES(CURL::libcurl PROPERTIES
                        IMPORTED_LOCATION "${CURL_LINK_LIBRARIES}")
                ENDIF()
            ENDIF()
            MESSAGE(STATUS "Found CURL: ${CURL_LINK_LIBRARIES} (found version \"${CURL_VERSION}\")")
        ENDIF()
    ENDIF()
    IF(NOT CURL_FOUND)
        FIND_PACKAGE(CURL REQUIRED COMPONENTS libcurl MODULE)
        IF(NOT CURL_LIBRARIES MATCHES "_imp.lib$|${CMAKE_SHARED_LIBRARY_SUFFIX}$")
            ADD_LIBRARY(CURL::libcurl STATIC IMPORTED)
            SET_TARGET_PROPERTIES(CURL::libcurl PROPERTIES
                INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIRS}"
                INTERFACE_COMPILE_DEFINITIONS CURL_STATICLIB
                IMPORTED_LINK_INTERFACE_LANGUAGES C
                IMPORTED_LOCATION "${CURL_LIBRARIES}")
        ELSE()
            ADD_LIBRARY(CURL::libcurl SHARED IMPORTED)
            SET_TARGET_PROPERTIES(CURL::libcurl PROPERTIES
                INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIRS}")
            IF(WIN32)
                SET_TARGET_PROPERTIES(CURL::libcurl PROPERTIES
                    IMPORTED_IMPLIB "${CURL_LIBRARIES}")
            ELSE()
                SET_TARGET_PROPERTIES(CURL::libcurl PROPERTIES
                    IMPORTED_LOCATION "${CURL_LIBRARIES}")
            ENDIF()
        ENDIF()
        SET(CURL_VERSION ${CURL_VERSION_STRING})
    ENDIF()
ENDIF()


# --------- Collect sources and targets ---------
SET(MMEX_BIN_DIR bin)

## Tuning for VisualStudio IDE ##
IF(MSVC)
    MESSAGE(STATUS "Tuning for Visual Studio IDE")

    IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        SET(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/install"
            CACHE PATH "Install directory used by INSTALL target" FORCE)
    ENDIF()
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_INSTALL_PREFIX}/${MMEX_BIN_DIR}>)
    IF(NOT CMAKE_VERSION VERSION_LESS 3.3)
        SET(CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD ON)
    ENDIF()
ENDIF()


## Subdirectories with targets ##
ADD_SUBDIRECTORY(3rd)
ADD_SUBDIRECTORY(po)
ADD_SUBDIRECTORY(src)

## Tuning for VisualStudio IDE ##
IF(NOT CMAKE_VERSION VERSION_LESS 3.6)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${MMEX_EXE})
ENDIF()

# --------- Install ---------
# Before moving to INSTALL commands for macOS bundle this needs to be set
IF(APPLE)
    STRING(REGEX REPLACE "^(.).*\\.([0-9]+)$" "\\1\\2" MMEX_VERSION_UNSTABLE_SHORT ${MMEX_VERSION_UNSTABLE})

    # Configure variables that are substituted into the plist
    # https://cmake.org/cmake/help/latest/prop_tgt/MACOSX_BUNDLE_INFO_PLIST.html
    # https://developer.apple.com/library/archive/documentation/General/Reference/InfoPlistKeyReference/Introduction/Introduction.html

    SET_TARGET_PROPERTIES(${MMEX_EXE} PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
        MACOSX_BUNDLE_EXECUTABLE_NAME ${MMEX_EXE}
        MACOSX_BUNDLE_GUI_IDENTIFIER org.moneymanagerex.${MMEX_EXE}
        MACOSX_BUNDLE_BUNDLE_VERSION ${MMEX_VERSION}${MMEX_VERSION_UNSTABLE_SHORT}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${MMEX_VERSION}
        MACOSX_BUNDLE_LONG_VERSION_STRING ${MMEX_VERSION_FULL}
        MACOSX_BUNDLE_COPYRIGHT "Copyright © 2009-2017 Nikolay\nCopyright © 2011-2017 LiSheng\nCopyright © 2013-2017 James, Gabriele\nCopyright © 2010-2017 Stefano\nCopyright © 2009-2010 VaDiM, Wesley Ellis"
        MACOSX_BUNDLE_ICON_FILE ${MACOSX_APP_ICON_NAME}
        # Set the path to the plist template
        MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/resources/MacOSXBundleInfo.plist.in")
ENDIF()

IF(WIN32)
    IF(MMEX_PORTABLE_INSTALL)
        # Besides normal installable version, for windows the portable
        # version needs additionall files
        FILE(WRITE "${CMAKE_CURRENT_BINARY_DIR}/mmexini.db3" "")
        INSTALL(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/mmexini.db3"
            DESTINATION "${MMEX_DOC_DIR}")
    ENDIF()

    # add shared wxWidgets DLLs
    GET_FILENAME_COMPONENT(WXDLLSUFF "${wxWidgets_LIB_DIR}" NAME)
    IF(WXDLLSUFF MATCHES "_dll$")
        STRING(REPLACE _dll .dll WXDLLSUFF ${WXDLLSUFF})
        SET(WXDLLPREF ${wxWidgets_VERSION_MAJOR}${wxWidgets_VERSION_MINOR})
        # DLL filenames are different for different versions
        # see https://github.com/wxWidgets/wxWidgets/blob/master/build/tools/msvs/package.bat#L42
        IF(wxWidgets_VERSION_MINOR MATCHES "^[13579]$")
            SET(WXDLLPREF ${WXDLLPREF}${wxWidgets_VERSION_PATCH})
        ENDIF()
        SET(WXDLLPREF ${WXDLLPREF}u)
        
        IF(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)
            LIST(APPEND MMEX_WXDLLS
                wxbase${WXDLLPREF}
                wxbase${WXDLLPREF}_xml
                wxmsw${WXDLLPREF}_adv
                wxmsw${WXDLLPREF}_aui
                wxmsw${WXDLLPREF}_core
                wxmsw${WXDLLPREF}_html
                wxmsw${WXDLLPREF}_qa
                wxmsw${WXDLLPREF}_stc
                wxmsw${WXDLLPREF}_webview)
        ENDIF()
        IF(CMAKE_INSTALL_DEBUG_LIBRARIES)
            SET(WXDLLPREF ${WXDLLPREF}d)
            LIST(APPEND MMEX_WXDLLS
                wxbase${WXDLLPREF}
                wxbase${WXDLLPREF}_xml
                wxmsw${WXDLLPREF}_adv
                wxmsw${WXDLLPREF}_aui
                wxmsw${WXDLLPREF}_core
                wxmsw${WXDLLPREF}_html
                wxmsw${WXDLLPREF}_qa
                wxmsw${WXDLLPREF}_stc
                wxmsw${WXDLLPREF}_webview)
        ENDIF()
        UNSET(WXDLLPREF)

        # check custom build name vs original one
        FOREACH(m_dll ${MMEX_WXDLLS})
            SET(m_dll "${wxWidgets_LIB_DIR}/${m_dll}")
            FOREACH(m_ext vc_mmex.dll vc_custom.dll vc_x64_custom.dll "${WXDLLSUFF}" NOTFOUND)
                IF(EXISTS "${m_dll}_${m_ext}")
                    LIST(APPEND CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS "${m_dll}_${m_ext}")
                    BREAK()
                ELSEIF(NOT m_ext)
                    MESSAGE(SEND_ERROR "${m_dll}_${WXDLLSUFF} not found")
                ENDIF()
            ENDFOREACH()
        ENDFOREACH()
        UNSET(m_dll)
        UNSET(m_ext)
        UNSET(MMEX_WXDLLS)
        UNSET(WXDLLSUFF)
    ENDIF()

    # This may be required to run on old Windows version
    SET(CMAKE_INSTALL_UCRT_LIBRARIES ON)

    # Following can be used for Debug builds, but
    # cannot be redistributed according to
    # https://www.visualstudio.com/productinfo/2017-redistribution-vs#visual-c-runtime-files
    # https://www.visualstudio.com/license-terms/distributable-code-microsoft-visual-studio-2015-rc-microsoft-visual-studio-2015-sdk-rc-includes-utilities-buildserver-files/#visual-c-runtime
    # https://docs.microsoft.com/cpp/ide/determining-which-dlls-to-redistribute
    # SET(CMAKE_INSTALL_DEBUG_LIBRARIES ON)
    # SET(CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY ON)

    INCLUDE(InstallRequiredSystemLibraries)
ENDIF()

# Help Files
INSTALL(FILES
    contrib.txt
    license.txt
    version.txt
    README.TXT
    DESTINATION "${MMEX_DOC_DIR}")
INSTALL(DIRECTORY
    docs/
    DESTINATION "${MMEX_DOC_DIR}/help")
INSTALL(FILES
    resources/master.css
    DESTINATION "${MMEX_DOC_DIR}/help")

# Resources
INSTALL(FILES
    3rd/ChartNew.js/ChartNew.js
    3rd/ChartNew.js/Add-ins/format.js
    3rd/sorttable.js/sorttable.js
    resources/home_page.htt
    resources/kaching.wav
    resources/master.css
    DESTINATION "${MMEX_RES_DIR}")

IF(LINUX)
    # .desktop File
    INSTALL(FILES
        resources/mmex.desktop
        DESTINATION share/applications)
ENDIF()

# Icons
IF(LINUX)
    INSTALL(FILES
        resources/mmex.svg
        DESTINATION share/icons/hicolor/scalable/apps)
ELSEIF(APPLE)
    INSTALL(FILES
        "${MACOSX_APP_ICON_FILE}"
        DESTINATION "${MMEX_RES_DIR}")
ELSEIF(WIN32)
    INSTALL(FILES
        resources/mmex.ico
        DESTINATION "${MMEX_RES_DIR}")
ENDIF()


# --------- CPack ---------
SET(CPACK_PACKAGE_NAME ${MMEX_EXE})
SET(CPACK_PACKAGE_VERSION ${MMEX_VERSION_FULL})
SET(CPACK_PACKAGE_VERSION_MAJOR ${MMEX_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${MMEX_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${MMEX_VERSION_PATCH})
SET(CPACK_PACKAGE_CONTACT "MoneyManagerEx Developers <developers@moneymanagerex.org>")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Simple to use financial management software")
SET(CPACK_PACKAGE_DESCRIPTION "Money Manager Ex (MMEX) is a free, open-source, cross-platform, easy-to-use personal finance software. It primarily helps organize one's finances and keeps track of where, when and how the money goes. MMEX includes all the basic features that 90% of users would want to see in a personal finance application. The design goals are to concentrate on simplicity and user friendliness - something one can use everyday.")
SET(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/resources/mmex.ico")
SET(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/license.txt")
SET(CPACK_RESOURCE_FILE_README "${PROJECT_SOURCE_DIR}/README.TXT")
# TODO: do we package Debug builds with stripped binaries?
SET(CPACK_STRIP_FILES ON)
SET(CPACK_DEBIAN_PACKAGE_SECTION misc)
SET(CPACK_DEBIAN_PACKAGE_PRIORITY extra)
SET(CPACK_DEBIAN_PACKAGE_HOMEPAGE http://www.moneymanagerex.org/)
SET(CPACK_DEBIAN_PACKAGE_REPLACES moneymanagerex)
SET(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
SET(CPACK_DEBIAN_PACKAGE_RELEASE 1)
SET(CPACK_DEBIAN_PACKAGE_DEPENDS wx${wxWidgets_VERSION_MAJOR}.${wxWidgets_VERSION_MINOR}-i18n)
SET(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
SET(CPACK_RPM_FILE_NAME RPM-DEFAULT)
SET(CPACK_RPM_PACKAGE_LICENSE GPL-2+)
SET(CPACK_RPM_PACKAGE_URL ${CPACK_DEBIAN_PACKAGE_HOMEPAGE})
SET(CPACK_RPM_PACKAGE_OBSOLETES ${CPACK_DEBIAN_PACKAGE_REPLACES})
SET(CPACK_RPM_PACKAGE_AUTOREQ ON)
SET(CPACK_RPM_PACKAGE_DESCRIPTION ${CPACK_PACKAGE_DESCRIPTION})
SET(CPACK_RPM_PACKAGE_RELEASE_DIST ON)
SET(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
    /usr/share/applications
    /usr/share/icons
    /usr/share/icons/hicolor
    /usr/share/icons/hicolor/scalable
    /usr/share/icons/hicolor/scalable/apps)
# TODO: distribute dbg packages
# SET(CPACK_RPM_DEBUGINFO_PACKAGE ON)
# SET(CPACK_RPM_DEBUGINFO_FILE_NAME RPM-DEFAULT)

IF(LINUX)
    IF(LINUX_DISTRO STREQUAL "Ubuntu" OR LINUX_DISTRO STREQUAL "Debian" OR LINUX_DISTRO STREQUAL "LinuxMint" OR LINUX_DISTRO STREQUAL "Raspbian")
        SET(CPACK_GENERATOR DEB)
        SET(CPACK_DEBIAN_PACKAGE_RELEASE "${CPACK_DEBIAN_PACKAGE_RELEASE}~${LINUX_DISTRO_CODENAME}")
    ELSEIF(LINUX_DISTRO STREQUAL "Fedora")
        SET(CPACK_GENERATOR RPM)
        SET(CPACK_RPM_PACKAGE_RELEASE_DIST ".fc${LINUX_DISTRO_REL}")
    ELSEIF(LINUX_DISTRO STREQUAL "CentOS")
        SET(CPACK_GENERATOR RPM)
        STRING(REGEX REPLACE "\\..*" "" CPACK_RPM_PACKAGE_RELEASE_DIST ${LINUX_DISTRO_REL})
        SET(CPACK_RPM_PACKAGE_RELEASE_DIST ".el${CPACK_RPM_PACKAGE_RELEASE_DIST}.centos")
    ELSEIF(LINUX_DISTRO MATCHES "openSUSE")
        SET(CPACK_GENERATOR RPM)
        SET(CPACK_RPM_PACKAGE_REQUIRES wxWidgets-lang)
        SET(CPACK_RPM_PACKAGE_RELEASE_DIST ".opensuse${LINUX_DISTRO_REL}")
        SET(CPACK_RPM_PACKAGE_RELEASE 1)
    ELSEIF(LINUX_DISTRO STREQUAL "Slackware")
        SET(CPACK_GENERATOR TXZ)            
        IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
            SET(CPACK_SYSTEM_NAME x86_64)
        ELSE()
            SET(CPACK_SYSTEM_NAME i586)
        ENDIF()
        INSTALL(FILES util/slackware-desc
                DESTINATION /install RENAME slack-desc)
        INSTALL(PROGRAMS util/slackware-doinst.sh
                DESTINATION /install RENAME doinst.sh)
        SET(CPACK_SYSTEM_NAME ${CPACK_SYSTEM_NAME}-1_slack${LINUX_DISTRO_REL})
        SET(CPACK_SET_DESTDIR ON)
        SET(CPACK_PACKAGING_INSTALL_PREFIX /usr) # not working?
        SET(CMAKE_INSTALL_PREFIX /usr)
        SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
    ELSEIF(LINUX_DISTRO STREQUAL "Arch")
    ELSE()
        MESSAGE(WARNING "Unknown Linux distribution - generating generic package.")
    ENDIF()
ELSEIF(APPLE)
    SET(CPACK_GENERATOR DragNDrop)
    SET(CPACK_DMG_VOLUME_NAME "Money Manager Ex")
    SET(CPACK_DMG_FORMAT UDZO)
    SET(CPACK_DMG_BACKGROUND_IMAGE "${PROJECT_SOURCE_DIR}/resources/dmg-background.png")
    SET(CPACK_DMG_DS_STORE "${PROJECT_SOURCE_DIR}/resources/dmg-DS_Store")
    SET(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/resources/mmex-package.icns")
    SET(CPACK_BUNDLE_PLIST "${PROJECT_SOURCE_DIR}/resources/Info.plist")
ELSEIF(WIN32)
    # With using http://nsis.sourceforge.net/Main_Page
    SET(CPACK_GENERATOR NSIS ZIP)
    SET(CPACK_PACKAGE_INSTALL_DIRECTORY "Money Manager EX")
    SET(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/resources\\\\mmex.ico")
    SET(CPACK_NSIS_MUI_ICON "${CPACK_PACKAGE_ICON}")
    SET(CPACK_NSIS_MUI_UNIICON "${CPACK_PACKAGE_ICON}")
    SET(CPACK_NSIS_INSTALLED_ICON_NAME "${MMEX_BIN_DIR}\\\\${MMEX_EXE}${CMAKE_EXECUTABLE_SUFFIX}")
    SET(CPACK_NSIS_MUI_FINISHPAGE_RUN ${MMEX_EXE}${CMAKE_EXECUTABLE_SUFFIX})
    SET(CPACK_NSIS_URL_INFO_ABOUT ${CPACK_DEBIAN_PACKAGE_HOMEPAGE})
    SET(CPACK_NSIS_CONTACT "${CPACK_PACKAGE_CONTACT}")
    SET(CPACK_NSIS_CREATE_ICONS_EXTRA
            "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Money Manager EX.lnk' '$INSTDIR\\\\${MMEX_BIN_DIR}\\\\${MMEX_EXE}${CMAKE_EXECUTABLE_SUFFIX}'")
    SET(CPACK_NSIS_DELETE_ICONS_EXTRA "Delete '$SMPROGRAMS\\\\$START_MENU\\\\Money Manager EX.lnk'")
    SET(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
ENDIF()

IF(LINUX_DISTRO STREQUAL "Arch")
    # Use dedicated tool to build Arch Linux package
    SET(CMAKE_INSTALL_PREFIX /usr)
    IF (CMAKE_SIZEOF_VOID_P EQUAL 8)
        SET(ARCHLINUX_ARCH x86_64)
    ELSE()
        SET(ARCHLINUX_ARCH i686)
    ENDIF()
    STRING(REPLACE - . CPACK_PACKAGE_VERSION ${MMEX_VERSION_FULL})
    CONFIGURE_FILE(util/PKGBUILD.in PKGBUILD @ONLY)

    # allow use reserved for cpack package target
    CMAKE_POLICY(PUSH)
    IF(POLICY CMP0037)
        CMAKE_POLICY(SET CMP0037 OLD)
    ENDIF()
    ADD_CUSTOM_TARGET(package
        COMMAND sh -c "if [ `id -u` -eq 0 ]; then chown nobody -R . && runuser nobody -c makepkg; else makepkg; fi"
        VERBATIM)
    CMAKE_POLICY(POP)
ELSE()
    # workarount for old CMake without DEB-DEFAULT/RPM-DEFAULT support
    IF(CMAKE_VERSION VERSION_LESS 3.6)
        IF(CPACK_GENERATOR STREQUAL "DEB")
            IF(NOT CPACK_DEBIAN_PACKAGE_ARCHITECTURE)
                FIND_PROGRAM(DPKG_CMD dpkg DOC "Debian packaging tool")
                IF(NOT DPKG_CMD)
                    MESSAGE("CPackDeb: Can not find dpkg in your path, default architecture to i386.")
                    SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE i386)
                ENDIF()
                EXECUTE_PROCESS(COMMAND "${DPKG_CMD}" --print-architecture
                    OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
                    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
            ENDIF()
            SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}-${CPACK_DEBIAN_PACKAGE_RELEASE}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
        ELSEIF(CPACK_GENERATOR STREQUAL "RPM")
            IF(NOT CPACK_RPM_PACKAGE_ARCHITECTURE)
                EXECUTE_PROCESS(COMMAND uname -m
                    OUTPUT_VARIABLE CPACK_RPM_PACKAGE_ARCHITECTURE
                    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
            ENDIF()
            SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}${CPACK_RPM_PACKAGE_RELEASE_DIST}-${CPACK_RPM_PACKAGE_ARCHITECTURE}")
        ENDIF()
    ENDIF(CMAKE_VERSION VERSION_LESS 3.6)

    INCLUDE(CPack)
ENDIF()


# --------- Print summary after configuration stage ---------
MESSAGE(STATUS "")
MESSAGE(STATUS "MMEX configuration summary")
MESSAGE(STATUS "==========================")
MESSAGE(STATUS "Version        : ${MMEX_VERSION_FULL}")
IF(GIT_COMMIT_HASH)
    MESSAGE(STATUS "Commit         : ${GIT_COMMIT_HASH} (${GIT_COMMIT_DATE})")
ENDIF()
IF(GIT_BRANCH)
    MESSAGE(STATUS "Branch         : ${GIT_BRANCH}")
ENDIF()
MESSAGE(STATUS "Host system    : ${CMAKE_HOST_SYSTEM_NAME} ${CMAKE_HOST_SYSTEM_PROCESSOR}")
IF(APPLE AND CMAKE_OSX_DEPLOYMENT_TARGET)
    MESSAGE(STATUS "Target system  : ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR} >= ${CMAKE_OSX_DEPLOYMENT_TARGET}")
ELSE()
    MESSAGE(STATUS "Target system  : ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
ENDIF()
IF(CMAKE_BUILD_TYPE)
    MESSAGE(STATUS "Build types    : ${CMAKE_BUILD_TYPE}")
ELSE()
    MESSAGE(STATUS "Build types    : ${CMAKE_CONFIGURATION_TYPES}")
ENDIF()
MESSAGE(STATUS "Generator      : ${CMAKE_GENERATOR}")
IF(MSVC)
    IF(CMAKE_VS_PLATFORM_TOOLSET)
        MESSAGE(STATUS "Toolset        : ${CMAKE_VS_PLATFORM_TOOLSET}")
        IF(CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE)
            MESSAGE(STATUS "Toolset host   : ${CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE}")
        ENDIF()
    ENDIF()
    IF(CMAKE_VS_PLATFORM_NAME)
        MESSAGE(STATUS "Platform       : ${CMAKE_VS_PLATFORM_NAME}")
    ENDIF()
    MESSAGE(STATUS "VC install dir : $ENV{VCINSTALLDIR}")
    IF(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
        FOREACH(m_dll ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
            GET_FILENAME_COMPONENT(m_dll ${m_dll} NAME_WE)
            SET(m_dlls "${m_dlls} ${m_dll}")
        ENDFOREACH()
        MESSAGE(STATUS "Run-time DLLs  :${m_dlls}")
    ENDIF()
ENDIF()
MESSAGE(STATUS "Install prefix : ${CMAKE_INSTALL_PREFIX}")
IF(WIN32)
    MESSAGE(STATUS "Portable state : ${MMEX_PORTABLE_INSTALL}")
ENDIF()
MESSAGE(STATUS "Webserver      : ${MMEX_WEBSERVER}")
MESSAGE(STATUS "DB encryption  : ${WXSQLITE3_HAVE_CODEC}")

MESSAGE(STATUS "")
MESSAGE(STATUS "Versions")
MESSAGE(STATUS "========")
MESSAGE(STATUS "${CMAKE_HOST_SYSTEM_NAME} ${CMAKE_HOST_SYSTEM_VERSION}")
IF(LINUX_DISTRO_STRING)
    MESSAGE(STATUS "${LINUX_DISTRO_STRING}")
ENDIF()
MESSAGE(STATUS "CMake ${CMAKE_VERSION}")
MESSAGE(STATUS "${CMAKE_MAKE_VERSION}")
IF(CCACHE_PROGRAM)
    GET_FILENAME_COMPONENT(CCACHE_TOOL "${CCACHE_PROGRAM}" NAME_WE)
    IF(CCACHE_VERSION)
        MESSAGE(STATUS "${CCACHE_TOOL} ${CCACHE_VERSION}")
    ELSE()
        MESSAGE(STATUS ${CCACHE_TOOL})
    ENDIF()
ENDIF()
IF(VS_VERSION)
    MESSAGE(STATUS "Visual Studio ${VS_VERSION}")
ENDIF()
IF(CMAKE_CXX_COMPILER_ID)
    MESSAGE(STATUS "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
ENDIF()
IF(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
    MESSAGE(STATUS "Win SDK ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
ENDIF()
IF(wxWidgets_VERSION_STRING)
    MESSAGE(STATUS "wxWidgets ${wxWidgets_VERSION_STRING}")
ENDIF()
IF(wxSQLite3_VERSION)
    MESSAGE(STATUS "wxSQLite3 ${wxSQLite3_VERSION}")
ENDIF()
IF(RapidJSON_VERSION)
    MESSAGE(STATUS "RapidJSON ${RapidJSON_VERSION}")
ENDIF()
IF(Lua_VERSION)
    MESSAGE(STATUS "Lua ${Lua_VERSION}")
ENDIF()
IF(CURL_VERSION)
    MESSAGE(STATUS "curl ${CURL_VERSION}")
ENDIF()
MESSAGE(STATUS "gettext ${GETTEXT_VERSION_STRING}")
MESSAGE(STATUS "")
