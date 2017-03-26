CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

SET(InMMEX TRUE)

PROJECT(MoneyManagerEx)

# Semantic versioning
SET(MoneyManagerEx_VERSION_MAJOR ${ARG_VER_MAJOR})
SET(MoneyManagerEx_VERSION_MINOR ${ARG_VER_MINOR})
SET(MoneyManagerEx_VERSION_PATCH ${ARG_VER_PATCH})
SET(MoneyManagerEx_VERSION "${MoneyManagerEx_VERSION_MAJOR}.${MoneyManagerEx_VERSION_MINOR}.${MoneyManagerEx_VERSION_PATCH}")

OPTION(BuildTests "Whether to build automatic unit tests" OFF)

MESSAGE(STATUS "Build Type = ${CMAKE_BUILD_TYPE}")
MESSAGE(STATUS "Building Tests = ${BuildTests}")
MESSAGE(STATUS "OS = ${CMAKE_SYSTEM_NAME}")

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    # Detect Linux distribution (if possible)
    EXECUTE_PROCESS(COMMAND sh getDistroName.sh
            TIMEOUT 4
            OUTPUT_VARIABLE LINUX_DISTRO
            #ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    MESSAGE(STATUS "Distro = ${LINUX_DISTRO}")
ENDIF ()


# --------- Set compiler parameters ---------
SET(CMAKE_CXX_STANDARD 11)

IF (MSVC)
    # using Visual Studio C++
    MESSAGE(STATUS "Compiler = MSVC")
    # Based on this http://stackoverflow.com/a/8294669
    # added solution to compile problems due to
    # conflict between winsock and winsock2
    # Partialy reinvented fix from commit
    # commit 06accae1273e66ced469672151522e45eee685a9
    ADD_DEFINITIONS(-DWIN32_LEAN_AND_MEAN)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)

ELSEIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # using GCC
    MESSAGE(STATUS "Compiler = GNU")
    #TODO:

ELSEIF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    # using regular Clang or AppleClang
    MESSAGE(STATUS "Compiler = Clang")

    IF (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND LINUX_DISTRO STREQUAL "Ubuntu")
        # Due to bug in Ubuntu
        # http://stackoverflow.com/questions/37096062/get-a-basic-c-program-to-compile-using-clang-on-ubuntu-16
        SET(CMAKE_CXX_FLAGS "-stdlib=libstdc++")
    ELSEIF (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        SET(CMAKE_CXX_FLAGS "-stdlib=libc++")
    ENDIF ()
ENDIF ()


# --------- Set various location ---------
SET(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")
SET(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/lib")

SET(INSTALL_BIN_DIR "bin")
SET(INSTALL_DOC_DIR "share/doc/mmex")
SET(INSTALL_RES_DIR "share/mmex/res")
SET(INSTALL_PO_DIR "share/mmex/po")
SET(INSTALL_MO_DIR "share/mmex/po/en")


# --------- Check for required libraries ---------
FIND_PACKAGE(Gettext REQUIRED)
#wxScintilla is needed on Windows
IF (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    FIND_PACKAGE(wxWidgets REQUIRED core qa net html xml aui adv scintilla stc webview base REQUIRED)
ELSE ()
    FIND_PACKAGE(wxWidgets REQUIRED core qa net html xml aui adv stc webview base REQUIRED)
ENDIF ()
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
ADD_EXECUTABLE(mmex WIN32 "")
IF (BuildTests)
    ADD_LIBRARY(CPPUnit STATIC "")
    ADD_EXECUTABLE(MMEXTest "")
ENDIF ()

## Subdirectories ##
ADD_SUBDIRECTORY(po)
ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(src)
IF (BuildTests)
    ADD_SUBDIRECTORY(tests)
ENDIF ()


## Build and Link ##
IF (NOT WIN32)
    TARGET_LINK_LIBRARIES(wxSQLite3 dl)
ENDIF ()
TARGET_LINK_LIBRARIES(mmex ${wxWidgets_LIBRARIES} wxSQLite3 LUA)
IF (BuildTests)
    TARGET_LINK_LIBRARIES(MMEXTest CPPUnit ${wxWidgets_LIBRARIES} wxSQLite3 LUA)
ENDIF ()


# --------- Install ---------
# OSX
IF (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    #TODO:
ENDIF ()
# Linux and Windows
IF (CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Windows")

    # Binary
    IF (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        INSTALL(PROGRAMS
                "${CMAKE_SOURCE_DIR}/bin/mmex"
                DESTINATION "${INSTALL_BIN_DIR}")
    ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        INSTALL(PROGRAMS
                "${CMAKE_SOURCE_DIR}/bin/mmex.exe"
                DESTINATION "${INSTALL_BIN_DIR}")
    ENDIF()

    # Help Files
    INSTALL(FILES
            "${CMAKE_SOURCE_DIR}/doc/contrib.txt"
            "${CMAKE_SOURCE_DIR}/doc/license.txt"
            "${CMAKE_SOURCE_DIR}/doc/version.txt"
            "${CMAKE_SOURCE_DIR}/README.TXT"
            DESTINATION "${INSTALL_DOC_DIR}")
    INSTALL(DIRECTORY
            "${CMAKE_SOURCE_DIR}/doc/help"
            DESTINATION "${INSTALL_DOC_DIR}")
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

    IF (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        # .desktop File
        INSTALL(FILES
                "${CMAKE_SOURCE_DIR}/resources/mmex.desktop"
                DESTINATION "share/applications")
    ENDIF()

    # Icons
    IF (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        INSTALL(FILES
                "${CMAKE_SOURCE_DIR}/resources/mmex.svg"
                DESTINATION "share/icons/hicolor/scalable/apps")
    ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        INSTALL(FILES
                "${CMAKE_SOURCE_DIR}/resources/mmex.ico"
                DESTINATION "${INSTALL_RES_DIR}")
    ENDIF()
ENDIF ()


# --------- CPack ---------
SET(CPACK_PACKAGE_NAME "mmex")
# CPACK_PACKAGE_VERSION "1.2.3") # to produce something like this
set(CPACK_PACKAGE_VERSION "${MoneyManagerEx_VERSION_MAJOR}.${MoneyManagerEx_VERSION_MINOR}.${MoneyManagerEx_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION_MAJOR "${MoneyManagerEx_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${MoneyManagerEx_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${MoneyManagerEx_VERSION_PATCH}")
SET(CPACK_PACKAGE_CONTACT "MoneyManagerEx Developers <developers@moneymanagerex.org>")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Simple to use financial management software")
SET(CPACK_PACKAGE_DESCRIPTION "Money Manager Ex (MMEX) is a free, open-source, cross-platform, easy-to-use personal finance software. It primarily helps organize one's finances and keeps track of where, when and how the money goes. MMEX includes all the basic features that 90% of users would want to see in a personal finance application. The design goals are to concentrate on simplicity and user friendliness - something one can use everyday.")

# Linux
# Set package dependencies
IF (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    IF (EXISTS "/etc/debian_version")
        SET(CPACK_GENERATOR "DEB")
        # Ubuntu
        SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.15), libgcc1 (>=1:4.1.1), libstdc++6 (>= 4.6), libwxgtk3.0-0v5 (>= 3.0.0) | libwxgtk3.0-0 (>=3.0.0), libwxgtk-webview3.0-0v5 (>= 3.0.0) | libwxgtk-webview3.0-0 (>= 3.0.0)")
    ELSEIF (EXISTS "/etc/redhat-release" OR LINUX_DISTRO MATCHES "openSUSE")
        SET(CPACK_GENERATOR "RPM")
        SET(CPACK_RPM_PACKAGE_LICENSE "GPL-2+")
        # Fedora
        IF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" AND LINUX_DISTRO MATCHES "Fedora")
            SET(CPACK_RPM_PACKAGE_REQUIRES "clang, wxGTK3-devel")
        ELSE ()
            SET(CPACK_RPM_PACKAGE_REQUIRES "gcc-c++, wxGTK3-devel")
        ENDIF ()
        # openSUSE
        IF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" AND LINUX_DISTRO MATCHES "openSUSE")
            SET(CPACK_RPM_PACKAGE_REQUIRES "llvm-clang, wxWidgets-3_0-devel")
        ELSE ()
            SET(CPACK_RPM_PACKAGE_REQUIRES "gcc-c++, wxWidgets-3_0-devel")
        ENDIF ()
    ELSE ()
        #TODO: tar.gz
    ENDIF ()
ENDIF ()

# macOS
IF (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    SET(CPACK_GENERATOR "DragNDrop")
    SET(CPACK_DMG_VOLUME_NAME "MMEX")
    SET(CPACK_DMG_FORMAT "UDZO")
    #TODO:
ENDIF ()

# Windows
IF (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # With using http://nsis.sourceforge.net/Main_Page
    SET(CPACK_GENERATOR "NSIS")
    SET(CPACK_PACKAGE_INSTALL_DIRECTORY "Money Manager EX")
    SET(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/resources\\\\mmex.ico")
    SET(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/resources\\\\mmex.ico")
    SET(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/resources\\\\mmex.ico")
    SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\mmex.exe")
    SET(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE)
    SET(CPACK_NSIS_CREATE_ICONS_EXTRA
            "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Money Manager EX.lnk' '$INSTDIR\\\\bin\\\\mmex.exe'")
    SET(CPACK_NSIS_DELETE_ICONS_EXTRA "Delete '$SMPROGRAMS\\\\$START_MENU\\\\Money Manager EX.lnk'")
ENDIF ()

INCLUDE(CPack)
