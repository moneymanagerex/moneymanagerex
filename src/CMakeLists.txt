IF(NOT InMMEX)
    MESSAGE(FATAL_ERROR "Use the top-level CMake script!")
ENDIF(NOT InMMEX)

# enable warnings while compile sources in src
IF(MSVC)
    STRING(REGEX REPLACE "(^| )[/-](w|W[0-4])( |$)" " " CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    STRING(REGEX REPLACE "(^| )[/-](w|W[0-4])( |$)" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
ELSE()
    ADD_COMPILE_OPTIONS("-Wall" "-Wextra" "-Wno-unknown-pragmas")
ENDIF()


# Pass version numbers to the sources
CONFIGURE_FILE(versions.h.in versions.h @ONLY)
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")

IF(APPLE)
    SET(MMEX_PLATFDEP mac)
ELSEIF(UNIX)
    SET(MMEX_PLATFDEP unix)
ELSEIF (WIN32)
    SET(MMEX_PLATFDEP win)
    CONFIGURE_FILE(
        "${PROJECT_SOURCE_DIR}/resources/mmex.rc.in"
        mmex.rc @ONLY)
    SET(MMEX_RC "${CMAKE_CURRENT_BINARY_DIR}/mmex.rc")
ENDIF ()

ADD_EXECUTABLE(${MMEX_EXE} WIN32 MACOSX_BUNDLE
    aboutdialog.cpp
    aboutdialog.h
    accountdialog.cpp
    accountdialog.h
    appstartdialog.cpp
    appstartdialog.h
    assetdialog.cpp
    assetdialog.h
    assetspanel.cpp
    assetspanel.h
    attachmentdialog.cpp
    attachmentdialog.h
    billsdepositsdialog.cpp
    billsdepositsdialog.h
    billsdepositspanel.cpp
    billsdepositspanel.h
    budgetentrydialog.cpp
    budgetentrydialog.h
    budgetingpanel.cpp
    budgetingpanel.h
    budgetyeardialog.cpp
    budgetyeardialog.h
    budgetyearentrydialog.cpp
    budgetyearentrydialog.h
    categdialog.cpp
    categdialog.h
    constants.cpp
    constants.h
    currencydialog.cpp
    currencydialog.h
    customfieldeditdialog.cpp
    customfieldeditdialog.h
    customfieldlistdialog.cpp
    customfieldlistdialog.h
    dbcheck.cpp
    dbcheck.h
    dbupgrade.cpp
    dbupgrade.h
    dbwrapper.cpp
    dbwrapper.h
    defs.h
    filtertransdialog.cpp
    filtertransdialog.h
    general_report_manager.cpp
    general_report_manager.h
    images_list.cpp
    images_list.h
    maincurrencydialog.cpp
    maincurrencydialog.h
    minimal_editor.cpp
    minimal_editor.h
    mmcheckingpanel.cpp
    mmcheckingpanel.h
    mmcombobox.h
    mmcustomdata.cpp
    mmcustomdata.h
    mmex.cpp
    mmex.h
    mmframe.cpp
    mmframe.h
    mmframereport.cpp
    mmhelppanel.cpp
    mmhelppanel.h
    mmhomepagepanel.cpp
    mmhomepagepanel.h
    mmHook.h
    mmpanelbase.cpp
    mmpanelbase.h
    mmreportspanel.cpp
    mmreportspanel.h
    mmSimpleDialogs.cpp
    mmSimpleDialogs.h
    mmTextCtrl.cpp
    mmTextCtrl.h
    mmTips.h
    option.cpp
    option.h
    optiondialog.cpp
    optiondialog.h
    optionsettingsattachment.cpp
    optionsettingsattachment.h
    optionsettingsbase.cpp
    optionsettingsbase.h
    optionsettingsgeneral.cpp
    optionsettingsgeneral.h
    optionsettingsmisc.cpp
    optionsettingsmisc.h
    optionsettingsnet.cpp
    optionsettingsnet.h
    optionsettingsview.cpp
    optionsettingsview.h
    paths.cpp
    paths.h
    payeedialog.cpp
    payeedialog.h
    platfdep.h
    recentfiles.cpp
    recentfiles.h
    relocatecategorydialog.cpp
    relocatecategorydialog.h
    relocatepayeedialog.cpp
    relocatepayeedialog.h
    resource.h
    singleton.h
    sharetransactiondialog.cpp
    sharetransactiondialog.h
    splitdetailsdialog.cpp
    splitdetailsdialog.h
    splittransactionsdialog.cpp
    splittransactionsdialog.h
    stockdialog.cpp
    stockdialog.h
    stockspanel.cpp
    stockspanel.h
    transactionsupdatedialog.h
    transactionsupdatedialog.cpp
    transdialog.cpp
    transdialog.h
    usertransactionpanel.cpp
    usertransactionpanel.h
    util.cpp
    util.h
    validators.h
    webapp.cpp
    webappdialog.cpp
    webappdialog.h
    webapp.h
    wizard_newaccount.cpp
    wizard_newaccount.h
    wizard_newdb.cpp
    wizard_newdb.h
    wizard_update.cpp
    wizard_update.h

    import_export/export.cpp
    import_export/export.h
    import_export/parsers.cpp
    import_export/parsers.h
    import_export/qif_export.cpp
    import_export/qif_export.h
    import_export/qif_import.cpp
    import_export/qif_import_gui.cpp
    import_export/qif_import_gui.h
    import_export/qif_import.h
    import_export/univcsvdialog.cpp
    import_export/univcsvdialog.h

    db/Table_Accountlist.h
    db/Table_Assets.h
    db/Table_Attachment.h
    db/Table_Billsdeposits.h
    db/Table_Budgetsplittransactions.h
    db/Table_Budgettable.h
    db/Table_Budgetyear.h
    db/Table_Category.h
    db/Table_Checkingaccount.h
    db/Table_Currencyformats.h
    db/Table_Currencyhistory.h
    db/Table_Customfielddata.h
    db/Table_Customfield.h
    db/Table.h
    db/Table_Infotable.h
    db/Table_Payee.h
    db/Table_Report.h
    db/Table_Setting_V1.h
    db/Table_Shareinfo.h
    db/Table_Splittransactions.h
    db/Table_Stockhistory.h
    db/Table_Stock.h
    db/Table_Subcategory.h
    db/Table_Translink.h
    db/Table_Usage_V1.h
    db/DB_Upgrade.h

    reports/budgetcategorysummary.cpp
    reports/budgetcategorysummary.h
    reports/budget.cpp
    reports/budget.h
    reports/budgetingperf.cpp
    reports/budgetingperf.h
    reports/cashflow.cpp
    reports/cashflow.h
    reports/categexp.cpp
    reports/categexp.h
    reports/categovertimeperf.cpp
    reports/categovertimeperf.h
    reports/forecast.cpp
    reports/forecast.h
    reports/htmlbuilder.cpp
    reports/htmlbuilder.h
    reports/incexpenses.cpp
    reports/incexpenses.h
    reports/mmDateRange.cpp
    reports/mmDateRange.h
    reports/myusage.cpp
    reports/myusage.h
    reports/payee.cpp
    reports/payee.h
    reports/reportbase.cpp
    reports/reportbase.h
    reports/summary.cpp
    reports/summary.h
    reports/summarystocks.cpp
    reports/summarystocks.h
    reports/transactions.cpp
    reports/transactions.h

    model/Model_Account.cpp
    model/Model_Account.h
    model/Model_Asset.cpp
    model/Model_Asset.h
    model/Model_Attachment.cpp
    model/Model_Attachment.h
    model/Model_Billsdeposits.cpp
    model/Model_Billsdeposits.h
    model/Model_Budget.cpp
    model/Model_Budget.h
    model/Model_Budgetsplittransaction.cpp
    model/Model_Budgetsplittransaction.h
    model/Model_Budgetyear.cpp
    model/Model_Budgetyear.h
    model/Model_Category.cpp
    model/Model_Category.h
    model/Model_Checking.cpp
    model/Model_Checking.h
    model/Model_Currency.cpp
    model/Model_Currency.h
    model/Model_CurrencyHistory.cpp
    model/Model_CurrencyHistory.h
    model/Model_CustomField.cpp
    model/Model_CustomFieldData.cpp
    model/Model_CustomFieldData.h
    model/Model_CustomField.h
    model/Model.h
    model/Model_Infotable.cpp
    model/Model_Infotable.h
    model/Model_Payee.cpp
    model/Model_Payee.h
    model/Model_Report.cpp
    model/Model_Report.h
    model/Model_Setting.cpp
    model/Model_Setting.h
    model/Model_Shareinfo.cpp
    model/Model_Shareinfo.h
    model/Model_Splittransaction.cpp
    model/Model_Splittransaction.h
    model/Model_Stock.cpp
    model/Model_Stock.h
    model/Model_StockHistory.cpp
    model/Model_StockHistory.h
    model/Model_Subcategory.cpp
    model/Model_Subcategory.h
    model/Model_Translink.cpp
    model/Model_Translink.h
    model/Model_Usage.cpp
    model/Model_Usage.h

    "${CMAKE_CURRENT_BINARY_DIR}/versions.h"
    "platfdep_${MMEX_PLATFDEP}.cpp"
    "${MACOSX_APP_ICON_FILE}"
    "${MMEX_RC}")

IF(MMEX_WEBSERVER)
    TARGET_SOURCES(${MMEX_EXE} PRIVATE
        webserver.cpp
        webserver.h)
    SET_SOURCE_FILES_PROPERTIES(
        constants.cpp
        mmex.cpp
        optiondialog.cpp
        optionsettingsnet.cpp
        optionsettingsnet.h
        PROPERTIES COMPILE_DEFINITIONS MMEX_WEBSERVER)
    TARGET_LINK_LIBRARIES(${MMEX_EXE} PRIVATE Mongoose)
ENDIF()

IF(";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";cxx_std_11;")
    TARGET_COMPILE_FEATURES(${MMEX_EXE} PUBLIC cxx_std_11)
ELSEIF(";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";cxx_range_for;"
       AND ";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";cxx_nullptr;")
    TARGET_COMPILE_FEATURES(${MMEX_EXE} PUBLIC cxx_range_for cxx_nullptr)
ELSE()
    INCLUDE(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=gnu++11" COMPILER_SUPPORTS_GXX11)
    CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
    CHECK_CXX_COMPILER_FLAG("-std=gnu++0x" COMPILER_SUPPORTS_GXX0X)
    CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
    IF(COMPILER_SUPPORTS_GXX11)
        TARGET_COMPILE_OPTIONS(${MMEX_EXE} PUBLIC -std=gnu++11)
    ELSEIF(COMPILER_SUPPORTS_CXX11)
        TARGET_COMPILE_OPTIONS(${MMEX_EXE} PUBLIC -std=c++11)
    ELSEIF(COMPILER_SUPPORTS_GXX0X)
        TARGET_COMPILE_OPTIONS(${MMEX_EXE} PUBLIC -std=gnu++0x)
    ELSEIF(COMPILER_SUPPORTS_CXX0X)
        TARGET_COMPILE_OPTIONS(${MMEX_EXE} PUBLIC -std=c++0x)
    ELSE()
        MESSAGE(SEND_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support.")
    ENDIF()
ENDIF()

TARGET_INCLUDE_DIRECTORIES(${MMEX_EXE} PUBLIC . model db)
TARGET_LINK_LIBRARIES(${MMEX_EXE} PUBLIC
    wxSQLite3
    RapidJSON
    HTML-template
    CURL::libcurl
    LuaGlue
    Lua)

IF(MSVC)
    # Based on this http://stackoverflow.com/a/8294669
    # added solution to compile problems due to
    # conflict between winsock and winsock2
    # Partially reinvented fix from commit
    # commit 06accae1273e66ced469672151522e45eee685a9
    TARGET_COMPILE_DEFINITIONS(${MMEX_EXE} PRIVATE WIN32_LEAN_AND_MEAN)
ENDIF()

INSTALL(TARGETS ${MMEX_EXE}
    RUNTIME DESTINATION ${MMEX_BIN_DIR}
    BUNDLE  DESTINATION .)
