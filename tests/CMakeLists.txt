IF(NOT InMMEX)
    MESSAGE(FATAL_ERROR "Use the top-level CMake script!")
ENDIF(NOT InMMEX)

IF(WIN32)
    # TODO: update strings inside...
    SET(MMEX_RC mmex_tests.rc)
ENDIF()

# Pass version numbers to the sources
# TODO: find better way for this
CONFIGURE_FILE("${PROJECT_SOURCE_DIR}/src/versions.h.in" versions.h @ONLY)
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")

ADD_EXECUTABLE(${MMEX_EXE}-tests MACOSX_BUNDLE
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

    "${PROJECT_SOURCE_DIR}/src/mmex.cpp"
    "${PROJECT_SOURCE_DIR}/src/mmex.h"
    "${MACOSX_APP_ICON_FILE}"
    ${MMEX_RC})

TARGET_COMPILE_DEFINITIONS(${MMEX_EXE}-tests PRIVATE wxIMPLEMENT_WXWIN_MAIN=)
IF(MSVC)
    TARGET_COMPILE_OPTIONS(${MMEX_EXE}-tests PRIVATE /wd4005)
ENDIF()

TARGET_INCLUDE_DIRECTORIES(${MMEX_EXE}-tests PRIVATE .)
TARGET_LINK_LIBRARIES(${MMEX_EXE}-tests MMEXLib CPPUnitLib)

INSTALL(TARGETS ${MMEX_EXE}-tests
    RUNTIME DESTINATION ${MMEX_BIN_DIR}
    BUNDLE  DESTINATION .)
