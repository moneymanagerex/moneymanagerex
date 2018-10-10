IF(NOT InMMEX)
    MESSAGE(FATAL_ERROR "Use the top-level CMake script!")
ENDIF(NOT InMMEX)

# disable all warnings while compile sources in 3rd
IF(MSVC)
    STRING(REGEX REPLACE "(^| )[/-]W[0-4]( |$)" " " CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    STRING(REGEX REPLACE "(^| )[/-]W[0-4]( |$)" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W0")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W0")
ELSE()
    ADD_COMPILE_OPTIONS("-w")
ENDIF()

FIND_PACKAGE(RapidJSON 1.1 QUIET)
IF(NOT RapidJSON_FOUND)
    SET(RAPIDJSON_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include")
ENDIF()
MESSAGE(STATUS "Found RapidJSON: ${RAPIDJSON_INCLUDE_DIRS}")
ADD_LIBRARY(RapidJSON INTERFACE)
TARGET_INCLUDE_DIRECTORIES(RapidJSON SYSTEM INTERFACE
    $<BUILD_INTERFACE:${RAPIDJSON_INCLUDE_DIRS}>)

ADD_LIBRARY(LuaGlue INTERFACE)
TARGET_INCLUDE_DIRECTORIES(LuaGlue SYSTEM INTERFACE LuaGlue/include)

FIND_PACKAGE(Lua)
IF(LUA_FOUND)
    ADD_LIBRARY(Lua INTERFACE)
    TARGET_INCLUDE_DIRECTORIES(Lua SYSTEM INTERFACE ${LUA_INCLUDE_DIR})
    TARGET_LINK_LIBRARIES(Lua INTERFACE ${LUA_LIBRARIES})
ELSE()
    ADD_LIBRARY(Lua STATIC EXCLUDE_FROM_ALL
        lua/lapi.c
        lua/lauxlib.c
        lua/lbaselib.c
        lua/lbitlib.c
        lua/lcode.c
        lua/lcorolib.c
        lua/lctype.c
        lua/ldblib.c
        lua/ldebug.c
        lua/ldo.c
        lua/ldump.c
        lua/lfunc.c
        lua/lgc.c
        lua/linit.c
        lua/liolib.c
        lua/llex.c
        lua/lmathlib.c
        lua/lmem.c
        lua/loadlib.c
        lua/lobject.c
        lua/lopcodes.c
        lua/loslib.c
        lua/lparser.c
        lua/lstate.c
        lua/lstring.c
        lua/lstrlib.c
        lua/ltable.c
        lua/ltablib.c
        lua/ltm.c
        lua/lua.c
        lua/lundump.c
        lua/lutf8lib.c
        lua/lvm.c
        lua/lzio.c)
    #add C++ wrapper for Lua include files
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/resources/lua.hpp lua.hpp COPYONLY)
    TARGET_INCLUDE_DIRECTORIES(Lua SYSTEM INTERFACE lua ${CMAKE_CURRENT_BINARY_DIR})
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

    FILE(STRINGS lua/lua.h
        LUA_VERSION_STRING
        REGEX "#define LUA_VERSION_(MAJOR|MINOR|RELEASE)[\t ]+\"[0-9]+\"")
    STRING(REPLACE ";" "." LUA_VERSION_STRING "${LUA_VERSION_STRING}")
    STRING(REGEX REPLACE "[^0-9.]+" "" LUA_VERSION_STRING ${LUA_VERSION_STRING})

ENDIF(LUA_FOUND)
TARGET_COMPILE_DEFINITIONS(Lua INTERFACE LUA_COMPAT_5_2)


ADD_LIBRARY(HTML-template STATIC EXCLUDE_FROM_ALL cgitemplate/html_template.cpp)
TARGET_INCLUDE_DIRECTORIES(HTML-template SYSTEM INTERFACE cgitemplate)

ADD_LIBRARY(Mongoose STATIC EXCLUDE_FROM_ALL mongoose/mongoose.c)
TARGET_INCLUDE_DIRECTORIES(Mongoose SYSTEM INTERFACE .)
TARGET_COMPILE_DEFINITIONS(Mongoose
    PRIVATE
        # MG_ENABLE_SSL
        MG_ENABLE_HTTP_WEBSOCKET=0
        MG_DISABLE_HTTP_DIGEST_AUTH
        CS_DISABLE_SHA1
        CS_DISABLE_MD5)

FIND_PACKAGE(PkgConfig)
IF(PkgConfig_FOUND)
    PKG_SEARCH_MODULE(wxSQLite3 QUIET wxsqlite3-3.0 wxsqlite3 wxsqlite>=3)
ENDIF()
IF(wxSQLite3_FOUND)
    MESSAGE(STATUS "Found wxSQLite3: ${wxSQLite3_LIBRARIES} (found version \"${wxSQLite3_VERSION}\")")

    FILE(WRITE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/WXSQLITE3_HAVE_CODEC.c"
        "#include <wx/wxsqlite3opt.h>\n#if !WXSQLITE3_HAVE_CODEC\n#error\n#endif\n\nint main(int argc, char** argv)\n{\n  (void)argv;\n  (void)argc;\n  return 0;\n}\n")
    TRY_COMPILE(WXSQLITE3_HAVE_CODEC ${CMAKE_BINARY_DIR}
        "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/WXSQLITE3_HAVE_CODEC.c"
        CMAKE_FLAGS "-DINCLUDE_DIRECTORIES:STRING=${wxSQLite3_INCLUDE_DIRS}")
    SET(WXSQLITE3_HAVE_CODEC ${WXSQLITE3_HAVE_CODEC} CACHE INTERNAL "Have symbol ${SYMBOL}")

    IF(WXSQLITE3_HAVE_CODEC OR MMEX_ENCRYPTION_OPTIONAL)
        ADD_LIBRARY(wxSQLite3 INTERFACE)
        TARGET_INCLUDE_DIRECTORIES(wxSQLite3 SYSTEM INTERFACE ${wxSQLite3_INCLUDE_DIRS})
        TARGET_LINK_LIBRARIES(wxSQLite3 INTERFACE ${wxSQLite3_LIBRARIES} wxWidgets)
        TARGET_COMPILE_OPTIONS(wxSQLite3 INTERFACE ${wxSQLite3_CFLAGS})
    ELSE()
        MESSAGE(WARNING "wxSQLite3 found does not support database encryption - compiling from sources")
    ENDIF()
ENDIF(wxSQLite3_FOUND)
IF(NOT wxSQLite3_FOUND OR NOT (WXSQLITE3_HAVE_CODEC OR MMEX_ENCRYPTION_OPTIONAL))
    SET(WXSQLITE3_HAVE_CODEC ON)
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
    TARGET_INCLUDE_DIRECTORIES(SQLite3 SYSTEM PUBLIC wxsqlite3/${sqlite3_dir}/src)

    IF(LINUX)
        INCLUDE(CheckLibraryExists)
        CHECK_LIBRARY_EXISTS(${CMAKE_DL_LIBS} dlopen "" HAVE_DLOPEN)
        IF(NOT HAVE_DLOPEN)
            MESSAGE(SEND_ERROR "Could not find required dl library.")
        ENDIF()
        TARGET_LINK_LIBRARIES(SQLite3 PRIVATE ${CMAKE_DL_LIBS})
    ENDIF()

    ADD_LIBRARY(wxSQLite3 STATIC EXCLUDE_FROM_ALL wxsqlite3/src/wxsqlite3.cpp)
    TARGET_INCLUDE_DIRECTORIES(wxSQLite3 SYSTEM PUBLIC wxsqlite3/include)
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
            CODEC_TYPE=CODEC_TYPE_AES128
            WXSQLITE3_USE_SQLCIPHER_LEGACY)
    TARGET_COMPILE_DEFINITIONS(wxSQLite3
        PUBLIC
            WXSQLITE3_HAVE_CODEC
            WXSQLITE3_HAVE_METADATA
            WXSQLITE3_USER_AUTHENTICATION)

    IF(";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";cxx_std_11;")
        TARGET_COMPILE_FEATURES(wxSQLite3 PRIVATE cxx_std_11)
    ELSEIF(";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";cxx_range_for;")
        TARGET_COMPILE_FEATURES(wxSQLite3 PRIVATE cxx_range_for)
    ELSE()
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
    ENDIF()

    IF(NOT MSVC) # wxSQLite3 security ext doesn't require C 99 for MSVC
        IF(";${CMAKE_C_COMPILE_FEATURES};" MATCHES ";c_std_99;")
            TARGET_COMPILE_FEATURES(SQLite3 PRIVATE c_std_99)
        ELSEIF(";${CMAKE_C_COMPILE_FEATURES};" MATCHES ";c_restrict;")
            TARGET_COMPILE_FEATURES(SQLite3 PRIVATE c_restrict)
        ELSE()
            INCLUDE(CheckCCompilerFlag)
            CHECK_C_COMPILER_FLAG("-std=c99" COMPILER_SUPPORTS_C99)
            IF(COMPILER_SUPPORTS_C99)
                TARGET_COMPILE_OPTIONS(SQLite3 PRIVATE -std=c99)
            ELSE()
                MESSAGE(SEND_ERROR "The compiler ${CMAKE_C_COMPILER} has no C99 support.")
            ENDIF()
        ENDIF()
    ENDIF()
ENDIF()

SET(RapidJSON_VERSION ${RapidJSON_VERSION} PARENT_SCOPE)
SET(Lua_VERSION ${LUA_VERSION_STRING} PARENT_SCOPE)
SET(wxSQLite3_VERSION ${wxSQLite3_VERSION} PARENT_SCOPE)
SET(WXSQLITE3_HAVE_CODEC ${WXSQLITE3_HAVE_CODEC} PARENT_SCOPE)
