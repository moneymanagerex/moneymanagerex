IF(NOT InMMEX)
    MESSAGE(FATAL_ERROR "Use the top-level CMake script!")
ENDIF(NOT InMMEX)

ADD_LIBRARY(LUA STATIC EXCLUDE_FROM_ALL
    lua/src/lapi.c
    lua/src/lauxlib.c
    lua/src/lbaselib.c
    lua/src/lbitlib.c
    lua/src/lcode.c
    lua/src/lcorolib.c
    lua/src/lctype.c
    lua/src/ldblib.c
    lua/src/ldebug.c
    lua/src/ldo.c
    lua/src/ldump.c
    lua/src/lfunc.c
    lua/src/lgc.c
    lua/src/linit.c
    lua/src/liolib.c
    lua/src/llex.c
    lua/src/lmathlib.c
    lua/src/lmem.c
    lua/src/loadlib.c
    lua/src/lobject.c
    lua/src/lopcodes.c
    lua/src/loslib.c
    lua/src/lparser.c
    lua/src/lstate.c
    lua/src/lstring.c
    lua/src/lstrlib.c
    lua/src/ltable.c
    lua/src/ltablib.c
    lua/src/ltm.c
    lua/src/lua.c
    lua/src/luac.c
    lua/src/lundump.c
    lua/src/lvm.c
    lua/src/lzio.c)

ADD_LIBRARY(wxSQLite3 STATIC EXCLUDE_FROM_ALL
    wxsqlite3/src/wxsqlite3.cpp
    wxsqlite3/sqlite3/secure/src/sqlite3secure.c)

TARGET_COMPILE_DEFINITIONS(wxSQLite3 PRIVATE
    NOPCH
    SQLITE_CORE
    SQLITE_ENABLE_FTS3
    SQLITE_ENABLE_FTS3_PARENTHESIS
    SQLITE_HAS_CODEC
    SQLITE_ENABLE_EXTFUNC
    SQLITE_ENABLE_COLUMN_METADATA
    SQLITE_ENABLE_JSON1
    HAVE_ACOSH
    HAVE_ASINH
    HAVE_ATANH
    HAVE_ISBLANK
    WXSQLITE3_HAVE_CODEC
    WXSQLITE3_HAVE_METADATA
    WXSQLITE3_USER_AUTHENTICATION)

IF(NOT WIN32)
    TARGET_COMPILE_DEFINITIONS(LUA PRIVATE
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
    TARGET_LINK_LIBRARIES(wxSQLite3 ${CMAKE_DL_LIBS})
    TARGET_LINK_LIBRARIES(LUA ${CMAKE_DL_LIBS})
ENDIF()

IF(MMEX_BUILD_TESTS)
    INCLUDE(ExternalProject)
    IF(MSVC)
        ExternalProject_ADD(CPPUnit
            SOURCE_DIR "${CMAKE_SOURCE_DIR}/lib/cppunit"
            CONFIGURE_COMMAND devenv /upgrade Build2010.sln
            # TODO: CMAKE_BUILD_TYPE is not set...
            BUILD_COMMAND msbuild Build2010.sln /m /p:Configuration=${CMAKE_BUILD_TYPE} /target:cppunit
            INSTALL_COMMAND ""
            BUILD_IN_SOURCE 1)
    ELSE(MSVC)
        ExternalProject_ADD(CPPUnit
            SOURCE_DIR "${CMAKE_SOURCE_DIR}/lib/cppunit"
            CONFIGURE_COMMAND sh autogen.sh && ./configure -q --disable-shared --disable-doxygen --enable-silent-rules "--prefix=${CMAKE_BINARY_DIR}/cppunit"
            INSTALL_COMMAND make install-exec
            BUILD_IN_SOURCE 1)
    ENDIF(MSVC)

    ADD_LIBRARY(CPPUnitLib STATIC IMPORTED GLOBAL)
    ADD_DEPENDENCIES(CPPUnitLib CPPUnit)
    SET_TARGET_PROPERTIES(CPPUnitLib PROPERTIES
        IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/cppunit/lib/${CMAKE_STATIC_LIBRARY_PREFIX}cppunit${CMAKE_STATIC_LIBRARY_SUFFIX}"
        IMPORTED_LOCATION_RELEASE "${CMAKE_BINARY_DIR}/cppunit/lib/${CMAKE_STATIC_LIBRARY_PREFIX}cppunit${CMAKE_STATIC_LIBRARY_SUFFIX}"
        IMPORTED_LOCATION_DEBUG "${CMAKE_BINARY_DIR}/cppunit/lib/${CMAKE_STATIC_LIBRARY_PREFIX}cppunit${CMAKE_DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}"
        #IMPORTED_IMPLIB "${CMAKE_BINARY_DIR}/cppunit/lib/${CMAKE_STATIC_LIBRARY_PREFIX}cppunit${CMAKE_STATIC_LIBRARY_SUFFIX}"
        IMPORTED_LINK_INTERFACE_LANGUAGES CXX)
ENDIF(MMEX_BUILD_TESTS)
