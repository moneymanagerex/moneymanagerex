CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

SET(InMMEX TRUE)

PROJECT(MoneyManagerEx)

OPTION(BuildTests "Whether to build automatic unit tests" OFF)

MESSAGE("-- Install Prefix = ${CMAKE_INSTALL_PREFIX}")
MESSAGE("-- Build Type = ${CMAKE_BUILD_TYPE}")
MESSAGE("-- Building Tests = ${BuildTests}")

#Set compiler parameters
SET(CMAKE_CXX_STANDARD 11)
#MSVC
IF (MSVC)
	MESSAGE("-- Compiler = MSVC")

#GCC or Clang
ELSEIF (("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU") OR ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang"))
	IF ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
		MESSAGE("-- Compiler = GNU")
	ELSE()
		MESSAGE("-- Compiler = Clang")
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
IF ((MSVC) OR (UNIX AND APPLE))
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
ADD_EXECUTABLE(mmex "")
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
IF (UNIX AND APPLE)

ENDIF ()
#*nix
IF (UNIX AND NOT APPLE)

	#Binary	
	INSTALL(PROGRAMS "${CMAKE_SOURCE_DIR}/bin/mmex" 
		DESTINATION bin)	
	
	#Help Files
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/doc/contrib.txt"
		"${CMAKE_SOURCE_DIR}/doc/license.txt"
		"${CMAKE_SOURCE_DIR}/doc/version.txt"
		DESTINATION "${INSTALL_DOC_DIR}")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/doc/help/backtotop.png"
		"${CMAKE_SOURCE_DIR}/doc/help/btn_donateCC_LG.gif"
		"${CMAKE_SOURCE_DIR}/doc/help/budget.html"
		"${CMAKE_SOURCE_DIR}/doc/help/budget_editor.png"
		"${CMAKE_SOURCE_DIR}/doc/help/budget_entry_details.png"
		"${CMAKE_SOURCE_DIR}/doc/help/budget_grid.png"
		"${CMAKE_SOURCE_DIR}/doc/help/contents.js"
		"${CMAKE_SOURCE_DIR}/doc/help/general_report_manager.html"
		"${CMAKE_SOURCE_DIR}/doc/help/GRM.gif"
		"${CMAKE_SOURCE_DIR}/doc/help/index.html"
		"${CMAKE_SOURCE_DIR}/doc/help/investment.html"
		"${CMAKE_SOURCE_DIR}/doc/help/mmex_concept.png"
		"${CMAKE_SOURCE_DIR}/doc/help/mmex_db_structure.png"
		"${CMAKE_SOURCE_DIR}/doc/help/stock_dialog.png"
		"${CMAKE_SOURCE_DIR}/doc/help/stock_options.png"
		"${CMAKE_SOURCE_DIR}/doc/help/stock_view.png"
		"${CMAKE_SOURCE_DIR}/resources/master.css"
		DESTINATION "${INSTALL_DOC_DIR}/help")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/doc/help/french/account.jpg"
		"${CMAKE_SOURCE_DIR}/doc/help/french/add_acc_wizard1.jpg"
		"${CMAKE_SOURCE_DIR}/doc/help/french/add_acc_wizard2.jpg"
		"${CMAKE_SOURCE_DIR}/doc/help/french/add_acc_wizard3.jpg"
		"${CMAKE_SOURCE_DIR}/doc/help/french/index.html"
		"${CMAKE_SOURCE_DIR}/doc/help/french/investment.html"
		"${CMAKE_SOURCE_DIR}/doc/help/french/main_screen.jpg"
		"${CMAKE_SOURCE_DIR}/doc/help/french/mmex_concept.png"
		DESTINATION "${INSTALL_DOC_DIR}/help/french")
	INSTALL (FILES "${CMAKE_SOURCE_DIR}/doc/help/hungarian/budget.html"
		"${CMAKE_SOURCE_DIR}/doc/help/hungarian/index.html"
		"${CMAKE_SOURCE_DIR}/doc/help/hungarian/investment.html"
		"${CMAKE_SOURCE_DIR}/doc/help/hungarian/mmex_concept_hu.png"
		DESTINATION "${INSTALL_DOC_DIR}/help/hungarian")
	INSTALL (FILES "${CMAKE_SOURCE_DIR}/doc/help/italian/index.html"
		"${CMAKE_SOURCE_DIR}/doc/help/italian/investment.html"
		"${CMAKE_SOURCE_DIR}/doc/help/italian/mmex_concept.png"
		"${CMAKE_SOURCE_DIR}/doc/help/italian/stock_dialog.jpg"
		"${CMAKE_SOURCE_DIR}/doc/help/italian/stock_options.jpg"
		"${CMAKE_SOURCE_DIR}/doc/help/italian/stock_view.jpg"
		DESTINATION "${INSTALL_DOC_DIR}/help/italian")
	INSTALL (FILES "${CMAKE_SOURCE_DIR}/doc/help/polish/btn_donateCC_LG.gif"
		"${CMAKE_SOURCE_DIR}/doc/help/polish/budget.html"
		"${CMAKE_SOURCE_DIR}/doc/help/polish/budget_editor.png"
		"${CMAKE_SOURCE_DIR}/doc/help/polish/budget_entry_details.png"
		"${CMAKE_SOURCE_DIR}/doc/help/polish/budget_grid.png"
		"${CMAKE_SOURCE_DIR}/doc/help/polish/general_report_manager.html"
		"${CMAKE_SOURCE_DIR}/doc/help/polish/GRM.gif"
		"${CMAKE_SOURCE_DIR}/doc/help/polish/index.html"
		"${CMAKE_SOURCE_DIR}/doc/help/polish/investment.html"
		"${CMAKE_SOURCE_DIR}/doc/help/polish/mmex_concept.png"
		"${CMAKE_SOURCE_DIR}/doc/help/polish/mmex_structure.jpg"
		"${CMAKE_SOURCE_DIR}/doc/help/polish/stock_dialog.jpg"
		"${CMAKE_SOURCE_DIR}/doc/help/polish/stock_options.jpg"
		"${CMAKE_SOURCE_DIR}/doc/help/polish/stock_view.jpg"
		DESTINATION "${INSTALL_DOC_DIR}/help/polish")
	INSTALL (FILES "${CMAKE_SOURCE_DIR}/doc/help/russian/index.html"
		"${CMAKE_SOURCE_DIR}/doc/help/russian/investment.html"
		"${CMAKE_SOURCE_DIR}/doc/help/russian/mmex_concept.png"
		"${CMAKE_SOURCE_DIR}/doc/help/russian/stock_dialog.png"
		DESTINATION "${INSTALL_DOC_DIR}/help/russian")
	INSTALL (FILES "${CMAKE_SOURCE_DIR}/doc/help/spanish/budget.html"
		"${CMAKE_SOURCE_DIR}/doc/help/spanish/index.html"
		"${CMAKE_SOURCE_DIR}/doc/help/spanish/investment.html"
		DESTINATION "${INSTALL_DOC_DIR}/help/spanish")

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
IF (WIN32)
ENDIF ()


