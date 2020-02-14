IF(NOT InMMEX)
    MESSAGE(FATAL_ERROR "Use the top-level CMake script!")
ENDIF(NOT InMMEX)

# disable all warnings while compile sources in 3rd
IF(MSVC)
    SET_DIRECTORY_PROPERTIES(PROPERTIES COMPILE_OPTIONS /W0)
ELSE()
    SET_DIRECTORY_PROPERTIES(PROPERTIES COMPILE_OPTIONS -w)
ENDIF()

ADD_LIBRARY(RapidJSON INTERFACE)
TARGET_INCLUDE_DIRECTORIES(RapidJSON INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include>)

ADD_LIBRARY(LuaGlue INTERFACE)
TARGET_INCLUDE_DIRECTORIES(LuaGlue INTERFACE LuaGlue/include)

FIND_PACKAGE(Lua) 
IF(LUA_FOUND)
    MESSAGE(STATUS "Found Lua: ${LUA_LIBRARIES} (found version \"${LUA_VERSION_STRING}\")") 
    ADD_LIBRARY(Lua INTERFACE)
    TARGET_INCLUDE_DIRECTORIES(Lua INTERFACE ${LUA_INCLUDE_DIR})
    TARGET_LINK_LIBRARIES(Lua INTERFACE ${LUA_LIBRARIES})
ELSE()
    add_library(Lua STATIC EXCLUDE_FROM_ALL
        lua/lapi.c
        lua/lapi.h
        lua/lauxlib.c
        lua/lauxlib.h
        lua/lbaselib.c
        lua/lcode.c
        lua/lcode.h
        lua/lcorolib.c
        lua/lctype.c
        lua/lctype.h
        lua/ldblib.c
        lua/ldebug.c
        lua/ldebug.h
        lua/ldo.c
        lua/ldo.h
        lua/ldump.c
        lua/lfunc.c
        lua/lfunc.h
        lua/lgc.c
        lua/lgc.h
        lua/linit.c
        lua/liolib.c
        lua/ljumptab.h
        lua/llex.c
        lua/llex.h
        lua/llimits.h
        lua/lmathlib.c
        lua/lmem.c
        lua/lmem.h
        lua/loadlib.c
        lua/lobject.c
        lua/lobject.h
        lua/lopcodes.c
        lua/lopcodes.h
        lua/lopnames.h
        lua/loslib.c
        lua/lparser.c
        lua/lparser.h
        lua/lprefix.h
        lua/lstate.c
        lua/lstate.h
        lua/lstring.c
        lua/lstring.h
        lua/lstrlib.c
        lua/ltable.c
        lua/ltable.h
        lua/ltablib.c
        lua/ltests.c
        lua/ltests.h
        lua/ltm.c
        lua/ltm.h
        lua/lua.c
        lua/lua.h
        lua/luaconf.h
        lua/lualib.h
        lua/lundump.c
        lua/lundump.h
        lua/lutf8lib.c
        lua/lvm.c
        lua/lvm.h
        lua/lzio.c
        lua/lzio.h
        lua/onelua.c
    )
    #add C++ wrapper for Lua include files
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/resources/lua.hpp lua.hpp COPYONLY)
    TARGET_INCLUDE_DIRECTORIES(Lua INTERFACE lua ${CMAKE_CURRENT_BINARY_DIR})
    IF(NOT WIN32)
        TARGET_COMPILE_DEFINITIONS(Lua PRIVATE
            LUA_USE_POSIX
            LUA_USE_DLOPEN
            LUA_USE_STRTODHEX
            LUA_USE_AFORMAT
            LUA_USE_LONGLONG)
    ENDIF()

    IF(LINUX)
        INCLUDE(CheckLibraryExists)
        CHECK_LIBRARY_EXISTS(${CMAKE_DL_LIBS} dlopen "" HAVE_DLOPEN)
        IF(NOT HAVE_DLOPEN)
            MESSAGE(SEND_ERROR "Could not find required dl library.")
        ENDIF()
        TARGET_LINK_LIBRARIES(Lua PRIVATE ${CMAKE_DL_LIBS})
    ENDIF()

    file(STRINGS lua/lua.h
        LUA_VERSION_STRING
        REGEX "#define LUA_VERSION_(MAJOR|MINOR|RELEASE)[\t ]+\"[0-9]+\"")
    string(REPLACE ";" "." LUA_VERSION_STRING "${LUA_VERSION_STRING}")
    string(REGEX REPLACE "[^0-9.]+" "" LUA_VERSION_STRING ${LUA_VERSION_STRING})

ENDIF(LUA_FOUND)
TARGET_COMPILE_DEFINITIONS(Lua INTERFACE LUA_COMPAT_5_3)

ADD_LIBRARY(HTML-template STATIC EXCLUDE_FROM_ALL cgitemplate/html_template.cpp)
TARGET_INCLUDE_DIRECTORIES(HTML-template INTERFACE cgitemplate)

FIND_PACKAGE(PkgConfig)
IF(PkgConfig_FOUND)
    PKG_SEARCH_MODULE(wxSQLite3 QUIET wxsqlite3-3.0 wxsqlite3 wxsqlite>=3)
ENDIF()
IF(wxSQLite3_FOUND)
    MESSAGE(STATUS "Found wxSQLite3: ${wxSQLite3_LIBRARIES} (found version \"${wxSQLite3_VERSION}\")")
    ADD_LIBRARY(wxSQLite3 INTERFACE)
    TARGET_INCLUDE_DIRECTORIES(wxSQLite3 INTERFACE ${wxSQLite3_INCLUDE_DIRS})
    TARGET_LINK_LIBRARIES(wxSQLite3 INTERFACE ${wxSQLite3_LIBRARIES} wxWidgets)
    TARGET_COMPILE_OPTIONS(wxSQLite3 INTERFACE ${wxSQLite3_CFLAGS})
ELSE()
    FILE(STRINGS wxsqlite3/include/wx/wxsqlite3_version.h
        wxSQLite3_VERSION LIMIT_COUNT 1
            REGEX "#define WXSQLITE3_VERSION_STRING[\t ]+\"[^\"]+ [0-9.]+\"")
    STRING(REGEX REPLACE ".+\"[^\"]+ ([0-9.]+)\".*" \\1 wxSQLite3_VERSION ${wxSQLite3_VERSION})
    IF(wxSQLite3_VERSION VERSION_LESS 4)
        SET(sqlite3_dir sqlite3/secure)
    ELSE()
        SET(sqlite3_dir sqlite3secure)
    ENDIF()
    ADD_LIBRARY(SQLite3 STATIC EXCLUDE_FROM_ALL wxsqlite3/${sqlite3_dir}/src/sqlite3secure.c)
    TARGET_INCLUDE_DIRECTORIES(SQLite3 PUBLIC wxsqlite3/${sqlite3_dir}/src)

    IF(LINUX)
        INCLUDE(CheckLibraryExists)
        CHECK_LIBRARY_EXISTS(${CMAKE_DL_LIBS} dlopen "" HAVE_DLOPEN)
        IF(NOT HAVE_DLOPEN)
            MESSAGE(SEND_ERROR "Could not find required dl library.")
        ENDIF()
        TARGET_LINK_LIBRARIES(SQLite3 PRIVATE ${CMAKE_DL_LIBS})
    ENDIF()

    ADD_LIBRARY(wxSQLite3 STATIC EXCLUDE_FROM_ALL wxsqlite3/src/wxsqlite3.cpp)
    TARGET_INCLUDE_DIRECTORIES(wxSQLite3 PUBLIC wxsqlite3/include)
    TARGET_LINK_LIBRARIES(wxSQLite3 PUBLIC wxWidgets SQLite3)

    TARGET_COMPILE_DEFINITIONS(SQLite3
        PRIVATE
            NOPCH
            SQLITE_CORE
            SQLITE_ENABLE_FTS5
            SQLITE_ENABLE_EXTFUNC
            SQLITE_ENABLE_COLUMN_METADATA
            SQLITE_ENABLE_JSON1
            HAVE_ACOSH
            HAVE_ASINH
            HAVE_ATANH
            HAVE_ISBLANK
        PUBLIC
            SQLITE_HAS_CODEC
            CODEC_TYPE=CODEC_TYPE_AES128)
    TARGET_COMPILE_DEFINITIONS(wxSQLite3
        PUBLIC
            WXSQLITE3_HAVE_CODEC
            WXSQLITE3_HAVE_METADATA
            WXSQLITE3_USER_AUTHENTICATION)

    IF(CMAKE_VERSION VERSION_LESS 3.1)
        INCLUDE(CheckCXXCompilerFlag)
        CHECK_CXX_COMPILER_FLAG("-std=gnu++11" COMPILER_SUPPORTS_GXX11)
        CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
        CHECK_CXX_COMPILER_FLAG("-std=gnu++0x" COMPILER_SUPPORTS_GXX0X)
        CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
        IF(COMPILER_SUPPORTS_GXX11)
            TARGET_COMPILE_OPTIONS(wxSQLite3 PRIVATE -std=gnu++11)
        ELSEIF(COMPILER_SUPPORTS_CXX11)
            TARGET_COMPILE_OPTIONS(wxSQLite3 PRIVATE -std=c++11)
        ELSEIF(COMPILER_SUPPORTS_GXX0X)
            TARGET_COMPILE_OPTIONS(wxSQLite3 PRIVATE -std=gnu++0x)
        ELSEIF(COMPILER_SUPPORTS_CXX0X)
            TARGET_COMPILE_OPTIONS(wxSQLite3 PRIVATE -std=c++0x)
        ELSE()
            MESSAGE(SEND_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support.")
        ENDIF()
    ELSEIF(CMAKE_VERSION VERSION_LESS 3.8)
        TARGET_COMPILE_FEATURES(wxSQLite3 PRIVATE cxx_range_for)
    ELSE()
        TARGET_COMPILE_FEATURES(wxSQLite3 PRIVATE cxx_std_11)
    ENDIF()

    IF(NOT MSVC) # wxSQLite3 security ext doesn't require C 99 for MSVC
        IF(CMAKE_VERSION VERSION_LESS 3.1)
            INCLUDE(CheckCCompilerFlag)
            CHECK_C_COMPILER_FLAG("-std=c99" COMPILER_SUPPORTS_C99)
            IF(COMPILER_SUPPORTS_C99)
                TARGET_COMPILE_OPTIONS(SQLite3 PRIVATE -std=c99)
            ELSE()
                MESSAGE(SEND_ERROR "The compiler ${CMAKE_C_COMPILER} has no C99 support.")
            ENDIF()
        ELSEIF(CMAKE_VERSION VERSION_LESS 3.8)
            TARGET_COMPILE_FEATURES(SQLite3 PRIVATE c_restrict)
        ELSE()
            TARGET_COMPILE_FEATURES(SQLite3 PRIVATE c_std_99)
        ENDIF()
    ENDIF(NOT MSVC)
ENDIF(wxSQLite3_FOUND)
