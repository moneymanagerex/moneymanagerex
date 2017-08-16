IF(NOT InMMEX)
    MESSAGE(FATAL_ERROR "Use the top-level CMake script!")
ENDIF(NOT InMMEX)

IF(WIN32)
    # TODO: update strings inside...
    SET(MMEX_RC mmex_tests.rc)
ENDIF()

ADD_EXECUTABLE(${MMEX_EXE}-tests WIN32 MACOSX_BUNDLE
    cpu_timer.cpp
    cpu_timer.h
    db_init_model.cpp
    db_init_model.h
    defined_test_selection.h
    framebase_tests.cpp
    framebase_tests.h
    mmex_tests.cpp
    mmex_tests.rc
    resource.h
    test_about_dialog.cpp
    test_about_dialog.h
    test_assets.cpp
    test_assets.h
    test_billsdeposits.cpp
    test_billsdeposits.h
    test_callback_hooks.cpp
    test_callback_hooks.h
    test_checking.cpp
    test_checking.h
    test_currency.cpp
    test_currency.h
    test_database_initialisation.cpp
    test_database_initialisation.h
    test_date_range.cpp
    test_date_range.h
    test_filtertrans_dialog.cpp
    test_filtertrans_dialog.h
    test_options_dialog.cpp
    test_options_dialog.h
    test_relocate_category.cpp
    test_relocate_category.h
    test_relocate_payee.cpp
    test_relocate_payee.h
    test_stocks.cpp
    test_stocks.h
    test_translink.cpp
    test_translink.h
    ${MMEX_RC}

    "${CMAKE_SOURCE_DIR}/src/mmex.cpp"
    "${CMAKE_SOURCE_DIR}/src/mmex.h"

    "${CMAKE_SOURCE_DIR}/3rd/cgitemplate/html_template.cpp"
    "${CMAKE_SOURCE_DIR}/3rd/mongoose/mongoose.c"
    "${MACOSX_APP_ICON_FILE}")

TARGET_COMPILE_DEFINITIONS(${MMEX_EXE}-tests PRIVATE wxIMPLEMENT_WXWIN_MAIN=)
TARGET_LINK_LIBRARIES(${MMEX_EXE}-tests MMEXLib CPPUnitLib ${wxWidgets_LIBRARIES} wxSQLite3 LUA)

# hide warnings from cppunit includes
IF(CMAKE_COMPILER_IS_GNUCXX)
    TARGET_COMPILE_OPTIONS(${MMEX_EXE}-tests PRIVATE -Wno-deprecated-declarations)
ENDIF()

INSTALL(TARGETS ${MMEX_EXE}-tests
    RUNTIME DESTINATION ${MMEX_BIN_DIR}
    BUNDLE  DESTINATION .)

MESSAGE(STATUS "Using cppunit lib: ${CMAKE_BINARY_DIR}/cppunit/lib/${CMAKE_STATIC_LIBRARY_PREFIX}cppunit${CMAKE_STATIC_LIBRARY_SUFFIX}")
