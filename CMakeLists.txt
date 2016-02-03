CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(InMMEX TRUE)

PROJECT(MoneyManagerEx)

OPTION(BuildTests "Whether to build automatic unit tests" OFF)
OPTION(BuildPortableVersion "Build portable version (does not read or write files from your user/home directory" OFF)

MESSAGE("-- Install Prefix = ${CMAKE_INSTALL_PREFIX}")

#Set compiler parameters
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
	SET(CMAKE_C_PARAMS "-O2")
	SET(CMAKE_C_WARN_FLAGS "")
	SET(CMAKE_C_FLAGS "${CMAKE_C_PARAMS} ${CMAKE_C_WARN_FLAGS}")
	SET(CMAKE_CXX_PARAMS "-std=c++11 -O2")
	SET(CMAKE_CXX_WARN_FLAGS "")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_PARAMS} ${CMAKE_CXX_WARN_FLAGS}")
ENDIF ()

SET(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")
SET(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/lib")

#Check for required libraries
FIND_PACKAGE(Gettext REQUIRED)
FIND_PACKAGE(wxWidgets REQUIRED core qa net html xml aui adv stc webview base REQUIRED)
INCLUDE(${wxWidgets_USE_FILE})

MESSAGE("-- Base Directory =  ${CMAKE_SOURCE_DIR}")

INCLUDE_DIRECTORIES(
	${wxWidgets_INCLUDE_DIRS}
	"src"
	"3rd"
	"3rd/csv-parser/include"
	"3rd/LuaGlue/include"
	"lib/lua/src"
	"lib/wxsqlite3/include"
	"lib/wxsqlite3/sqlite3/secure/src/"
)

ADD_SUBDIRECTORY(po)
ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(src)

IF(BuildTests)
	ADD_SUBDIRECTORY(tests)
ENDIF(BuildTests)

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
		DESTINATION "share/doc/mmex")
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
		DESTINATION "share/doc/mmex/help")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/doc/help/french/account.jpg"
		"${CMAKE_SOURCE_DIR}/doc/help/french/add_acc_wizard1.jpg"
		"${CMAKE_SOURCE_DIR}/doc/help/french/add_acc_wizard2.jpg"
		"${CMAKE_SOURCE_DIR}/doc/help/french/add_acc_wizard3.jpg"
		"${CMAKE_SOURCE_DIR}/doc/help/french/index.html"
		"${CMAKE_SOURCE_DIR}/doc/help/french/investment.html"
		"${CMAKE_SOURCE_DIR}/doc/help/french/main_screen.jpg"
		"${CMAKE_SOURCE_DIR}/doc/help/french/mmex_concept.png"
		DESTINATION "share/doc/mmex/help/french")
	INSTALL (FILES "${CMAKE_SOURCE_DIR}/doc/help/hungarian/budget.html"
		"${CMAKE_SOURCE_DIR}/doc/help/hungarian/index.html"
		"${CMAKE_SOURCE_DIR}/doc/help/hungarian/investment.html"
		"${CMAKE_SOURCE_DIR}/doc/help/hungarian/mmex_concept_hu.png"
		DESTINATION "share/doc/mmex/help/hungarian")
	INSTALL (FILES "${CMAKE_SOURCE_DIR}/doc/help/italian/index.html"
		"${CMAKE_SOURCE_DIR}/doc/help/italian/investment.html"
		"${CMAKE_SOURCE_DIR}/doc/help/italian/mmex_concept.png"
		"${CMAKE_SOURCE_DIR}/doc/help/italian/stock_dialog.jpg"
		"${CMAKE_SOURCE_DIR}/doc/help/italian/stock_options.jpg"
		"${CMAKE_SOURCE_DIR}/doc/help/italian/stock_view.jpg"
		DESTINATION "share/doc/mmex/help/italian")
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
		DESTINATION "share/doc/mmex/help/polish")
	INSTALL (FILES "${CMAKE_SOURCE_DIR}/doc/help/russian/index.html"
		"${CMAKE_SOURCE_DIR}/doc/help/russian/investment.html"
		"${CMAKE_SOURCE_DIR}/doc/help/russian/mmex_concept.png"
		"${CMAKE_SOURCE_DIR}/doc/help/russian/stock_dialog.png"
		DESTINATION "share/doc/mmex/help/russian")
	INSTALL (FILES "${CMAKE_SOURCE_DIR}/doc/help/spanish/budget.html"
		"${CMAKE_SOURCE_DIR}/doc/help/spanish/index.html"
		"${CMAKE_SOURCE_DIR}/doc/help/spanish/investment.html"
		DESTINATION "share/doc/mmex/help/spanish")

	#Resources
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/3rd/ChartNew.js/ChartNew.js"
		"${CMAKE_SOURCE_DIR}/3rd/sorttable.js/sorttable.js"
		"${CMAKE_SOURCE_DIR}/resources/home_page.htt"
		"${CMAKE_SOURCE_DIR}/resources/kaching.wav"
		"${CMAKE_SOURCE_DIR}/resources/master.css"
		"${CMAKE_SOURCE_DIR}/resources/mmex.ico"
		DESTINATION "share/mmex/res")

	#.desktop File
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/resources/mmex.desktop"
		DESTINATION "share/applications")

	#Icons
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/resources/mmex.svg"
		DESTINATION "share/icons/hicolor/scalable/apps")

	#PO Files
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/arabic.gmo"
		DESTINATION "share/mmex/po"
		RENAME "arabic.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/bulgarian.gmo"
		DESTINATION "share/mmex/po"
		RENAME "bulgarian.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/chinese_chs.gmo"
		DESTINATION "share/mmex/po"
		RENAME "chinese_chs.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/chinese_zh.gmo"
		DESTINATION "share/mmex/po"
		RENAME "chinese_zh.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/croatian.gmo"
		DESTINATION "share/mmex/po"
		RENAME "croation.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/czech.gmo"
		DESTINATION "share/mmex/po"
		RENAME "czech.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/dutch.gmo"
		DESTINATION "share/mmex/po"
		RENAME "dutch.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/dutch_be.gmo"
		DESTINATION "share/mmex/po"
		RENAME "dutch_be.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/english.gmo"
		DESTINATION "share/mmex/po"
		RENAME "english.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/english-uk.gmo"
		DESTINATION "share/mmex/po"
		RENAME "english-uk.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/french.gmo"
		DESTINATION "share/mmex/po"
		RENAME "french.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/german.gmo"
		DESTINATION "share/mmex/po"
		RENAME "german.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/greek.gmo"
		DESTINATION "share/mmex/po"
		RENAME "greek.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/hebrew.gmo"
		DESTINATION "share/mmex/po"
		RENAME "hebrew.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/hindi.gmo"
		DESTINATION "share/mmex/po"
		RENAME "hindi.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/hungarian.gmo"
		DESTINATION "share/mmex/po"
		RENAME "hungarian.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/indonesian.gmo"
		DESTINATION "share/mmex/po"
		RENAME "indonesian.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/italian.gmo"
		DESTINATION "share/mmex/po"
		RENAME "italian.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/japanese.gmo"
		DESTINATION "share/mmex/po"
		RENAME "japanese.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/latvian.gmo"
		DESTINATION "share/mmex/po"
		RENAME "latvian.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/lithuanian.gmo"
		DESTINATION "share/mmex/po"
		RENAME "lithuanian.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/norwegian.gmo"
		DESTINATION "share/mmex/po"
		RENAME "norwegian.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/polish.gmo"
		DESTINATION "share/mmex/po"
		RENAME "polish.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/portuguese.gmo"
		DESTINATION "share/mmex/po"
		RENAME "portuguese.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/portuguese_br.gmo"
		DESTINATION "share/mmex/po"
		RENAME "portuguese_br.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/portuguese_portugal.gmo"
		DESTINATION "share/mmex/po"
		RENAME "portuguese_portugal.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/romanian.gmo"
		DESTINATION "share/mmex/po"
		RENAME "romanian.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/russian.gmo"
		DESTINATION "share/mmex/po"
		RENAME "russian.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/serbian.gmo"
		DESTINATION "share/mmex/po"
		RENAME "serbian.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/serbo-croatian.gmo"
		DESTINATION "share/mmex/po"
		RENAME "serbo-croatian.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/slovak.gmo"
		DESTINATION "share/mmex/po"
		RENAME "slovak.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/slovenian.gmo"
		DESTINATION "share/mmex/po"
		RENAME "slovenian.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/spanish.gmo"
		DESTINATION "share/mmex/po"
		RENAME "spanish.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/swedish.gmo"
		DESTINATION "share/mmex/po"
		RENAME "swedish.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/tamil.gmo"
		DESTINATION "share/mmex/po"
		RENAME "tamil.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/turkish.gmo"
		DESTINATION "share/mmex/po"
		RENAME "turkish.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/ukrainian.gmo"
		DESTINATION "share/mmex/po"
		RENAME "ukrainian.mo")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/po/vietnamese.gmo"
		DESTINATION "share/mmex/po"
		RENAME "vietnamese.mo")
ENDIF ()
#Windows
IF (WIN32)
ENDIF ()


