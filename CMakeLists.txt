CMAKE_MINIMUM_REQUIRED(VERSION 3.0 FATAL_ERROR)
SET(InMMEX TRUE)

PROJECT(MMEX VERSION 1.4.0)
OPTION(MMEX_BUILD_TESTS "Whether to build automatic unit tests" OFF)
OPTION(MMEX_PORTABLE_INSTALL "Include an empty mmexini.db3 file in the installation" ON) 

# Name of the resulted executable binary
SET(MMEX_EXE mmex)

# ---------------------------------------------------------------------------
# MMEX uses semantic versioning Ref: http://semver.org
#
# PROJECT VERSION - Format: MAJOR.MINOR.PATCH
# Update the version definition for the PROJECT as follows:
# 1. MAJOR version when incompatible API changes are made.
# 2. MINOR version adding functionality in a backwards-compatible manner.
# 3. PATCH version makeing backwards-compatible bug fixes.
#
# Alpha, Beta, RC = -1 (Stable) won't add any suffix
# Alpha, Beta, RC = 0 (Unstable) will add suffix to version without number
# Alpha, Beta, RC > 0 (Unstable) will add suffix to version with number
# ---------------------------------------------------------------------------
# By setting the application version here, CMake will transfer the variable
# values to the corresponding variable names in the following files:
# ./src/versions.h
# ./resources/mmex.rc
# ---------------------------------------------------------------------------
SET(MMEX_VERSION_ALPHA 3)
SET(MMEX_VERSION_BETA -1)
SET(MMEX_VERSION_RC -1)

# Adjusting semantic versioning for alpha/beta/rc version
IF(MMEX_VERSION_ALPHA EQUAL 0)
    SET(MMEX_VERSION_UNSTABLE alpha)
ELSEIF(MMEX_VERSION_ALPHA GREATER 0)
    SET(MMEX_VERSION_UNSTABLE alpha${MMEX_VERSION_ALPHA})
ELSEIF(MMEX_VERSION_BETA EQUAL 0)
    SET(MMEX_VERSION_UNSTABLE beta)
ELSEIF(MMEX_VERSION_BETA GREATER 0)
    SET(MMEX_VERSION_UNSTABLE beta${MMEX_VERSION_BETA})
ELSEIF(MMEX_VERSION_RC EQUAL 0)
    SET(MMEX_VERSION_UNSTABLE rc)
ELSEIF(MMEX_VERSION_RC GREATER 0)
    SET(MMEX_VERSION_UNSTABLE rc${MMEX_VERSION_RC})
ENDIF()
SET(MMEX_VERSION_WITH_UNSTABLE ${MMEX_VERSION})
IF(MMEX_VERSION_UNSTABLE)
    SET(MMEX_VERSION_WITH_UNSTABLE "${MMEX_VERSION}~${MMEX_VERSION_UNSTABLE}")
ENDIF()

# Find Git commit details to put into versions.h later
FIND_PACKAGE(Git)
IF(Git_FOUND)
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
ENDIF()

# detect target arch
IF(NOT APPLE)
    IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
        SET(TARGET_ARCH x86)
    ELSE(CMAKE_SIZEOF_VOID_P EQUAL 4)
        SET(TARGET_ARCH x64)
    ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 4)
ENDIF(NOT APPLE)

IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # define shortcut
    SET(LINUX TRUE)

    # Detect Linux distribution (if possible)
    FIND_PROGRAM(LSB_RELEASE lsb_release
        DOC "tool to read linux distribution name")
    IF(LSB_RELEASE-NOTFOUND)
        MESSAGE(SEND_ERROR "lsb_release tool not found.")
    ELSE()
        EXECUTE_PROCESS(COMMAND lsb_release -is
            OUTPUT_VARIABLE LINUX_DISTRO
            OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
        EXECUTE_PROCESS(COMMAND lsb_release -cs
            OUTPUT_VARIABLE LINUX_DISTRO_CODENAME
            OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
        EXECUTE_PROCESS(COMMAND lsb_release -rs
            OUTPUT_VARIABLE LINUX_DISTRO_REL
            OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
        IF(NOT LINUX_DISTRO OR NOT LINUX_DISTRO_CODENAME OR NOT LINUX_DISTRO_REL)
            MESSAGE(SEND_ERROR "Could not find a GNU/Linux distribution info")
        ENDIF()
        IF(LINUX_DISTRO_CODENAME STREQUAL "n/a")
            UNSET(LINUX_DISTRO_CODENAME)
        ENDIF()
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
ELSEIF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # using regular Clang or AppleClang
    IF(LINUX_DISTRO STREQUAL "Ubuntu")
        # Due to bug in Ubuntu
        # http://stackoverflow.com/questions/37096062/get-a-basic-c-program-to-compile-using-clang-on-ubuntu-16
        SET(CMAKE_CXX_FLAGS "-stdlib=libstdc++ ${CMAKE_CXX_FLAGS}")
    ELSEIF(APPLE)
        SET(CMAKE_CXX_FLAGS "-stdlib=libc++ ${CMAKE_CXX_FLAGS}")
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
    # Real setting of icons
    SET(MACOSX_BUNDLE_ICON_FILE "${MACOSX_APP_ICON_NAME}")

    SET(MMEX_DOC_DIR ${MMEX_EXE}.app/Contents/SharedSupport/doc)
    SET(MMEX_RES_DIR ${MMEX_EXE}.app/Contents/Resources)
    SET(MMEX_LOCALE_DIR ${MMEX_EXE}.app/Contents/SharedSupport/locale/en)
ELSEIF(WIN32)
    SET(MMEX_DOC_DIR .)
    SET(MMEX_RES_DIR res)
    SET(MMEX_LOCALE_DIR locale/en)
ELSE()
    SET(MMEX_DOC_DIR share/doc/mmex)
    SET(MMEX_RES_DIR share/mmex/res)
    SET(MMEX_LOCALE_DIR share/mmex/locale/en)
ENDIF()


# --------- wxWidgets dependency ---------
SET(wxWidgets_USE_REL_AND_DBG ON)
IF(WIN32)
    #wxScintilla is needed on Windows
    FIND_PACKAGE(wxWidgets REQUIRED core qa net html xml aui adv stc webview base scintilla REQUIRED)
ELSE()
    FIND_PACKAGE(wxWidgets REQUIRED core qa net html xml aui adv stc webview base REQUIRED)
ENDIF()
IF(wxWidgets_VERSION_STRING AND wxWidgets_VERSION_STRING VERSION_LESS 2.9.2)
    MESSAGE(SEND_ERROR "wxWidget version to old.")
ENDIF()
ADD_LIBRARY(wxWidgets INTERFACE)
TARGET_INCLUDE_DIRECTORIES(wxWidgets INTERFACE ${wxWidgets_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(wxWidgets INTERFACE ${wxWidgets_LIBRARIES})
TARGET_COMPILE_OPTIONS(wxWidgets INTERFACE ${wxWidgets_CXX_FLAGS})
TARGET_COMPILE_DEFINITIONS(wxWidgets INTERFACE ${wxWidgets_DEFINITIONS}
    $<$<CONFIG:Debug>:${wxWidgets_DEFINITIONS_DEBUG}>)


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
IF(MMEX_BUILD_TESTS)
    IF(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)
        MESSAGE(SEND_ERROR "MMEX_BUILD_TESTS not supported under MSVC x64.")
    ENDIF()
    ADD_SUBDIRECTORY(tests)
ENDIF()

## Tuning for VisualStudio IDE ##
IF(NOT CMAKE_VERSION VERSION_LESS 3.6)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${MMEX_EXE})
ENDIF()

# --------- Install ---------
# Before moving to INSTALL commands for macOS bundle this needs to be set
IF(APPLE)
    # Configure variables that are substituted into the plist
    # https://cmake.org/cmake/help/latest/prop_tgt/MACOSX_BUNDLE_INFO_PLIST.html

    SET_TARGET_PROPERTIES(${MMEX_EXE} PROPERTIES
        # Set CFBundleName, which controls the application menu label
        MACOSX_BUNDLE_BUNDLE_NAME ${MMEX_EXE}
        # Set CFBundleVersion to the git describe output. Apple docs say it should be "three non-negative, period-separated integers with the first integer being greater than zero"
        MACOSX_BUNDLE_BUNDLE_VERSION ${MMEX_VERSION}
        # Set CFBundleExecutable
        #MACOSX_BUNDLE_EXECUTABLE_NAME "${OUTPUT_NAME}"
        # Set CFBundleShortVersionString to "2.0.0". This is displayed in the Finder and Spotlight.
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${MMEX_VERSION}
        # Set the path to the plist template
        MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/resources/Info.plist")
ENDIF()

# Besides normal installable version, for windows the portable version needs additionally files
IF(WIN32)
    IF(MMEX_PORTABLE_INSTALL)
        FILE(WRITE "${CMAKE_CURRENT_BINARY_DIR}/mmexini.db3" "")
        INSTALL(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/mmexini.db3"
            DESTINATION "${MMEX_DOC_DIR}")
    ENDIF()

    # add shared wxWidgets DLLs
    GET_FILENAME_COMPONENT(WXDLLSUFF "${wxWidgets_LIB_DIR}" NAME)
    IF(WXDLLSUFF MATCHES "_dll$")
        STRING(REPLACE _dll .dll WXDLLSUFF ${WXDLLSUFF})
        STRING(REPLACE . "" WXDLLPREF ${wxWidgets_VERSION_STRING})
        SET(WXDLLPREF ${WXDLLPREF}u)
        
        IF(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)
            LIST(APPEND MMEX_WXDLLS
                wxbase${WXDLLPREF}
                wxbase${WXDLLPREF}_net
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
                wxbase${WXDLLPREF}_net
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
            IF(EXISTS "${m_dll}_vc_custom.dll")
                LIST(APPEND CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS "${m_dll}_vc_custom.dll")
            ELSEIF(EXISTS "${m_dll}_${WXDLLSUFF}")
                LIST(APPEND CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS "${m_dll}_${WXDLLSUFF}")
            ELSE()
                MESSAGE(SEND_ERROR "${m_dll}_${WXDLLSUFF} not found")
            ENDIF()
        ENDFOREACH()
        UNSET(m_dll)
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
SET(CPACK_PACKAGE_VERSION ${MMEX_VERSION_WITH_UNSTABLE})
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
SET(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
SET(CPACK_RPM_FILE_NAME RPM-DEFAULT)
SET(CPACK_RPM_PACKAGE_LICENSE GPL-2+)
SET(CPACK_RPM_PACKAGE_URL ${CPACK_DEBIAN_PACKAGE_HOMEPAGE})
SET(CPACK_RPM_PACKAGE_OBSOLETES ${CPACK_DEBIAN_PACKAGE_REPLACES})
SET(CPACK_RPM_PACKAGE_AUTOREQ ON)
# TODO: distribute dbg packages
# SET(CPACK_RPM_DEBUGINFO_PACKAGE ON)
# SET(CPACK_RPM_DEBUGINFO_FILE_NAME RPM-DEFAULT)

IF(LINUX)
    IF(LINUX_DISTRO STREQUAL "Ubuntu")
        SET(CPACK_GENERATOR DEB)
        SET(CPACK_DEBIAN_PACKAGE_RELEASE "0ubuntu1~${LINUX_DISTRO_CODENAME}")
    ELSEIF(LINUX_DISTRO STREQUAL "Debian" OR LINUX_DISTRO STREQUAL "LinuxMint")
        SET(CPACK_GENERATOR DEB)
        SET(CPACK_DEBIAN_PACKAGE_RELEASE "1~${LINUX_DISTRO_CODENAME}")
    ELSEIF(LINUX_DISTRO STREQUAL "Fedora")
        SET(CPACK_GENERATOR RPM)
        SET(CPACK_RPM_PACKAGE_RELEASE_DIST ".fc${LINUX_DISTRO_REL}")
    ELSEIF(LINUX_DISTRO STREQUAL "CentOS")
        SET(CPACK_GENERATOR RPM)
        STRING(REGEX REPLACE "\\..*" "" CPACK_RPM_PACKAGE_RELEASE_DIST ${LINUX_DISTRO_REL})
        SET(CPACK_RPM_PACKAGE_RELEASE_DIST ".el${CPACK_RPM_PACKAGE_RELEASE_DIST}.centos")
    ELSEIF(LINUX_DISTRO MATCHES "openSUSE")
        SET(CPACK_GENERATOR RPM)
        SET(CPACK_RPM_PACKAGE_RELEASE_DIST ".opensuse${LINUX_DISTRO_REL}")
        SET(CPACK_RPM_PACKAGE_RELEASE 1)
    ELSEIF(NOT LINUX_DISTRO STREQUAL "Arch")
        MESSAGE(WARNING "Unknown Linux distribution - generating generic package.")
    ENDIF()
ELSEIF(APPLE)
    SET(CPACK_GENERATOR DragNDrop)
    SET(CPACK_DMG_VOLUME_NAME "Money Manager Ex")
    SET(CPACK_DMG_FORMAT UDZO)
    SET(CPACK_DMG_BACKGROUND_IMAGE "${PROJECT_SOURCE_DIR}/resources/dmg-background.png")
    SET(CPACK_DMG_DS_STORE "${PROJECT_SOURCE_DIR}/resources/.DS_Store")
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

IF(NOT LINUX_DISTRO STREQUAL "Arch")
    # workarount for old CMake without DEB-DEFAULT/RPM-DEFAULT support
    IF(CMAKE_VERSION VERSION_LESS 3.6)
        IF(CPACK_GENERATOR STREQUAL "DEB")
            FIND_PROGRAM(DPKG_CMD dpkg DOC "Debian packaging tool")
            IF(DPKG_CMD-NOTFOUND)
                MESSAGE("CPackDeb: Can not find dpkg in your path, default architecture to i386.")
                SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE i386)
            ENDIF()
            EXECUTE_PROCESS(COMMAND "${DPKG_CMD}" --print-architecture
                OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
                OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
            SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}-${CPACK_DEBIAN_PACKAGE_RELEASE}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
        ELSEIF(CPACK_GENERATOR STREQUAL "RPM")
            EXECUTE_PROCESS(COMMAND uname -m
                OUTPUT_VARIABLE CPACK_RPM_PACKAGE_ARCHITECTURE
                OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
            SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}${CPACK_RPM_PACKAGE_RELEASE_DIST}-${CPACK_RPM_PACKAGE_ARCHITECTURE}")
        ENDIF()
    ENDIF(CMAKE_VERSION VERSION_LESS 3.6)

    INCLUDE(CPack)
ELSE()
    # Use dedicated tool to build Arch Linux package
    IF (TARGET_ARCH STREQUAL "x64")
        SET(ARCHLINUX_ARCH x86_64)
    ELSE()
        SET(ARCHLINUX_ARCH i686)
    ENDIF()
    CONFIGURE_FILE(util/PKGBUILD.in PKGBUILD @ONLY)
    SET(CMAKE_INSTALL_PREFIX /usr)

    # allow use reserved for cpack package target
    CMAKE_POLICY(PUSH)
    IF(POLICY CMP0037)
        CMAKE_POLICY(SET CMP0037 OLD)
    ENDIF()
    ADD_CUSTOM_TARGET(package
        COMMAND sh -c "if [ `id -u` -eq 0 ]; then chown nobody -R . ../3rd/cppunit && su nobody -c makepkg; else makepkg; fi"
        VERBATIM)
    CMAKE_POLICY(POP)
ENDIF()


# --------- Print summary after configuration stage ---------
MESSAGE(STATUS "")
MESSAGE(STATUS "MMEX configuration summary")
MESSAGE(STATUS "==========================")
MESSAGE(STATUS "Version        : ${MMEX_VERSION_WITH_UNSTABLE}")
IF(GIT_COMMIT_HASH)
    MESSAGE(STATUS "Commit         : ${GIT_COMMIT_HASH} (${GIT_COMMIT_DATE})")
ENDIF()
IF(GIT_BRANCH)
    MESSAGE(STATUS "Branch         : ${GIT_BRANCH}")
ENDIF()
MESSAGE(STATUS "Build tests    : ${MMEX_BUILD_TESTS}")
MESSAGE(STATUS "Host system    : ${CMAKE_HOST_SYSTEM_NAME} ${CMAKE_HOST_SYSTEM_PROCESSOR}")
MESSAGE(STATUS "Target system  : ${CMAKE_SYSTEM_NAME} ${TARGET_ARCH}")
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
MESSAGE(STATUS "Portable State : ${MMEX_PORTABLE_INSTALL}")

MESSAGE(STATUS "")
MESSAGE(STATUS "Versions")
MESSAGE(STATUS "========")
MESSAGE(STATUS "${CMAKE_HOST_SYSTEM_NAME} ${CMAKE_HOST_SYSTEM_VERSION}")
IF(LINUX_DISTRO)
    IF(LINUX_DISTRO_CODENAME)
        MESSAGE(STATUS "${LINUX_DISTRO} ${LINUX_DISTRO_REL} \"${LINUX_DISTRO_CODENAME}\"")
    ELSE()
        MESSAGE(STATUS "${LINUX_DISTRO} ${LINUX_DISTRO_REL}")
    ENDIF()
ENDIF()
MESSAGE(STATUS "CMake ${CMAKE_VERSION}")
MESSAGE(STATUS "${CMAKE_MAKE_VERSION}")
IF(VS_VERSION)
    MESSAGE(STATUS "Visual Studio ${VS_VERSION}")
ENDIF()
MESSAGE(STATUS "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
IF(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
    MESSAGE(STATUS "Win SDK ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
ENDIF()
IF(wxWidgets_VERSION_STRING)
    MESSAGE(STATUS "wxWidgets ${wxWidgets_VERSION_STRING}")
ENDIF()
MESSAGE(STATUS "gettext ${GETTEXT_VERSION_STRING}")
MESSAGE(STATUS "")
