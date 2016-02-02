IF(NOT InMMEX)
	MESSAGE(FATAL_ERROR "Use the top-level CMake script!")
ENDIF(NOT InMMEX)

SET(LUA_SOURCES 
	"lua/src/lapi.c"
	"lua/src/lauxlib.c"
	"lua/src/lbaselib.c"
	"lua/src/lbitlib.c"
	"lua/src/lcode.c"
	"lua/src/lcorolib.c"
	"lua/src/lctype.c"
	"lua/src/ldblib.c"
	"lua/src/ldebug.c"
	"lua/src/ldo.c"
	"lua/src/ldump.c"
	"lua/src/lfunc.c"
	"lua/src/lgc.c"
	"lua/src/linit.c"
	"lua/src/liolib.c"
	"lua/src/llex.c"
	"lua/src/lmathlib.c"
	"lua/src/lmem.c"
	"lua/src/loadlib.c"
	"lua/src/lobject.c"
	"lua/src/lopcodes.c"
	"lua/src/loslib.c"
	"lua/src/lparser.c"
	"lua/src/lstate.c"
	"lua/src/lstring.c"
	"lua/src/lstrlib.c"
	"lua/src/ltable.c"
	"lua/src/ltablib.c"
	"lua/src/ltm.c"
	"lua/src/lua.c"
	"lua/src/luac.c"
	"lua/src/lundump.c"
	"lua/src/lvm.c"
	"lua/src/lzio.c"
)

SET(WXSQLITE3_SOURCES
	"wxsqlite3/src/wxsqlite3.cpp"
	"wxsqlite3/sqlite3/secure/src/sqlite3secure.c"
)

ADD_LIBRARY(lua STATIC ${LUA_SOURCES})
ADD_LIBRARY(wxsqlite3 STATIC ${WXSQLITE3_SOURCES})
TARGET_LINK_LIBRARIES(wxsqlite3 dl)

