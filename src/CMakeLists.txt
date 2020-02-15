IF(NOT InMMEX)
    MESSAGE(FATAL_ERROR "Use the top-level CMake script!")
ENDIF(NOT InMMEX)

# Pass version numbers to the sources
CONFIGURE_FILE(versions.h.in versions.h @ONLY)
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")

IF(APPLE)
    SET(MMEX_PLAT_DIR mac)
ELSEIF(UNIX)
    SET(MMEX_PLAT_DIR unix)
ELSEIF (WIN32)
    SET(MMEX_PLAT_DIR win)
    CONFIGURE_FILE(
        "${PROJECT_SOURCE_DIR}/resources/mmex.rc.in"
        mmex.rc @ONLY)
    SET(MMEX_RC "${CMAKE_CURRENT_BINARY_DIR}/mmex.rc")
ENDIF ()

ADD_LIBRARY(MMEXLib STATIC EXCLUDE_FROM_ALL
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
    customfielddialog.cpp
    customfielddialog.h
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
    mmCalculator.cpp
    mmCalculator.h
    mmcheckingpanel.cpp
    mmcheckingpanel.h
    mmcombobox.h
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

    db/DB_Table_Accountlist_V1.h
    db/DB_Table_Assets_V1.h
    db/DB_Table_Attachment_V1.h
    db/DB_Table_Billsdeposits_V1.h
    db/DB_Table_Budgetsplittransactions_V1.h
    db/DB_Table_Budgettable_V1.h
    db/DB_Table_Budgetyear_V1.h
    db/DB_Table_Category_V1.h
    db/DB_Table_Checkingaccount_V1.h
    db/DB_Table_Currencyformats_V1.h
    db/DB_Table_Currencyhistory_V1.h
    db/DB_Table_Customfielddata_V1.h
    db/DB_Table_Customfield_V1.h
    db/DB_Table.h
    db/DB_Table_Infotable_V1.h
    db/DB_Table_Payee_V1.h
    db/DB_Table_Report_V1.h
    db/DB_Table_Setting_V1.h
    db/DB_Table_Shareinfo_V1.h
    db/DB_Table_Splittransactions_V1.h
    db/DB_Table_Stockhistory_V1.h
    db/DB_Table_Stock_V1.h
    db/DB_Table_Subcategory_V1.h
    db/DB_Table_Translink_V1.h
    db/DB_Table_Usage_V1.h

    reports/allreport.h
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

    model/allmodel.h
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
    "${MMEX_PLAT_DIR}/platfdep.cpp")

ADD_EXECUTABLE(${MMEX_EXE} WIN32 MACOSX_BUNDLE
    mmex.cpp
    mmex.h
    "${MACOSX_APP_ICON_FILE}"
    "${MMEX_RC}")

IF(CMAKE_VERSION VERSION_LESS 3.1)
    INCLUDE(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=gnu++11" COMPILER_SUPPORTS_GXX11)
    CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
    CHECK_CXX_COMPILER_FLAG("-std=gnu++0x" COMPILER_SUPPORTS_GXX0X)
    CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
    IF(COMPILER_SUPPORTS_GXX11)
        TARGET_COMPILE_OPTIONS(MMEXLib PUBLIC -std=gnu++11)
    ELSEIF(COMPILER_SUPPORTS_CXX11)
        TARGET_COMPILE_OPTIONS(MMEXLib PUBLIC -std=c++11)
    ELSEIF(COMPILER_SUPPORTS_GXX0X)
        TARGET_COMPILE_OPTIONS(MMEXLib PUBLIC -std=gnu++0x)
    ELSEIF(COMPILER_SUPPORTS_CXX0X)
        TARGET_COMPILE_OPTIONS(MMEXLib PUBLIC -std=c++0x)
    ELSE()
        MESSAGE(SEND_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support.")
    ENDIF()
ELSEIF(CMAKE_VERSION VERSION_LESS 3.8)
    TARGET_COMPILE_FEATURES(MMEXLib PUBLIC cxx_range_for)
ELSE()
    TARGET_COMPILE_FEATURES(MMEXLib PUBLIC cxx_std_11)
ENDIF()

TARGET_INCLUDE_DIRECTORIES(MMEXLib PUBLIC .)
TARGET_LINK_LIBRARIES(${MMEX_EXE} MMEXLib)
TARGET_LINK_LIBRARIES(MMEXLib
    PUBLIC
        wxSQLite3
        RapidJSON
        cajun
        HTML-template
        CURL::libcurl
    PRIVATE
        LuaGlue
        Lua
    )

IF(MSVC)
    # Based on this http://stackoverflow.com/a/8294669
    # added solution to compile problems due to
    # conflict between winsock and winsock2
    # Partialy reinvented fix from commit
    # commit 06accae1273e66ced469672151522e45eee685a9
    TARGET_COMPILE_DEFINITIONS(MMEXLib PRIVATE WIN32_LEAN_AND_MEAN)
ENDIF()

INSTALL(TARGETS ${MMEX_EXE}
    RUNTIME DESTINATION ${MMEX_BIN_DIR}
    BUNDLE  DESTINATION .)
