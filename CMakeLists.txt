CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

SET(InMMEX TRUE)

PROJECT(MoneyManagerEx VERSION 1.3.0)

OPTION(BuildTests "Whether to build automatic unit tests" OFF)

MESSAGE("-- Build Type = ${CMAKE_BUILD_TYPE}")
MESSAGE("-- Building Tests = ${BuildTests}")

#Set compiler parameters
SET(CMAKE_CXX_STANDARD 11)
#MSVC
IF (MSVC)
	MESSAGE("-- Compiler = MSVC")
	#TODO:

#GCC or Clang
ELSEIF (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
	IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		MESSAGE("-- Compiler = GNU")
		#TODO:
	ELSE()
		MESSAGE("-- Compiler = Clang")
		#TODO:
	ENDIF()
ENDIF ()


#Set various location
SET(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")
SET(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/lib")

SET(INSTALL_BIN_DIR "bin")
SET(INSTALL_DOC_DIR "share/doc/mmex")
SET(INSTALL_RES_DIR "share/mmex/res")
SET(INSTALL_MO_DIR "share/mmex/po")

#Check for required libraries
FIND_PACKAGE(Gettext REQUIRED)
#wxScintilla is needed on Windows
IF (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    FIND_PACKAGE(wxWidgets REQUIRED core qa net html xml aui adv scintilla stc webview base REQUIRED)
ELSE()
    FIND_PACKAGE(wxWidgets REQUIRED core qa net html xml aui adv stc webview base REQUIRED)
ENDIF()
INCLUDE(${wxWidgets_USE_FILE})

MESSAGE("-- Base Directory =  ${CMAKE_SOURCE_DIR}")

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

## Install ##

#OSX
IF (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	#TODO:
ENDIF ()
#Linux
IF (CMAKE_SYSTEM_NAME STREQUAL "Linux")

	#Binary	
	INSTALL(PROGRAMS "${CMAKE_SOURCE_DIR}/bin/mmex" 
		DESTINATION bin)
	
	#Help Files
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/doc/contrib.txt"
		"${CMAKE_SOURCE_DIR}/doc/license.txt"
		"${CMAKE_SOURCE_DIR}/doc/version.txt"
		DESTINATION "${INSTALL_DOC_DIR}")
	INSTALL(DIRECTORY "${CMAKE_SOURCE_DIR}/doc/help"
		DESTINATION "${INSTALL_DOC_DIR}")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/resources/master.css"
        	DESTINATION "${INSTALL_DOC_DIR}/help")

	#Resources
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/3rd/ChartNew.js/ChartNew.js"
		"${CMAKE_SOURCE_DIR}/3rd/sorttable.js/sorttable.js"
		"${CMAKE_SOURCE_DIR}/resources/home_page.htt"
		"${CMAKE_SOURCE_DIR}/resources/kaching.wav"
		"${CMAKE_SOURCE_DIR}/resources/master.css"
		"${CMAKE_SOURCE_DIR}/resources/mmex.ico"
		DESTINATION "${INSTALL_RES_DIR}/")

	#.desktop File
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/resources/mmex.desktop"
		DESTINATION "share/applications")

	#Icons
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/resources/mmex.svg"
		DESTINATION "share/icons/hicolor/scalable/apps")

ENDIF ()
#Windows
IF (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	#TODO:
ENDIF ()

## CPack ##
SET(CPACK_PACKAGE_NAME "mmex")
SET(CPACK_PACKAGE_VERSION "1.3.0")
SET(CPACK_PACKAGE_CONTACT "MoneyManagerEx Developers <developers@moneymanagerex.org>")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Simple to use financial management software")
SET(CPACK_PACKAGE_DESCRIPTION "Money Manager Ex (MMEX) is a free, open-source, cross-platform, easy-to-use personal finance software. It primarily helps organize one's finances and keeps track of where, when and how the money goes. MMEX includes all the basic features that 90% of users would want to see in a personal finance application. The design goals are to concentrate on simplicity and user friendliness - something one can use everyday.")
IF (CMAKE_SYSTEM_NAME STREQUAL "Linux")
	IF (EXISTS "/etc/debian_version")
		SET(CPACK_GENERATOR "DEB")
		SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.15), libgcc1 (>=1:4.1.1), libstdc++6 (>= 4.6), libwxgtk3.0-0v5 (>= 3.0.0) | libwxgtk3.0-0 (>=3.0.0), libwxgtk-webview3.0-0v5 (>= 3.0.0) | libwxgtk-webview3.0-0v5 (>= 3.0.0)")
	ELSEIF (EXISTS "/etc/redhat-release")
		SET(CPACK_GENERATOR "RPM")
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
ENDIF()


INCLUDE(CPack) 
	
