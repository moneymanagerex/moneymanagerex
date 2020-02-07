CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

SET(InMMEX TRUE)

PROJECT(MoneyManagerEx)
# Semantic versioning
SET(MoneyManagerEx_VERSION_MAJOR 1)
SET(MoneyManagerEx_VERSION_MINOR 3)
SET(MoneyManagerEx_VERSION_PATCH 4)

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
SET(INSTALL_MO_DIR "share/mmex/po")


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

# --------- curl dependency ---------
# curl version >=7.57 can have config files
find_package(CURL QUIET COMPONENTS libcurl CONFIG)
if(NOT CURL_FOUND AND NOT CURL_VERSION VERSION_LESS 7.62)
    # curl 7.62 removed support for COMPONENTS
    find_package(CURL QUIET CONFIG)
endif()
if(CURL_FOUND)
    message(STATUS "Found CURL: ${CURL_LIBRARIES} (found version \"${CURL_VERSION}\")")
    get_target_property(LIBCURL_TYPE CURL::libcurl TYPE)
    # CURL_STATICLIB is missing for curl versions <7.61.1
    if(CURL_VERSION VERSION_LESS 7.61.1 AND LIBCURL_TYPE STREQUAL STATIC_LIBRARY)
        set_target_properties(CURL::libcurl PROPERTIES
            INTERFACE_COMPILE_DEFINITIONS CURL_STATICLIB)
    endif()
else()
    set(CMAKE_FIND_LIBRARY_PREFIXES ${CMAKE_FIND_LIBRARY_PREFIXES} lib)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES} _imp.lib -d.lib -d_imp.lib)
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)
        PKG_SEARCH_MODULE(CURL libcurl)
        if(CURL_FOUND)
            # CMake <3.12 workaround
            if(NOT CURL_LINK_LIBRARIES)
                find_library(CURL_LINK_LIBRARIES NAMES ${CURL_LIBRARIES}
                    HINTS "${CURL_LIBDIR}" NO_DEFAULT_PATH)
                if(NOT CURL_LINK_LIBRARIES)
                    message(WARNING "CURL library file cannot be found!")
                endif()
            endif()
            if(NOT CURL_LINK_LIBRARIES MATCHES "_imp.lib$|${CMAKE_SHARED_LIBRARY_SUFFIX}$")
                list(REMOVE_ITEM CURL_STATIC_LIBRARIES ${CURL_LIBRARIES})
                add_library(CURL::libcurl STATIC IMPORTED)
                set_target_properties(CURL::libcurl PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES "${CURL_STATIC_INCLUDE_DIRS}"
                    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${CURL_STATIC_INCLUDE_DIRS}"
                    INTERFACE_COMPILE_DEFINITIONS CURL_STATICLIB
                    INTERFACE_LINK_LIBRARIES "${CURL_STATIC_LIBRARIES}"
                    IMPORTED_LINK_INTERFACE_LANGUAGES C
                    IMPORTED_LOCATION "${CURL_LINK_LIBRARIES}")
                link_directories(CURL_STATIC_LIBRARY_DIRS)
            else()
                add_library(CURL::libcurl SHARED IMPORTED)
                set_target_properties(CURL::libcurl PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIRS}"
                    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIRS}")
                if(WIN32)
                    set_target_properties(CURL::libcurl PROPERTIES
                        IMPORTED_IMPLIB "${CURL_LINK_LIBRARIES}")
                else()
                    set_target_properties(CURL::libcurl PROPERTIES
                        IMPORTED_LOCATION "${CURL_LINK_LIBRARIES}")
                endif()
            endif()
            message(STATUS "Found CURL: ${CURL_LINK_LIBRARIES} (found version \"${CURL_VERSION}\")")
        endif()
    endif()
    if(NOT CURL_FOUND)
        find_package(CURL REQUIRED COMPONENTS libcurl MODULE)
        if(NOT CURL_LIBRARIES MATCHES "_imp.lib$|${CMAKE_SHARED_LIBRARY_SUFFIX}$")
            add_library(CURL::libcurl STATIC IMPORTED)
            set_target_properties(CURL::libcurl PROPERTIES
                INTERFACE_COMPILE_DEFINITIONS CURL_STATICLIB
                IMPORTED_LINK_INTERFACE_LANGUAGES C
                IMPORTED_LOCATION "${CURL_LIBRARIES}")
        else()
            add_library(CURL::libcurl SHARED IMPORTED)
            if(WIN32)
                set_target_properties(CURL::libcurl PROPERTIES
                    IMPORTED_IMPLIB "${CURL_LIBRARIES}")
            else()
                set_target_properties(CURL::libcurl PROPERTIES
                    IMPORTED_LOCATION "${CURL_LIBRARIES}")
            endif()
        endif()
        set_target_properties(CURL::libcurl PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIRS}"
            INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIRS}")
        set(CURL_VERSION ${CURL_VERSION_STRING})
    endif()
endif()

if(CURL_VERSION)
    message(STATUS "curl version ${CURL_VERSION}")
    message(STATUS "curl libraries ${CURL_LIBRARIES}")
    message(STATUS "curl include dir ${CURL_INCLUDE_DIRS}")
    message(STATUS "Preprocessor macro ${INTERFACE_COMPILE_DEFINITIONS}")
endif()

# --------- include Directories ---------
INCLUDE_DIRECTORIES(
        ${wxWidgets_INCLUDE_DIRS}
        ${CURL_INCLUDE_DIRS}
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
# Linux
IF (CMAKE_SYSTEM_NAME STREQUAL "Linux")

    # Binary
    INSTALL(PROGRAMS "${CMAKE_SOURCE_DIR}/bin/mmex"
            DESTINATION bin)

    # Help Files
    INSTALL(FILES "${CMAKE_SOURCE_DIR}/doc/contrib.txt"
            "${CMAKE_SOURCE_DIR}/doc/license.txt"
            "${CMAKE_SOURCE_DIR}/doc/version.txt"
            DESTINATION "${INSTALL_DOC_DIR}")
    INSTALL(DIRECTORY "${CMAKE_SOURCE_DIR}/doc/help"
            DESTINATION "${INSTALL_DOC_DIR}")
    INSTALL(FILES "${CMAKE_SOURCE_DIR}/resources/master.css"
            DESTINATION "${INSTALL_DOC_DIR}/help")

    # Resources
    INSTALL(FILES "${CMAKE_SOURCE_DIR}/3rd/ChartNew.js/ChartNew.js"
            "${CMAKE_SOURCE_DIR}/3rd/sorttable.js/sorttable.js"
            "${CMAKE_SOURCE_DIR}/resources/home_page.htt"
            "${CMAKE_SOURCE_DIR}/resources/kaching.wav"
            "${CMAKE_SOURCE_DIR}/resources/master.css"
            "${CMAKE_SOURCE_DIR}/resources/mmex.ico"
            DESTINATION "${INSTALL_RES_DIR}/")

    # .desktop File
    INSTALL(FILES "${CMAKE_SOURCE_DIR}/resources/mmex.desktop"
            DESTINATION "share/applications")

    # Icons
    INSTALL(FILES "${CMAKE_SOURCE_DIR}/resources/mmex.svg"
            DESTINATION "share/icons/hicolor/scalable/apps")

ENDIF ()
# Windows
IF (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    #TODO:
ENDIF ()


# --------- CPack ---------
SET(CPACK_PACKAGE_NAME "mmex")
SET(CPACK_PACKAGE_VERSION "1.3.4")
SET(CPACK_PACKAGE_CONTACT "MoneyManagerEx Developers <developers@moneymanagerex.org>")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Simple to use financial management software")
SET(CPACK_PACKAGE_DESCRIPTION "Money Manager Ex (MMEX) is a free, open-source, cross-platform, easy-to-use personal finance software. It primarily helps organize one's finances and keeps track of where, when and how the money goes. MMEX includes all the basic features that 90% of users would want to see in a personal finance application. The design goals are to concentrate on simplicity and user friendliness - something one can use everyday.")

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

IF (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    SET(CPACK_GENERATOR "DMG")
    SET(CPACK_DMG_VOLUME_NAME "MMEX")
    SET(CPACK_DMG_FORMAT "UDZO")
    #TODO:
ENDIF ()

IF (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    #TODO:
ENDIF ()

INCLUDE(CPack)
