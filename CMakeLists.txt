cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

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

# By setting the application version here, CMake will transfer the variable
# values to the corresponding variable names in the following files:
# ./src/build.h
# ./resources/mmex.rc
# ---------------------------------------------------------------------------

set(MMEX_VERSION 1.6.0)
SET(MMEX_VERSION_ALPHA -1)
SET(MMEX_VERSION_BETA 1)
SET(MMEX_VERSION_RC -1)

# ---------------------------------------------------------------------------

if(DEFINED ENV{VCPKG_ROOT})
    if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "")
    elseif(NOT CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg.cmake$")
        set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_TOOLCHAIN_FILE}")
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "")
    endif()
endif()

if(DEFINED ENV{VCPKG_DEFAULT_TRIPLET} AND NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "")
endif()

# Adjusting semantic versioning for alpha/beta/rc version
IF(MMEX_VERSION_ALPHA EQUAL 0)
    SET(MMEX_VERSION_UNSTABLE Alpha)
ELSEIF(MMEX_VERSION_ALPHA GREATER 0)
    SET(MMEX_VERSION_UNSTABLE Alpha.${MMEX_VERSION_ALPHA})
ELSEIF(MMEX_VERSION_BETA EQUAL 0)
    SET(MMEX_VERSION_UNSTABLE Beta)
ELSEIF(MMEX_VERSION_BETA GREATER 0)
    SET(MMEX_VERSION_UNSTABLE Beta.${MMEX_VERSION_BETA})
ELSEIF(MMEX_VERSION_RC EQUAL 0)
    SET(MMEX_VERSION_UNSTABLE RC)
ELSEIF(MMEX_VERSION_RC GREATER 0)
    SET(MMEX_VERSION_UNSTABLE RC.${MMEX_VERSION_RC})
ENDIF()
SET(MMEX_VERSION_WITH_UNSTABLE ${MMEX_VERSION})
IF(MMEX_VERSION_UNSTABLE)
    SET(MMEX_VERSION_WITH_UNSTABLE "${MMEX_VERSION}-${MMEX_VERSION_UNSTABLE}")
ENDIF()

set(MMEX_VERSION_FULL ${MMEX_VERSION_WITH_UNSTABLE})

project(MMEX VERSION ${MMEX_VERSION})
option(MMEX_PORTABLE_INSTALL "Include an empty mmexini.db3 file in the Windows installation" OFF)
option(MMEX_ENCRYPTION_OPTIONAL "Build even if encryption is not supported by wxsqlite library" OFF)

# Name of the resulted executable binary
set(MMEX_EXE mmex)

# build date/time and version save to file
FILE (WRITE ${PROJECT_SOURCE_DIR}/src/build.h "#define BUILD_DATE __DATE__\n"
"#define BUILD_TIME __TIME__\n"
"#define MMEX_VERSION_WITH_UNSTABLE \"${MMEX_VERSION_WITH_UNSTABLE}\"\n"
)

# Find Git commit details to put into versions.h later
# Get the current working branch
find_package(Git REQUIRED)
execute_process(COMMAND ${GIT_EXECUTABLE} symbolic-ref --short -q HEAD
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    TIMEOUT 4 ERROR_QUIET)
# Get the latest abbreviated commit hash of the working branch
execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --format=%h
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    TIMEOUT 4 ERROR_QUIET)
# Get the latest commit date of the working branch
execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --format=%cd --date=short
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_DATE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    TIMEOUT 4 ERROR_QUIET)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # define shortcut
    set(LINUX TRUE)

    # Detect Linux distribution (if possible)
    find_program(LSB_RELEASE lsb_release
        DOC "tool to read linux distribution name")
    if(LSB_RELEASE)
        execute_process(COMMAND lsb_release -is
            OUTPUT_VARIABLE LINUX_DISTRO
            OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
        execute_process(COMMAND lsb_release -cs
            OUTPUT_VARIABLE LINUX_DISTRO_CODENAME
            OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
        execute_process(COMMAND lsb_release -rs
            OUTPUT_VARIABLE LINUX_DISTRO_REL
            OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
        execute_process(COMMAND lsb_release -ds
            OUTPUT_VARIABLE LINUX_DISTRO_DESCRIPTION
            OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
        string(REPLACE "\"" "" LINUX_DISTRO_DESCRIPTION "${LINUX_DISTRO_DESCRIPTION}")
        string(REPLACE " " "." LINUX_DISTRO_DESCRIPTION "${LINUX_DISTRO_DESCRIPTION}")
    else()
        if(EXISTS /etc/slackware-version)
            file(STRINGS /etc/slackware-version LINUX_DISTRO LIMIT_COUNT 1)
            if(LINUX_DISTRO MATCHES "^([^.]+) +([0-9.]+)")
                set(LINUX_DISTRO "${CMAKE_MATCH_1}")
                set(LINUX_DISTRO_REL ${CMAKE_MATCH_2})
            endif()
        elseif(EXISTS /etc/arch-release)
            execute_process(COMMAND  uname -r
                OUTPUT_VARIABLE LINUX_DISTRO_REL
                OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
            set(LINUX_DISTRO "Arch")
        else()
            message(SEND_ERROR "lsb_release tool not found.")
        endif()
    endif()
    if(NOT LINUX_DISTRO OR NOT LINUX_DISTRO_REL)
        message(SEND_ERROR "Could not find a GNU/Linux distribution info")
    endif()
    set(LINUX_DISTRO_STRING "${LINUX_DISTRO}.${LINUX_DISTRO_REL}")
    if(LINUX_DISTRO_CODENAME STREQUAL "n/a")
        if(LINUX_DISTRO STREQUAL "Debian")
            execute_process(COMMAND lsb_release -ds
                OUTPUT_VARIABLE LINUX_DISTRO_CODENAME
                OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
            if(LINUX_DISTRO_CODENAME MATCHES " ([a-z]+)/sid$")
                set(LINUX_DISTRO_CODENAME ${CMAKE_MATCH_1})
            else()
                unset(LINUX_DISTRO_CODENAME)
            endif()
        else()
            unset(LINUX_DISTRO_CODENAME)
        endif()
    endif()
    if(LINUX_DISTRO_CODENAME)
        set(LINUX_DISTRO_STRING "${LINUX_DISTRO_STRING}.${LINUX_DISTRO_CODENAME}")
    endif()
endif()

get_filename_component(CMAKE_MAKE_NAME "${CMAKE_MAKE_PROGRAM}" NAME_WE)
if(CMAKE_MAKE_NAME STREQUAL "MSBuild")
    execute_process(COMMAND "${CMAKE_MAKE_PROGRAM}" /nologo /version
        OUTPUT_VARIABLE CMAKE_MAKE_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    set(CMAKE_MAKE_VERSION "MSBuild ${CMAKE_MAKE_VERSION}")
elseif(CMAKE_MAKE_NAME STREQUAL "ninja")
    execute_process(COMMAND "${CMAKE_MAKE_PROGRAM}" --version
        OUTPUT_VARIABLE CMAKE_MAKE_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    set(CMAKE_MAKE_VERSION "Ninja ${CMAKE_MAKE_VERSION}")
elseif(CMAKE_MAKE_NAME STREQUAL "xcodebuild")
    execute_process(COMMAND "${CMAKE_MAKE_PROGRAM}" -version
        OUTPUT_VARIABLE CMAKE_MAKE_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    string(REGEX REPLACE "\n.*" "" CMAKE_MAKE_VERSION "${CMAKE_MAKE_VERSION}")
else()
    execute_process(COMMAND "${CMAKE_MAKE_PROGRAM}" --version
        OUTPUT_VARIABLE CMAKE_MAKE_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    string(REGEX REPLACE "\n.*" "" CMAKE_MAKE_VERSION "${CMAKE_MAKE_VERSION}")
endif()
if(DEFINED ENV{VisualStudioVersion})
    set(VS_VERSION $ENV{VisualStudioVersion})
endif()


# --------- Set compiler parameters ---------
if(MSVC)
    # using Visual Studio C++
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    # use multicore
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL AppleClang)
    # force libc++, not libstdc++ for Xcode 10
    # https://developer.apple.com/documentation/xcode_release_notes/xcode_10_release_notes#3035631
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()


# try to use clcache/ccache to speed up builds
if(NOT CCACHE_PROGRAM)
    if(MSVC)
        find_program(CCACHE_PROGRAM clcache)
        set(_PARAM --help)
    else()
        find_program(CCACHE_PROGRAM ccache)
        set(_PARAM --version)
    endif()
    if(CCACHE_PROGRAM)
        execute_process(COMMAND "${CCACHE_PROGRAM}" ${_PARAM}
            OUTPUT_VARIABLE CCACHE_VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
        string(REGEX REPLACE "\n.*" "" CCACHE_VERSION "${CCACHE_VERSION}")
        string(REGEX REPLACE ".* " "" CCACHE_VERSION "${CCACHE_VERSION}")
        string(REGEX REPLACE "^v" "" CCACHE_VERSION "${CCACHE_VERSION}")
        if(CCACHE_VERSION)
            message(STATUS "Found compile cache tool: ${CCACHE_PROGRAM} (found version \"${CCACHE_VERSION}\")")
        else()
            message(STATUS "Found compile cache tool: ${CCACHE_PROGRAM}")
        endif()
    else()
        message(STATUS "Could NOT find compile cache tool (ccache or clcache)")
    endif()
    unset(_PARAM)
endif()
if(CCACHE_PROGRAM)
    if(MSVC)
        get_filename_component(CCACHE_EXEC "${CCACHE_PROGRAM}" NAME CACHE)
        get_filename_component(CCACHE_PATH "${CCACHE_PROGRAM}" DIRECTORY CACHE)
        file(TO_NATIVE_PATH "${CCACHE_PATH}" CCACHE_PATH)
        if(NOT CMAKE_VERSION VERSION_LESS 3.13)
            list(APPEND CMAKE_VS_GLOBALS
                "CLToolExe=${CCACHE_EXEC}"
                "CLToolPath=${CCACHE_PATH}"
                "TrackFileAccess=false")
        elseif(NOT DEFINED ENV{VCPKG_ROOT})
            function(any_target_hook)
                set(NON_COMPILE_TARGETS INTERFACE IMPORTED UNKNOWN ALIAS)
                list(FIND NON_COMPILE_TARGETS "${ARGV1}" found)
                if(${found} GREATER -1)
                    return()
                endif()

                set_target_properties(${ARGV0} PROPERTIES VS_GLOBAL_CLToolExe       "${CCACHE_EXEC}")
                set_target_properties(${ARGV0} PROPERTIES VS_GLOBAL_CLToolPath      "${CCACHE_PATH}")
                set_target_properties(${ARGV0} PROPERTIES VS_GLOBAL_TrackFileAccess false)
            endfunction()

            function(add_library)
                _add_library(${ARGN})
                any_target_hook(${ARGN})
            endfunction()

            function(add_executable)
                _add_executable(${ARGN})
                any_target_hook(${ARGN})
            endfunction()
        endif()
    else()
        if(NOT CMAKE_VERSION VERSION_LESS 3.4)
            set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
            set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
        else()
            set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
            set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_PROGRAM}")
        endif()
    endif()
endif()

# --------- cmake --grapgviz settings ---------
file(WRITE "${CMAKE_BINARY_DIR}/CMakeGraphVizOptions.cmake" "
    set(GRAPHVIZ_GRAPH_NAME \"MMEX build dependency graph\")
    set(GRAPHVIZ_GENERATE_PER_TARGET FALSE)
    set(GRAPHVIZ_GENERATE_DEPENDERS FALSE)
")

# --------- Set various location ---------
if(APPLE)
    # Preparing for configure icons
    set(MACOSX_APP_ICON_NAME mmex.icns)
    # Setting path to icon
    set(MACOSX_APP_ICON_FILE "${PROJECT_SOURCE_DIR}/resources/${MACOSX_APP_ICON_NAME}")
    set(MMEX_DOC_DIR ${MMEX_EXE}.app/Contents/SharedSupport/doc)
    set(MMEX_RES_DIR ${MMEX_EXE}.app/Contents/Resources)
elseif(WIN32)
    set(MMEX_DOC_DIR .)
    set(MMEX_RES_DIR res)
else()
    set(MMEX_DOC_DIR share/doc/mmex)
    set(MMEX_RES_DIR share/mmex/res)
endif()
set(MMEX_RES_DIR_THEMES "${MMEX_RES_DIR}/themes")
set(MMEX_RES_DIR_REPORTS "${MMEX_RES_DIR}/reports")


# --------- wxWidgets dependency ---------
if(CMAKE_VERSION VERSION_LESS 3.25)
    # FindwxWidgets.cmake module will be fixed in CMake 3.25 to support later versions of wxWidgets
    list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/util")
endif()
set(wxWidgets_USE_REL_AND_DBG ON)
find_package(wxWidgets 2.9.2 REQUIRED
    COMPONENTS core qa html xml aui adv stc webview base
    OPTIONAL_COMPONENTS scintilla)
if(APPLE AND wxWidgets_LIBRARIES MATCHES "${CMAKE_STATIC_LIBRARY_PREFIX}wx_baseu-[0-9\\.]+${CMAKE_STATIC_LIBRARY_SUFFIX}")
    # use static libs in place of dynamic if using static wxMac
    foreach(deplib png jpeg)
        find_library(${deplib}path ${CMAKE_STATIC_LIBRARY_PREFIX}${deplib}${CMAKE_STATIC_LIBRARY_SUFFIX})
        if(${deplib}path)
            string(REPLACE "-l${deplib}" "${${deplib}path}"
                wxWidgets_LIBRARIES "${wxWidgets_LIBRARIES}")
        endif()
    endforeach()
    unset(deplib)
    unset(${deplib}path)
endif()
add_library(wxWidgets INTERFACE)
target_include_directories(wxWidgets SYSTEM INTERFACE ${wxWidgets_INCLUDE_DIRS})
target_link_libraries(wxWidgets INTERFACE ${wxWidgets_LIBRARIES})
target_compile_options(wxWidgets INTERFACE ${wxWidgets_CXX_FLAGS})
target_compile_definitions(wxWidgets INTERFACE ${wxWidgets_DEFINITIONS}
    wxNO_UNSAFE_WXSTRING_CONV=1
    $<$<CONFIG:Debug>:${wxWidgets_DEFINITIONS_DEBUG}>)

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
            #add_library(CURL::libcurl STATIC IMPORTED)
            set_target_properties(CURL::libcurl PROPERTIES
                INTERFACE_COMPILE_DEFINITIONS CURL_STATICLIB
                IMPORTED_LINK_INTERFACE_LANGUAGES C
                IMPORTED_LOCATION "${CURL_LIBRARIES}")
        else()
            if(WIN32)
                set_target_properties(CURL::libcurl PROPERTIES
                    IMPORTED_IMPLIB "${CURL_LIBRARIES}")
            else()
                add_library(CURL::libcurl SHARED IMPORTED)
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


add_subdirectory(3rd/fmt)

# --------- Collect sources and targets ---------
set(MMEX_BIN_DIR bin)

## Tuning for VisualStudio IDE ##
if(MSVC)
    message(STATUS "Tuning for Visual Studio IDE")

    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/install"
            CACHE PATH "Install directory used by INSTALL target" FORCE)
    endif()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_INSTALL_PREFIX}/${MMEX_BIN_DIR}>)
    if(NOT CMAKE_VERSION VERSION_LESS 3.3)
        set(CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD ON)
    endif()
endif()


## Subdirectories with targets ##
add_subdirectory(3rd)
add_subdirectory(po)
add_subdirectory(src)


function(create_zip output_file input_files working_dir)
    add_custom_command(
        COMMAND ${CMAKE_COMMAND} -E tar "cf" "${output_file}" --format=zip -- ${input_files} 
        WORKING_DIRECTORY "${working_dir}"
        OUTPUT  "${output_file}"
        DEPENDS ${input_files}
        COMMENT "Zipping to ${output_file}."
    )
endfunction()

MACRO(GETSUBDIRS result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

# Generate Themes
GETSUBDIRS(THEMESDIR "${PROJECT_SOURCE_DIR}/themes/system-themes/")
FOREACH(THEMENAME ${THEMESDIR})
    file(GLOB THEME_DEFAULT_FILES "${PROJECT_SOURCE_DIR}/themes/system-themes/${THEMENAME}/*")
    create_zip("${CMAKE_CURRENT_BINARY_DIR}/${THEMENAME}.mmextheme" "${THEME_DEFAULT_FILES}" "${PROJECT_SOURCE_DIR}/themes/system-themes/${THEMENAME}/")
    list(APPEND THEMEFILES "${CMAKE_CURRENT_BINARY_DIR}/${THEMENAME}.mmextheme")
ENDFOREACH()
add_custom_target(generate_theme_files ALL DEPENDS ${THEMEFILES})

# Generate Reports
GETSUBDIRS(GRMGROUPDIR "${PROJECT_SOURCE_DIR}/general-reports/packages/")
FOREACH(GRMGROUP ${GRMGROUPDIR})
    GETSUBDIRS(GRMDIR "${PROJECT_SOURCE_DIR}/general-reports/packages/${GRMGROUP}/")
    FOREACH(GRMNAME ${GRMDIR})
        set(GRM_DEFAULT_FILES   "${PROJECT_SOURCE_DIR}/general-reports/packages/${GRMGROUP}/${GRMNAME}/description.txt" 
                                "${PROJECT_SOURCE_DIR}/general-reports/packages/${GRMGROUP}/${GRMNAME}/luacontent.lua" 
                                "${PROJECT_SOURCE_DIR}/general-reports/packages/${GRMGROUP}/${GRMNAME}/sqlcontent.sql" 
                                "${PROJECT_SOURCE_DIR}/general-reports/packages/${GRMGROUP}/${GRMNAME}/template.htt"
        )
        create_zip("${CMAKE_CURRENT_BINARY_DIR}/${GRMGROUP}-${GRMNAME}.grm" "${GRM_DEFAULT_FILES}" "${PROJECT_SOURCE_DIR}/general-reports/packages/${GRMGROUP}/${GRMNAME}/")
        list(APPEND GRMFILES "${CMAKE_CURRENT_BINARY_DIR}/${GRMGROUP}-${GRMNAME}.grm")
    ENDFOREACH()
ENDFOREACH()
add_custom_target(generate_grm_files ALL DEPENDS ${GRMFILES})

## Tuning for VisualStudio IDE ##
if(NOT CMAKE_VERSION VERSION_LESS 3.6)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${MMEX_EXE})
endif()

# --------- Install ---------
# Before moving to INSTALL commands for macOS bundle this needs to be set
if(APPLE)
    string(REGEX REPLACE "^(.).*\\.([0-9]+)$" "\\1\\2" MMEX_VERSION_UNSTABLE_SHORT ${MMEX_VERSION_WITH_UNSTABLE})

    # Configure variables that are substituted into the plist
    # https://cmake.org/cmake/help/latest/prop_tgt/MACOSX_BUNDLE_INFO_PLIST.html
    # https://developer.apple.com/library/archive/documentation/General/Reference/InfoPlistKeyReference/Introduction/Introduction.html

    set_target_properties(${MMEX_EXE} PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
        MACOSX_BUNDLE_EXECUTABLE_NAME ${MMEX_EXE}
        MACOSX_BUNDLE_GUI_IDENTIFIER org.moneymanagerex.${MMEX_EXE}
        MACOSX_BUNDLE_BUNDLE_VERSION ${MMEX_VERSION}${MMEX_VERSION_UNSTABLE_SHORT}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${MMEX_VERSION}
        MACOSX_BUNDLE_LONG_VERSION_STRING ${MMEX_VERSION_FULL}
        MACOSX_BUNDLE_COPYRIGHT "Copyright В© 2009-2017 Nikolay\nCopyright В© 2011-2017 LiSheng\nCopyright В© 2013-2017 James, Gabriele\nCopyright В© 2010-2017 Stefano\nCopyright В© 2009-2010 VaDiM, Wesley Ellis"
        MACOSX_BUNDLE_ICON_FILE ${MACOSX_APP_ICON_NAME}
        # Set the path to the plist template
        MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/resources/MacOSXBundleInfo.plist.in")
endif()

if(WIN32)
    if(MMEX_PORTABLE_INSTALL)
        # Besides normal installable version, for windows the portable
        # version needs additionall files
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/mmexini.db3" "")
        install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/mmexini.db3"
            DESTINATION "${MMEX_DOC_DIR}")
    endif()

    # add shared wxWidgets DLLs
    get_filename_component(WXDLLSUFF "${wxWidgets_LIB_DIR}" NAME)
    if(WXDLLSUFF MATCHES "_dll$")
        string(REPLACE _dll .dll WXDLLSUFF ${WXDLLSUFF})
        set(WXDLLPREF ${wxWidgets_VERSION_MAJOR}${wxWidgets_VERSION_MINOR})
        # DLL filenames are different for different versions
        # see https://github.com/wxWidgets/wxWidgets/blob/master/build/tools/msvs/package.bat#L42
        if(wxWidgets_VERSION_MINOR MATCHES "^[13579]$")
            set(WXDLLPREF ${WXDLLPREF}${wxWidgets_VERSION_PATCH})
        endif()
        set(WXDLLPREF ${WXDLLPREF}u)

        if(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)
            list(APPEND MMEX_WXDLLS
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
        endif()
        if(CMAKE_INSTALL_DEBUG_LIBRARIES)
            set(WXDLLPREF ${WXDLLPREF}d)
            list(APPEND MMEX_WXDLLS
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
        endif()
        unset(WXDLLPREF)

        # check custom build name vs original one
        foreach(m_dll ${MMEX_WXDLLS})
            set(m_dll "${wxWidgets_LIB_DIR}/${m_dll}")
            foreach(m_ext vc_mmex.dll vc_custom.dll vc_x64_custom.dll "${WXDLLSUFF}" NOTFOUND)
                if(EXISTS "${m_dll}_${m_ext}")
                    list(APPEND CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS "${m_dll}_${m_ext}")
                    break()
                elseif(NOT m_ext)
                    message(SEND_ERROR "${m_dll}_${WXDLLSUFF} not found")
                endif()
            endforeach()
        endforeach()
        unset(m_dll)
        unset(m_ext)
        unset(MMEX_WXDLLS)
        unset(WXDLLSUFF)
    endif()

    # This may be required to run on old Windows version
    set(CMAKE_INSTALL_UCRT_LIBRARIES ON)

    # Following can be used for Debug builds, but
    # cannot be redistributed according to
    # https://www.visualstudio.com/productinfo/2017-redistribution-vs#visual-c-runtime-files
    # https://www.visualstudio.com/license-terms/distributable-code-microsoft-visual-studio-2015-rc-microsoft-visual-studio-2015-sdk-rc-includes-utilities-buildserver-files/#visual-c-runtime
    # https://docs.microsoft.com/cpp/ide/determining-which-dlls-to-redistribute
    # set(CMAKE_INSTALL_DEBUG_LIBRARIES ON)
    # set(CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY ON)

    include(InstallRequiredSystemLibraries)
endif()

# Help Files
install(FILES
    contrib.txt
    license.txt
    README.TXT
    DESTINATION "${MMEX_DOC_DIR}")
install(DIRECTORY
    docs/
    DESTINATION "${MMEX_DOC_DIR}/help")
install(FILES
    DESTINATION "${MMEX_DOC_DIR}/help")

# Resources
install(FILES
    3rd/ChartNew.js/ChartNew.js
    3rd/ChartNew.js/Add-ins/format.js 
    3rd/apexcharts.js/dist/apexcharts.min.js  
    resources/ie-polyfill/polyfill.min.js
    resources/ie-polyfill/classlist.min.js
    resources/ie-polyfill/resize-observer.js
    resources/ie-polyfill/findindex.min.js
    resources/ie-polyfill/umd.min.js
    resources/sorttable.js
    resources/jquery.min.js
    resources/home_page.htt
    resources/kaching.wav
    resources/mmex.png
    DESTINATION "${MMEX_RES_DIR}")

#Themes & GRM
install(FILES ${THEMEFILES} DESTINATION ${MMEX_RES_DIR_THEMES})
install(FILES ${GRMFILES} DESTINATION ${MMEX_RES_DIR_REPORTS})

if(LINUX)
    # .desktop File
    install(FILES
        resources/mmex.desktop
        DESTINATION share/applications)
endif()

# Icons
if(LINUX)
    install(FILES
        resources/mmex.svg
        DESTINATION share/icons/hicolor/scalable/apps)
elseif(APPLE)
    install(FILES
        "${MACOSX_APP_ICON_FILE}"
        DESTINATION "${MMEX_RES_DIR}")
elseif(WIN32)
    install(FILES
        resources/mmex.ico
        DESTINATION "${MMEX_RES_DIR}")
endif()

# libcurl for Windows, if specified and exists then copy to bin
if(CMAKE_PREFIX_PATH)
    file(TO_CMAKE_PATH "${CMAKE_PREFIX_PATH}/bin/libcurl.dll" CURL_LIBPATH)
    if(EXISTS "${CURL_LIBPATH}")
        install(FILES 
            "${CURL_LIBPATH}"
            DESTINATION "${MMEX_BIN_DIR}")
    endif()
endif()

# --------- CPack ---------
set(CPACK_PACKAGE_NAME ${MMEX_EXE})
set(CPACK_PACKAGE_VERSION ${MMEX_VERSION_FULL})
set(CPACK_PACKAGE_VERSION_MAJOR ${MMEX_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${MMEX_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${MMEX_VERSION_PATCH})
set(CPACK_PACKAGE_CONTACT "MoneyManagerEx Developers <developers@moneymanagerex.org>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Simple to use financial management software")
set(CPACK_PACKAGE_DESCRIPTION "Money Manager Ex (MMEX) is a free, open-source, cross-platform, easy-to-use personal finance software. It primarily helps organize one's finances and keeps track of where, when and how the money goes. MMEX includes all the basic features that 90% of users would want to see in a personal finance application. The design goals are to concentrate on simplicity and user friendliness - something one can use everyday.")
set(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/resources/mmex.ico")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/license.txt")
set(CPACK_RESOURCE_FILE_README "${PROJECT_SOURCE_DIR}/README.TXT")
# TODO: do we package Debug builds with stripped binaries?
set(CPACK_STRIP_FILES ON)
set(CPACK_DEBIAN_PACKAGE_SECTION misc)
set(CPACK_DEBIAN_PACKAGE_PRIORITY extra)
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE http://www.moneymanagerex.org/)
set(CPACK_DEBIAN_PACKAGE_REPLACES moneymanagerex)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_RELEASE 1)
# set(CPACK_DEBIAN_PACKAGE_DEPENDS wx${wxWidgets_VERSION_MAJOR}.${wxWidgets_VERSION_MINOR}-i18n)
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_RPM_FILE_NAME RPM-DEFAULT)
set(CPACK_RPM_PACKAGE_LICENSE GPL-2+)
set(CPACK_RPM_PACKAGE_URL ${CPACK_DEBIAN_PACKAGE_HOMEPAGE})
set(CPACK_RPM_PACKAGE_OBSOLETES ${CPACK_DEBIAN_PACKAGE_REPLACES})
set(CPACK_RPM_PACKAGE_AUTOREQ ON)
set(CPACK_RPM_PACKAGE_DESCRIPTION ${CPACK_PACKAGE_DESCRIPTION})
set(CPACK_RPM_PACKAGE_RELEASE_DIST ON)
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
    /usr/share/applications
    /usr/share/icons
    /usr/share/icons/hicolor
    /usr/share/icons/hicolor/scalable
    /usr/share/icons/hicolor/scalable/apps)
# TODO: distribute dbg packages
# set(CPACK_RPM_DEBUGINFO_PACKAGE ON)
# set(CPACK_RPM_DEBUGINFO_FILE_NAME RPM-DEFAULT)

if(LINUX)
    if(LINUX_DISTRO STREQUAL "Ubuntu" OR LINUX_DISTRO STREQUAL "Debian" OR LINUX_DISTRO STREQUAL "LinuxMint" OR LINUX_DISTRO STREQUAL "Raspbian")
        set(CPACK_GENERATOR DEB)
        set(CPACK_DEBIAN_PACKAGE_RELEASE "${LINUX_DISTRO_STRING}")
    elseif(LINUX_DISTRO STREQUAL "Fedora")
        set(CPACK_GENERATOR RPM)
        set(CPACK_RPM_PACKAGE_RELEASE "${LINUX_DISTRO_STRING}")
    elseif(LINUX_DISTRO STREQUAL "CentOS")
        set(CPACK_GENERATOR RPM)
        string(REGEX REPLACE "\\..*" "" CPACK_RPM_PACKAGE_RELEASE_DIST ${LINUX_DISTRO_REL})
        set(CPACK_RPM_PACKAGE_RELEASE "${LINUX_DISTRO_STRING}")
    elseif(LINUX_DISTRO MATCHES "openSUSE" OR (LINUX_DISTRO MATCHES "SUSE" AND LINUX_DISTRO_REL VERSION_GREATER 42))
        set(CPACK_GENERATOR RPM)
        set(CPACK_RPM_PACKAGE_RELEASE "${LINUX_DISTRO_DESCRIPTION}")
    elseif(LINUX_DISTRO STREQUAL "Slackware")
        set(CPACK_GENERATOR TXZ)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(CPACK_SYSTEM_NAME x86_64)
        else()
            set(CPACK_SYSTEM_NAME i586)
        endif()
        install(FILES util/slackware-desc
                DESTINATION /install RENAME slack-desc)
        install(PROGRAMS util/slackware-doinst.sh
                DESTINATION /install RENAME doinst.sh)
        set(CPACK_SYSTEM_NAME ${CPACK_SYSTEM_NAME}-1_slack${LINUX_DISTRO_REL})
        set(CPACK_SET_DESTDIR ON)
        set(CPACK_PACKAGING_INSTALL_PREFIX /usr) # not working?
        set(CMAKE_INSTALL_PREFIX /usr)
        set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
    elseif(LINUX_DISTRO STREQUAL "Arch")
    else()
        message(WARNING "Unknown Linux distribution - generating generic package.")
    endif()
elseif(APPLE)
    set(CPACK_GENERATOR DragNDrop)
    set(CPACK_DMG_VOLUME_NAME "Money Manager Ex")
    set(CPACK_DMG_FORMAT UDZO)
    set(CPACK_DMG_BACKGROUND_IMAGE "${PROJECT_SOURCE_DIR}/resources/dmg-background.png")
    set(CPACK_DMG_DS_STORE "${PROJECT_SOURCE_DIR}/resources/dmg-DS_Store")
    set(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/resources/mmex-package.icns")
    set(CPACK_BUNDLE_PLIST "${PROJECT_SOURCE_DIR}/resources/Info.plist")
elseif(WIN32)
    # With using http://nsis.sourceforge.net/Main_Page
    set(CPACK_GENERATOR NSIS ZIP)
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "Money Manager EX")
    set(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/resources\\\\mmex.ico")
    set(CPACK_NSIS_MUI_ICON "${CPACK_PACKAGE_ICON}")
    set(CPACK_NSIS_MUI_UNIICON "${CPACK_PACKAGE_ICON}")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "${MMEX_BIN_DIR}\\\\${MMEX_EXE}${CMAKE_EXECUTABLE_SUFFIX}")
    set(CPACK_NSIS_MUI_FINISHPAGE_RUN ${MMEX_EXE}${CMAKE_EXECUTABLE_SUFFIX})
    set(CPACK_NSIS_URL_INFO_ABOUT ${CPACK_DEBIAN_PACKAGE_HOMEPAGE})
    set(CPACK_NSIS_CONTACT "${CPACK_PACKAGE_CONTACT}")
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
            "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Money Manager EX.lnk' '$INSTDIR\\\\${MMEX_BIN_DIR}\\\\${MMEX_EXE}${CMAKE_EXECUTABLE_SUFFIX}'")
    set(CPACK_NSIS_DELETE_ICONS_EXTRA "Delete '$SMPROGRAMS\\\\$START_MENU\\\\Money Manager EX.lnk'")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
endif()

if(LINUX_DISTRO STREQUAL "Arch")
    # Use dedicated tool to build Arch Linux package
    set(CMAKE_INSTALL_PREFIX /usr)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ARCHLINUX_ARCH x86_64)
    else()
        set(ARCHLINUX_ARCH i686)
    endif()
    string(REPLACE - . CPACK_PACKAGE_VERSION ${MMEX_VERSION_FULL})
    configure_file(util/PKGBUILD.in PKGBUILD @ONLY)

    # allow use reserved for cpack package target
    cmake_policy(PUSH)
    if(POLICY CMP0037)
        cmake_policy(SET CMP0037 OLD)
    endif()
    add_custom_target(package
        COMMAND sh -c "if [ `id -u` -eq 0 ]; then chown nobody -R . && runuser nobody -c makepkg; else makepkg; fi"
        VERBATIM)
    cmake_policy(POP)
else()
    # workarount for old CMake without DEB-DEFAULT/RPM-DEFAULT support
    if(CMAKE_VERSION VERSION_LESS 3.6)
        if(CPACK_GENERATOR STREQUAL "DEB")
            if(NOT CPACK_DEBIAN_PACKAGE_ARCHITECTURE)
                find_program(DPKG_CMD dpkg DOC "Debian packaging tool")
                if(NOT DPKG_CMD)
                    message("CPackDeb: Can not find dpkg in your path, default architecture to i386.")
                    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE i386)
                endif()
                execute_process(COMMAND "${DPKG_CMD}" --print-architecture
                    OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
                    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
            endif()
            set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}-${CPACK_DEBIAN_PACKAGE_RELEASE}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
        elseif(CPACK_GENERATOR STREQUAL "RPM")
            if(NOT CPACK_RPM_PACKAGE_ARCHITECTURE)
                execute_process(COMMAND uname -m
                    OUTPUT_VARIABLE CPACK_RPM_PACKAGE_ARCHITECTURE
                    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
            endif()
            set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}-${CPACK_RPM_PACKAGE_ARCHITECTURE}")
        endif()
    endif()

    include(CPack)
endif()


# --------- Print summary after configuration stage ---------
message(STATUS "")
message(STATUS "MMEX configuration summary")
message(STATUS "==========================")
message(STATUS "Version        : ${MMEX_VERSION_FULL}")
if(GIT_COMMIT_HASH)
    message(STATUS "Commit         : ${GIT_COMMIT_HASH} (${GIT_COMMIT_DATE})")
endif()
if(GIT_BRANCH)
    message(STATUS "Branch         : ${GIT_BRANCH}")
endif()
message(STATUS "Host system    : ${CMAKE_HOST_SYSTEM_NAME} ${CMAKE_HOST_SYSTEM_PROCESSOR}")
if(APPLE AND CMAKE_OSX_DEPLOYMENT_TARGET)
    message(STATUS "Target system  : ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR} >= ${CMAKE_OSX_DEPLOYMENT_TARGET}")
else()
    message(STATUS "Target system  : ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
endif()
if(CMAKE_BUILD_TYPE)
    message(STATUS "Build types    : ${CMAKE_BUILD_TYPE}")
else()
    message(STATUS "Build types    : ${CMAKE_CONFIGURATION_TYPES}")
endif()
message(STATUS "Generator      : ${CMAKE_GENERATOR}")
if(MSVC)
    if(CMAKE_VS_PLATFORM_TOOLSET)
        message(STATUS "Toolset        : ${CMAKE_VS_PLATFORM_TOOLSET}")
        if(CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE)
            message(STATUS "Toolset host   : ${CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE}")
        endif()
    endif()
    if(CMAKE_VS_PLATFORM_NAME)
        message(STATUS "Platform       : ${CMAKE_VS_PLATFORM_NAME}")
    endif()
    message(STATUS "VC install dir : $ENV{VCINSTALLDIR}")
    if(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
        foreach(m_dll ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
            get_filename_component(m_dll ${m_dll} NAME_WE)
            set(m_dlls "${m_dlls} ${m_dll}")
        endforeach()
        message(STATUS "Run-time DLLs  :${m_dlls}")
    endif()
endif()
message(STATUS "Install prefix : ${CMAKE_INSTALL_PREFIX}")
if(WIN32)
    message(STATUS "Portable state : ${MMEX_PORTABLE_INSTALL}")
endif()
message(STATUS "DB encryption  : ${WXSQLITE3_HAVE_CODEC}")

message(STATUS "")
message(STATUS "Versions")
message(STATUS "--========--")
message(STATUS "${CMAKE_HOST_SYSTEM_NAME} ${CMAKE_HOST_SYSTEM_VERSION}")
if(LINUX_DISTRO_STRING)
    message(STATUS "${LINUX_DISTRO_STRING}")
    message(STATUS "${LINUX_DISTRO_DESCRIPTION}")
endif()
message(STATUS "CMake ${CMAKE_VERSION}")
message(STATUS "${CMAKE_MAKE_VERSION}")
if(CCACHE_PROGRAM)
    get_filename_component(CCACHE_TOOL "${CCACHE_PROGRAM}" NAME_WE)
    if(CCACHE_VERSION)
        message(STATUS "${CCACHE_TOOL} ${CCACHE_VERSION}")
    else()
        message(STATUS ${CCACHE_TOOL})
    endif()
endif()
if(VS_VERSION)
    message(STATUS "Visual Studio ${VS_VERSION}")
endif()
if(CMAKE_CXX_COMPILER_ID)
    message(STATUS "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
endif()
if(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
    message(STATUS "Win SDK ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
endif()
if(wxWidgets_VERSION_STRING)
    message(STATUS "wxWidgets ${wxWidgets_VERSION_STRING}")
endif()
if(wxSQLite3_VERSION)
    message(STATUS "wxSQLite3 ${wxSQLite3_VERSION}")
endif()
if(RapidJSON_VERSION)
    message(STATUS "RapidJSON ${RapidJSON_VERSION}")
endif()
if(Lua_VERSION)
    message(STATUS "Lua ${Lua_VERSION}")
endif()
if(CURL_VERSION)
    message(STATUS "curl ${CURL_VERSION}")
endif()
message(STATUS "gettext ${GETTEXT_VERSION_STRING}")
message(STATUS "------------------ 8< -----------------")
