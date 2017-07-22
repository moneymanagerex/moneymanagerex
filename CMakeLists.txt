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
# ./appveyor.yml
# ./resources/mmex.rc
# ---------------------------------------------------------------------------
SET(MoneyManagerEx_VERSION_MAJOR 1)
SET(MoneyManagerEx_VERSION_MINOR 4)
SET(MoneyManagerEx_VERSION_PATCH 0)
SET(MoneyManagerEx_VERSION_ALPHA 1)
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
  TIMEOUT 4
  ERROR_QUIET
)

# Get the latest abbreviated commit hash of the working branch
EXECUTE_PROCESS(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
  TIMEOUT 4
  ERROR_QUIET
)

# Get the latest commit date of the working branch
EXECUTE_PROCESS(
  COMMAND git log -1 --format=%cd --date=short
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_DATE
  OUTPUT_STRIP_TRAILING_WHITESPACE
  TIMEOUT 4
  ERROR_QUIET
)

IF(GIT_COMMIT_HASH)
  ADD_DEFINITIONS(-DGIT_COMMIT_DATE="${GIT_COMMIT_DATE}" -DGIT_COMMIT_HASH="${GIT_COMMIT_HASH}")
ENDIF()
IF(GIT_BRANCH)
  ADD_DEFINITIONS(-DGIT_BRANCH="${GIT_BRANCH}")
ENDIF()
ADD_DEFINITIONS(-DCMAKE_VERSION="${CMAKE_VERSION}" -DMAKE_VERSION="$(MAKE_VERSION)")

# Passing CMake variables (specificaly current version numbers) to files
# AppVeyor
CONFIGURE_FILE(
        "${PROJECT_SOURCE_DIR}/appveyor.yml.in"
        "${PROJECT_SOURCE_DIR}/appveyor.yml"
)
# Header file
CONFIGURE_FILE(
        "${PROJECT_SOURCE_DIR}/src/constants.h.in"
        "${PROJECT_SOURCE_DIR}/src/constants.h"
)
# VS resource script
CONFIGURE_FILE(
        "${PROJECT_SOURCE_DIR}/resources/mmex.rc.in"
        "${PROJECT_SOURCE_DIR}/resources/mmex.rc"
)

OPTION(BuildTests "Whether to build automatic unit tests" OFF)

MESSAGE(STATUS "Build Type = ${CMAKE_BUILD_TYPE}")
MESSAGE(STATUS "Building Tests = ${BuildTests}")
MESSAGE(STATUS "OS = ${CMAKE_SYSTEM_NAME}")

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
    SET(INSTALL_PO_DIR "${PROJECT_TARGET_BIN}.app/Contents/SharedSupport/po")
    SET(INSTALL_MO_DIR "${PROJECT_TARGET_BIN}.app/Contents/SharedSupport/po/en")
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    SET(INSTALL_DOC_DIR ".")
    SET(INSTALL_RES_DIR "res")
    SET(INSTALL_PO_DIR "po")
    SET(INSTALL_MO_DIR "po/en")
ELSE()
    SET(INSTALL_DOC_DIR "share/doc/mmex")
    SET(INSTALL_RES_DIR "share/mmex/res")
    SET(INSTALL_PO_DIR "share/mmex/po")
    SET(INSTALL_MO_DIR "share/mmex/po/en")
ENDIF()


# --------- Check for required libraries ---------
FIND_PACKAGE(Gettext REQUIRED)
#wxScintilla is needed on Windows
IF(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    FIND_PACKAGE(wxWidgets REQUIRED core qa net html xml aui adv scintilla stc webview base REQUIRED)
ELSE()
    FIND_PACKAGE(wxWidgets REQUIRED core qa net html xml aui adv stc webview base REQUIRED)
ENDIF()

INCLUDE(${wxWidgets_USE_FILE})
SET(wxWidgets_USE_DEBUG false)

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
ADD_LIBRARY(MMEXLib STATIC "")
ADD_EXECUTABLE(${PROJECT_TARGET_BIN} WIN32 MACOSX_BUNDLE ${MACOSX_APP_ICON_FILE} "")
IF(BuildTests)
    INCLUDE(ExternalProject)
    ExternalProject_ADD(CPPUnit
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/lib/cppunit
        CONFIGURE_COMMAND sh autogen.sh && ./configure -q --disable-shared --disable-doxygen --enable-silent-rules --prefix=${CMAKE_CURRENT_BINARY_DIR}/cppunit
        INSTALL_COMMAND make install-exec
        BUILD_IN_SOURCE 1
        )
    ADD_LIBRARY(CPPUnitLib STATIC IMPORTED)
    SET_TARGET_PROPERTIES(CPPUnitLib PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/cppunit/lib/libcppunit.a
        )
    ADD_DEPENDENCIES(CPPUnitLib CPPUnit)
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
TARGET_LINK_LIBRARIES(${PROJECT_TARGET_BIN} MMEXLib ${wxWidgets_LIBRARIES} wxSQLite3 LUA)
IF(BuildTests)
    TARGET_LINK_LIBRARIES(MMEXTest MMEXLib CPPUnitLib ${wxWidgets_LIBRARIES} wxSQLite3 LUA)
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

# Finally set ups for multiplatform INSTALL
# Binary
IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    INSTALL(PROGRAMS
            "${CMAKE_SOURCE_DIR}/bin/${PROJECT_TARGET_BIN}"
            DESTINATION "${INSTALL_BIN_DIR}")
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    INSTALL(TARGETS
            "${PROJECT_TARGET_BIN}"
            BUNDLE DESTINATION .)
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    INSTALL(PROGRAMS
            "${CMAKE_SOURCE_DIR}/bin/${PROJECT_TARGET_BIN}.exe"
            DESTINATION "${INSTALL_BIN_DIR}")

    # Besides normal installable version, for windows the portable version needs additionally files
    FILE(WRITE "${CMAKE_CURRENT_BINARY_DIR}/mmexini.db3" "")
    INSTALL(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/mmexini.db3"
            DESTINATION "${INSTALL_DOC_DIR}")

    SET(MSVCP "C:\\\\Windows\\\\System32\\\\msvcp140.dll")
    SET(VCRUNTIME "C:\\\\Windows\\\\System32\\\\vcruntime140.dll")

    IF(EXISTS ${MSVCP})
        INSTALL(FILES
                "${MSVCP}"
                DESTINATION "${INSTALL_BIN_DIR}")
    ELSE()
        MESSAGE(WARNING "Can not copy ${MSVCP}. File does not exist")
    ENDIF()

    IF(EXISTS ${VCRUNTIME})
        INSTALL(FILES
                ${VCRUNTIME}
                DESTINATION "${INSTALL_BIN_DIR}")
    ELSE()
        MESSAGE(WARNING "Can not copy ${VCRUNTIME}. File does not exist")
    ENDIF()
ENDIF()

# Help Files
INSTALL(FILES
        "${CMAKE_SOURCE_DIR}/doc/contrib.txt"
        "${CMAKE_SOURCE_DIR}/doc/license.txt"
        "${CMAKE_SOURCE_DIR}/doc/version.txt"
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
        SET(CPACK_RPM_PACKAGE_LICENSE "GPL-2+")
		SET(CPACK_RPM_PACKAGE_REQUIRES "wxGTK3 webkitgtk3")

    # openSUSE
    ELSEIF(EXISTS "/etc/redhat-release" OR LINUX_DISTRO MATCHES "openSUSE")
        SET(CPACK_GENERATOR "RPM")
        SET(CPACK_RPM_PACKAGE_LICENSE "GPL-2+")
		SET(CPACK_RPM_PACKAGE_REQUIRES "wxWidgets-3_0-devel")

    ELSE()
        #TODO: tar.gz
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
    SET(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/doc\\\\license.txt)
    # Probably bug
    # https://public.kitware.com/Bug/bug_relationship_graph.php?bug_id=7880&graph=relation
    SET(CPACK_RESOURCE_FILE_README ${CMAKE_SOURCE_DIR}/README.TXT)
    SET(CPACK_NSIS_CREATE_ICONS_EXTRA
            "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Money Manager EX.lnk' '$INSTDIR\\\\bin\\\\${PROJECT_TARGET_BIN}.exe'")
    SET(CPACK_NSIS_DELETE_ICONS_EXTRA "Delete '$SMPROGRAMS\\\\$START_MENU\\\\Money Manager EX.lnk'")
ENDIF()

INCLUDE(CPack)
