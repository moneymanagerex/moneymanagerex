CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

SET(InMMEX TRUE)

PROJECT(MoneyManagerEx)

# Name of the resulted binary
SET(PROJECT_TARGET_BIN "mmex")

# ---------------------------------------------------------------------------
# Semantic versioning
# Ref: http://semver.org

# MMEX_VERSION - Format: MAJOR.MINOR.PATCH
# Update the version definition for the program as follows:
# 1. MAJOR version when incompatible API changes are made.
# 2. MINOR version adding functionality in a backwards-compatible manner.
# 3. PATCH version makeing backwards-compatible bug fixes.

# Alpha, Beta, RC  = -1 (Stable) won't add any suffix
# Alpha, Beta, RC  = 0 (Unstable) will add suffix to version without number
# Alpha, Beta, RC  > 0 (Unstable) will add suffix to version with number
# ---------------------------------------------------------------------------
# By setting the application version here, CMake will transfer the variable
# values to the corresponding variable names in the following files:
# ./src/constant.h
# ./resources/mmex.rc
# ---------------------------------------------------------------------------
SET(MoneyManagerEx_VERSION_MAJOR 1)
SET(MoneyManagerEx_VERSION_MINOR 4)
SET(MoneyManagerEx_VERSION_PATCH 0)
SET(MoneyManagerEx_VERSION_ALPHA 3)
SET(MoneyManagerEx_VERSION_BETA -1)
SET(MoneyManagerEx_VERSION_RC -1)

SET(MoneyManagerEx_VERSION "${MoneyManagerEx_VERSION_MAJOR}.${MoneyManagerEx_VERSION_MINOR}.${MoneyManagerEx_VERSION_PATCH}")

# Adjusting semantic versioning for ALPHA version release
IF(${MoneyManagerEx_VERSION_ALPHA} EQUAL 0)
    SET(CPACK_PACKAGE_FILE_NAME "${PROJECT_TARGET_BIN}-${MoneyManagerEx_VERSION}-ALPHA")
ELSEIF(${MoneyManagerEx_VERSION_ALPHA} GREATER 0)
    SET(CPACK_PACKAGE_FILE_NAME "${PROJECT_TARGET_BIN}-${MoneyManagerEx_VERSION}-ALPHA-${MoneyManagerEx_VERSION_ALPHA}")
ENDIF()

# Adjusting semantic versioning for BETA version release
IF(${MoneyManagerEx_VERSION_BETA} EQUAL 0)
    SET(CPACK_PACKAGE_FILE_NAME "${PROJECT_TARGET_BIN}-${MoneyManagerEx_VERSION}-BETA")
ELSEIF(${MoneyManagerEx_VERSION_BETA} GREATER 0)
    SET(CPACK_PACKAGE_FILE_NAME "${PROJECT_TARGET_BIN}-${MoneyManagerEx_VERSION}-BETA-${MoneyManagerEx_VERSION_BETA}")
ENDIF()

# Adjusting semantic versioning for RC version release
IF(${MoneyManagerEx_VERSION_RC} EQUAL 0)
    SET(CPACK_PACKAGE_FILE_NAME "${PROJECT_TARGET_BIN}-${MoneyManagerEx_VERSION}-RC")
ELSEIF(${MoneyManagerEx_VERSION_RC} GREATER 0)
    SET(CPACK_PACKAGE_FILE_NAME "${PROJECT_TARGET_BIN}-${MoneyManagerEx_VERSION}-RC-${MoneyManagerEx_VERSION_RC}")
ENDIF()

# Get the current working branch
EXECUTE_PROCESS(
  COMMAND git symbolic-ref --short -q HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
  TIMEOUT 4 ERROR_QUIET
)

# Get the latest abbreviated commit hash of the working branch
EXECUTE_PROCESS(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
  TIMEOUT 4 ERROR_QUIET
)

# Get the latest commit date of the working branch
EXECUTE_PROCESS(
  COMMAND git log -1 --format=%cd --date=short
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_DATE
  OUTPUT_STRIP_TRAILING_WHITESPACE
  TIMEOUT 4 ERROR_QUIET
)

IF(GIT_COMMIT_HASH)
  ADD_DEFINITIONS(-DGIT_COMMIT_DATE="${GIT_COMMIT_DATE}" -DGIT_COMMIT_HASH="${GIT_COMMIT_HASH}")
ENDIF()
IF(GIT_BRANCH)
  ADD_DEFINITIONS(-DGIT_BRANCH="${GIT_BRANCH}")
ENDIF()
ADD_DEFINITIONS(-DCMAKE_VERSION="${CMAKE_VERSION}" -DMAKE_VERSION="${MAKE_VERSION}")

# Passing CMake variables (specificaly current version numbers) to files
# Header file
CONFIGURE_FILE(
        src/constants.h.in
        "${PROJECT_SOURCE_DIR}/src/constants.h"
)
# VS resource script
CONFIGURE_FILE(
        resources/mmex.rc.in
        "${PROJECT_SOURCE_DIR}/resources/mmex.rc"
)

OPTION(BuildTests "Whether to build automatic unit tests" OFF)

MESSAGE(STATUS "Build Type = ${CMAKE_BUILD_TYPE}")
MESSAGE(STATUS "Building Tests = ${BuildTests}")
MESSAGE(STATUS "OS = ${CMAKE_SYSTEM_NAME}")

IF(NOT APPLE)
    IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
        SET(TARGET_ARCH "x86")
    ELSE(CMAKE_SIZEOF_VOID_P EQUAL 4)
        SET(TARGET_ARCH "x64")
    ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 4)
    MESSAGE(STATUS "Build arch = ${TARGET_ARCH}")
ENDIF(NOT APPLE)

IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    # Detect Linux distribution (if possible)
    EXECUTE_PROCESS(COMMAND sh "${PROJECT_SOURCE_DIR}/util/getDistroName.sh"
            TIMEOUT 4
            OUTPUT_VARIABLE LINUX_DISTRO
            #ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    MESSAGE(STATUS "Distro = ${LINUX_DISTRO}")
ENDIF()


# --------- Set compiler parameters ---------
SET(CMAKE_CXX_STANDARD 11)

IF(MSVC)
    # using Visual Studio C++
    MESSAGE(STATUS "Compiler = MSVC")
    # Based on this http://stackoverflow.com/a/8294669
    # added solution to compile problems due to
    # conflict between winsock and winsock2
    # Partialy reinvented fix from commit
    # commit 06accae1273e66ced469672151522e45eee685a9
    ADD_DEFINITIONS(-DWIN32_LEAN_AND_MEAN)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)

ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # using GCC
    MESSAGE(STATUS "Compiler = GNU")
    #TODO:

ELSEIF("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    # using regular Clang or AppleClang
    MESSAGE(STATUS "Compiler = Clang")

    IF(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND LINUX_DISTRO STREQUAL "Ubuntu")
        # Due to bug in Ubuntu
        # http://stackoverflow.com/questions/37096062/get-a-basic-c-program-to-compile-using-clang-on-ubuntu-16
        SET(CMAKE_CXX_FLAGS "-stdlib=libstdc++")
    ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        SET(CMAKE_CXX_FLAGS "-stdlib=libc++")
    ENDIF()
ENDIF()


# --------- Set various location ---------
SET(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")
SET(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/lib")
SET(INSTALL_BIN_DIR "bin")

IF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # Preparing for configure icons
    SET(MACOSX_APP_ICON_NAME "mmex.icns")
    # Setting path to icon
    SET(MACOSX_APP_ICON_FILE "${CMAKE_SOURCE_DIR}/resources/${MACOSX_APP_ICON_NAME}")
    # Real setting of icons
    SET(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/resources/mmex-package.icns")
    SET(MACOSX_BUNDLE_ICON_FILE "${MACOSX_APP_ICON_NAME}")
    SET(CPACK_BUNDLE_PLIST "${CMAKE_SOURCE_DIR}/resources/Info.plist")

    SET(INSTALL_DOC_DIR "${PROJECT_TARGET_BIN}.app/Contents/SharedSupport/doc")
    SET(INSTALL_RES_DIR "${PROJECT_TARGET_BIN}.app/Contents/Resources")
    SET(INSTALL_MO_DIR "${PROJECT_TARGET_BIN}.app/Contents/SharedSupport/po/en")
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    SET(INSTALL_DOC_DIR ".")
    SET(INSTALL_RES_DIR "res")
    SET(INSTALL_MO_DIR "po/en")
ELSE()
    SET(INSTALL_DOC_DIR "share/doc/mmex")
    SET(INSTALL_RES_DIR "share/mmex/res")
    SET(INSTALL_MO_DIR "share/mmex/po/en")
ENDIF()


# --------- Check for required libraries ---------
IF(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    #wxScintilla is needed on Windows
    FIND_PACKAGE(wxWidgets REQUIRED core qa net html xml aui adv stc webview base scintilla REQUIRED)
ELSE()
    FIND_PACKAGE(wxWidgets REQUIRED core qa net html xml aui adv stc webview base REQUIRED)
ENDIF()

INCLUDE(${wxWidgets_USE_FILE})
IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(wxWidgets_USE_DEBUG true)
ELSE()
    SET(wxWidgets_USE_DEBUG false)
ENDIF()

MESSAGE(STATUS "Base Directory =  ${CMAKE_SOURCE_DIR}")

INCLUDE_DIRECTORIES(
        ${wxWidgets_INCLUDE_DIRS}
        "src"
        "3rd"
        "3rd/cgitemplate"
        "3rd/csv-parser/include"
        "3rd/LuaGlue/include"
        "lib/cppunit/include"
        "lib/lua/src"
        "lib/wxsqlite3/include"
        "lib/wxsqlite3/sqlite3/secure/src/"
)

## Targets ##
ADD_LIBRARY(LUA STATIC "")
ADD_LIBRARY(wxSQLite3 STATIC "")
ADD_EXECUTABLE(${PROJECT_TARGET_BIN} WIN32 MACOSX_BUNDLE ${MACOSX_APP_ICON_FILE} "")
IF(BuildTests)
    ADD_LIBRARY(CPPUnit STATIC "")
    ADD_EXECUTABLE(MMEXTest "")
ENDIF()

## Subdirectories ##
ADD_SUBDIRECTORY(po)
ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(src)
IF(BuildTests)
    ADD_SUBDIRECTORY(tests)
ENDIF()

## Build and Link ##
IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    INCLUDE(CheckLibraryExists)
    CHECK_LIBRARY_EXISTS(${CMAKE_DL_LIBS} dlopen "" HAVE_DLOPEN)
    IF(NOT HAVE_DLOPEN)
        MESSAGE(FATAL_ERROR "Could not find required dl library.")
    ENDIF()
    TARGET_LINK_LIBRARIES(wxSQLite3 ${CMAKE_DL_LIBS})
    TARGET_LINK_LIBRARIES(LUA ${CMAKE_DL_LIBS})
ENDIF()
TARGET_LINK_LIBRARIES(${PROJECT_TARGET_BIN} ${wxWidgets_LIBRARIES} wxSQLite3 LUA)
IF(BuildTests)
    TARGET_LINK_LIBRARIES(MMEXTest CPPUnit ${wxWidgets_LIBRARIES} wxSQLite3 LUA)
ENDIF()


# --------- Install ---------
# Before moving to INSTALL commands for macOS bundle this needs to be set
IF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    #
    # Configure variables that are substituted into the plist
    # https://cmake.org/cmake/help/latest/prop_tgt/MACOSX_BUNDLE_INFO_PLIST.html
    #

    # Set CFBundleName, which controls the application menu label
    SET_TARGET_PROPERTIES(${PROJECT_TARGET_BIN} PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_TARGET_BIN}")

    # Set CFBundleVersion to the git describe output. Apple docs say it should be "three non-negative, period-separated integers with the first integer being greater than zero"
    SET_TARGET_PROPERTIES(${PROJECT_TARGET_BIN} PROPERTIES MACOSX_BUNDLE_BUNDLE_VERSION "${MoneyManagerEx_VERSION}")

    # Set CFBundleExecutable
    #SET_TARGET_PROPERTIES(${PROJECT_TARGET_BIN} PROPERTIES MACOSX_BUNDLE_EXECUTABLE_NAME "${OUTPUT_NAME}")

    # Set CFBundleShortVersionString to "2.0.0". This is displayed in the Finder and Spotlight.
    SET_TARGET_PROPERTIES(${PROJECT_TARGET_BIN} PROPERTIES MACOSX_BUNDLE_SHORT_VERSION_STRING "${MoneyManagerEx_VERSION}")

    # Set the path to the plist template
    SET_TARGET_PROPERTIES(${PROJECT_TARGET_BIN} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/resources/Info.plist")
ENDIF()

# Fixes for Visual Studio 2017 immediate build\execution\debug directly opening CMake folder without project
# Initialize IS_VS_2017CMAKE to 0, it will be override by Visual Studio cmake with -D command line parameter
SET(IS_VS_2017CMAKE 0 CACHE BOOL "Executed from VS2017 IDE")
IF(IS_VS_2017CMAKE EQUAL 1)
    SET(CONFIGURATION_NAME "${TARGET_ARCH}-${CMAKE_BUILD_TYPE}")
    SET(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/${CONFIGURATION_NAME}/bin")
    SET(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/${CONFIGURATION_NAME}/bin/lib")
    SET(INSTALL_BIN_DIR "${CMAKE_SOURCE_DIR}/bin/${CONFIGURATION_NAME}/bin")
    SET(INSTALL_DOC_DIR "${CMAKE_SOURCE_DIR}/bin/${CONFIGURATION_NAME}")
    SET(INSTALL_RES_DIR "${CMAKE_SOURCE_DIR}/bin/${CONFIGURATION_NAME}/res")
    SET(INSTALL_MO_DIR "${CMAKE_SOURCE_DIR}/bin/${CONFIGURATION_NAME}/po/en")
    ADD_CUSTOM_COMMAND(
        TARGET ${PROJECT_TARGET_BIN} POST_BUILD
        COMMAND cmake -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Copying files to output directory" VERBATIM
    )
ENDIF()
# Finally set ups for multiplatform INSTALL
# Binary
IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    INSTALL(PROGRAMS
            "${EXECUTABLE_OUTPUT_PATH}/${PROJECT_TARGET_BIN}"
            DESTINATION "${INSTALL_BIN_DIR}")
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    INSTALL(TARGETS
            "${PROJECT_TARGET_BIN}"
            BUNDLE DESTINATION .)
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # For VS2017 with cmake mmex.exe it's already in the right place
    IF(NOT IS_VS_2017CMAKE EQUAL 1)
        INSTALL(PROGRAMS
                "${EXECUTABLE_OUTPUT_PATH}/${PROJECT_TARGET_BIN}.exe"
                DESTINATION "${INSTALL_BIN_DIR}")
    ENDIF()
    
    # Besides normal installable version, for windows the portable version needs additionally files
    FILE(WRITE "${CMAKE_CURRENT_BINARY_DIR}/mmexini.db3" "")
    INSTALL(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/mmexini.db3"
            DESTINATION "${INSTALL_DOC_DIR}")

    IF(MSVC)
        FILE(TO_CMAKE_PATH "$ENV{VCINSTALLDIR}redist" BASEREDISTDIR)
        SET(REDISTDIR "${BASEREDISTDIR}/${TARGET_ARCH}/Microsoft.VC140.CRT")
        IF(NOT EXISTS REDISTDIR)
            # Visual Studio 2017 with VC 14.1 has changed redist path
            SET(REDISTDIR "${BASEREDISTDIR}/MSVC")
            FILE(GLOB REDISTSUBDIRS RELATIVE ${REDISTDIR} ${REDISTDIR}/*)
            FOREACH(REDISTSUBDIR ${REDISTSUBDIRS})
                IF(IS_DIRECTORY ${REDISTDIR}/${REDISTSUBDIR})
                    SET(REDISTDIR "${REDISTDIR}/${REDISTSUBDIR}/${TARGET_ARCH}/Microsoft.VC141.CRT")
                ENDIF()
            ENDFOREACH()
        ENDIF()
        SET(VCRUNTIME
            "${REDISTDIR}/msvcp140.dll"
            "${REDISTDIR}/vcruntime140.dll"
            )
        FOREACH(f IN LISTS VCRUNTIME)
            IF(NOT EXISTS ${f})
                MESSAGE(WARNING "Can not copy ${f}. File does not exist.")
            ENDIF()
        ENDFOREACH(f)
        INSTALL(FILES ${VCRUNTIME} DESTINATION "${INSTALL_BIN_DIR}" OPTIONAL)
    ENDIF(MSVC)
ENDIF()

# Help Files
INSTALL(FILES
        "${CMAKE_SOURCE_DIR}/contrib.txt"
        "${CMAKE_SOURCE_DIR}/license.txt"
        "${CMAKE_SOURCE_DIR}/version.txt"
        "${CMAKE_SOURCE_DIR}/README.TXT"
        DESTINATION "${INSTALL_DOC_DIR}")
INSTALL(DIRECTORY
        "${CMAKE_SOURCE_DIR}/docs/"
        DESTINATION "${INSTALL_DOC_DIR}/help")
INSTALL(FILES
        "${CMAKE_SOURCE_DIR}/resources/master.css"
        DESTINATION "${INSTALL_DOC_DIR}/help")

# Resources
INSTALL(FILES
        "${CMAKE_SOURCE_DIR}/3rd/ChartNew.js/ChartNew.js"
        "${CMAKE_SOURCE_DIR}/3rd/sorttable.js/sorttable.js"
        "${CMAKE_SOURCE_DIR}/resources/home_page.htt"
        "${CMAKE_SOURCE_DIR}/resources/kaching.wav"
        "${CMAKE_SOURCE_DIR}/resources/master.css"
        DESTINATION "${INSTALL_RES_DIR}")

IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # .desktop File
    INSTALL(FILES
            "${CMAKE_SOURCE_DIR}/resources/mmex.desktop"
            DESTINATION "share/applications")
ENDIF()

# Icons
IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    INSTALL(FILES
            "${CMAKE_SOURCE_DIR}/resources/mmex.svg"
            DESTINATION "share/icons/hicolor/scalable/apps")
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    INSTALL(FILES
            "${MACOSX_APP_ICON_FILE}"
            DESTINATION ${INSTALL_RES_DIR})
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    INSTALL(FILES
            "${CMAKE_SOURCE_DIR}/resources/mmex.ico"
            DESTINATION "${INSTALL_RES_DIR}")
ENDIF()


# --------- CPack ---------
SET(CPACK_PACKAGE_NAME "${PROJECT_TARGET_BIN}")
# CPACK_PACKAGE_VERSION "1.2.3") # to produce something like this
SET(CPACK_PACKAGE_VERSION "${MoneyManagerEx_VERSION}")
SET(CPACK_PACKAGE_VERSION_MAJOR "${MoneyManagerEx_VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${MoneyManagerEx_VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${MoneyManagerEx_VERSION_PATCH}")
SET(CPACK_PACKAGE_CONTACT "MoneyManagerEx Developers <developers@moneymanagerex.org>")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Simple to use financial management software")
SET(CPACK_PACKAGE_DESCRIPTION "Money Manager Ex (MMEX) is a free, open-source, cross-platform, easy-to-use personal finance software. It primarily helps organize one's finances and keeps track of where, when and how the money goes. MMEX includes all the basic features that 90% of users would want to see in a personal finance application. The design goals are to concentrate on simplicity and user friendliness - something one can use everyday.")
SET(CPACK_RPM_PACKAGE_LICENSE "GPL-2+")
SET(CPACK_STRIP_FILES ON)

# Linux
# Set package run time dependencies
IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")

    # Ubuntu
    IF(EXISTS "/etc/debian_version")
        SET(CPACK_GENERATOR "DEB")
        SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libwxgtk3.0-0v5, libwxgtk-webview3.0-0v5")

    # Fedora
    ELSEIF(EXISTS "/etc/redhat-release" OR LINUX_DISTRO MATCHES "Fedora")
        SET(CPACK_GENERATOR "RPM")
        SET(CPACK_RPM_PACKAGE_REQUIRES "wxGTK3 webkitgtk3")

    # openSUSE
    ELSEIF(EXISTS "/etc/redhat-release" OR LINUX_DISTRO MATCHES "openSUSE")
        SET(CPACK_GENERATOR "RPM")
        SET(CPACK_RPM_PACKAGE_REQUIRES "wxWidgets-3_0-devel")

    ELSEIF(NOT LINUX_DISTRO STREQUAL "Arch Linux")
        MESSAGE(WARNING "Unknown Linux distribution - generating generic package.")
    ENDIF()
ENDIF()

# macOS
IF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    SET(CPACK_GENERATOR "DragNDrop")
    SET(CPACK_DMG_VOLUME_NAME "Money Manager Ex")
    SET(CPACK_DMG_FORMAT "UDZO")
    SET(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_SOURCE_DIR}/resources/dmg-background.png")
    SET(CPACK_DMG_DS_STORE "${CMAKE_SOURCE_DIR}/resources/.DS_Store")
ENDIF()

# Windows
IF(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # With using http://nsis.sourceforge.net/Main_Page
    SET(CPACK_GENERATOR "NSIS;ZIP")
    SET(CPACK_PACKAGE_INSTALL_DIRECTORY "Money Manager EX")
    SET(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/resources\\\\mmex.ico")
    SET(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/resources\\\\mmex.ico")
    SET(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/resources\\\\mmex.ico")
    SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\${PROJECT_TARGET_BIN}.exe")
    SET(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}\\\\license.txt)
    # Probably bug
    # https://public.kitware.com/Bug/bug_relationship_graph.php?bug_id=7880&graph=relation
    SET(CPACK_RESOURCE_FILE_README ${CMAKE_SOURCE_DIR}/README.TXT)
    SET(CPACK_NSIS_CREATE_ICONS_EXTRA
            "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Money Manager EX.lnk' '$INSTDIR\\\\bin\\\\${PROJECT_TARGET_BIN}.exe'")
    SET(CPACK_NSIS_DELETE_ICONS_EXTRA "Delete '$SMPROGRAMS\\\\$START_MENU\\\\Money Manager EX.lnk'")
ENDIF()

IF(NOT LINUX_DISTRO STREQUAL "Arch Linux")
    INCLUDE(CPack)
ELSE()
    # Use dedicated tool to build Arch Linux package
    IF (TARGET_ARCH STREQUAL "x64")
        SET(ARCHLINUX_ARCH "x86_64")
    ELSE()
        SET(ARCHLINUX_ARCH "i686")
    ENDIF()
    CONFIGURE_FILE(util/PKGBUILD.in PKGBUILD @ONLY)
    SET(CMAKE_INSTALL_PREFIX /usr)

    # allow use reserved for cpack package target
    CMAKE_POLICY(PUSH)
    IF(POLICY CMP0037)
        CMAKE_POLICY(SET CMP0037 OLD)
    ENDIF()
    ADD_CUSTOM_TARGET(package
        COMMAND sh -c "if [ `id -u` -eq 0 ]; then chown nobody -R . ${EXECUTABLE_OUTPUT_PATH} ${LIBRARY_OUTPUT_PATH} ${PROJECT_SOURCE_DIR}/po && su nobody -c makepkg; else makepkg; fi"
        VERBATIM)
    CMAKE_POLICY(POP)
ENDIF()
