get_directory_property(m_hasParent PARENT_DIRECTORY)
if(NOT m_hasParent)
    message(FATAL_ERROR "Use the top-level CMake script!")
endif()
unset(m_hasParent)

include(CheckCXXCompilerFlag)

# enable warnings while compile sources in src
if(MSVC)
    string(REGEX REPLACE "(^| )[/-](w|W[0-4])( |$)" " " CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string(REGEX REPLACE "(^| )[/-](w|W[0-4])( |$)" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    check_cxx_compiler_flag(-Wduplicated-cond Wduplicated-cond)
    check_cxx_compiler_flag(-Wduplicated-branches Wduplicated-branches)
    check_cxx_compiler_flag(-Wnull-dereference Wnull-dereference)
    check_cxx_compiler_flag(-Wlogical-op Wlogical-op)
    add_compile_options(-Wall -Wextra -Wno-unknown-pragmas -Wformat=2
        $<$<BOOL:${Wduplicated-cond}>:-Wduplicated-cond>
        $<$<BOOL:${Wduplicated-branches}>:-Wduplicated-branches>
        $<$<BOOL:${Wnull-dereference}>:-Wnull-dereference>
        $<$<BOOL:${Wlogical-op}>:-Wlogical-op>)
    if(NOT (CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6))
        check_cxx_compiler_flag(-Wuseless-cast Wuseless-cast)
        add_compile_options(-Werror=old-style-cast
            $<$<BOOL:${Wuseless-cast}>:-Werror=useless-cast>)
    endif()
endif()

if(MSVC)
    check_cxx_compiler_flag(/utf-8 MSVC_UFT8_CXX_FLAG)
    if(MSVC_UFT8_CXX_FLAG)
        # use UTF-8 for /source-charset and /execution-charset
        add_compile_options(/utf-8)
    else()
        # use old pragma execution_character_set
        file(GENERATE
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/msvc_build_header_$<CONFIG>.h
            CONTENT "#pragma execution_character_set(\"utf-8\")"
        )
        add_compile_options(/FI"${CMAKE_CURRENT_BINARY_DIR}/msvc_build_header_$<CONFIG>.h")
    endif()
endif()

# Pass version numbers to the sources
configure_file(versions.h.in versions.h @ONLY)
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

if(APPLE)
    set(MMEX_PLATFDEP mac.mm)
elseif(UNIX)
    set(MMEX_PLATFDEP unix.cpp)
elseif(WIN32)
    set(MMEX_PLATFDEP win.cpp)
    configure_file(
        "${PROJECT_SOURCE_DIR}/resources/mmex.rc.in"
        mmex.rc @ONLY)
    set(MMEX_RC "${CMAKE_CURRENT_BINARY_DIR}/mmex.rc")
endif()

add_executable(${MMEX_EXE} WIN32 MACOSX_BUNDLE
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
    build.h
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
    daterange2.cpp
    daterange2.h
    dbcheck.cpp
    dbcheck.h
    dbupgrade.cpp
    dbupgrade.h
    dbwrapper.cpp
    dbwrapper.h
    defs.h
    diagnostics.cpp
    diagnostics.h
    filtertrans.cpp
    filtertrans.h
    filtertransdialog.cpp
    filtertransdialog.h
    fusedtransaction.cpp
    fusedtransaction.h
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
    mmchecking_list.cpp
    mmchecking_list.h
    mmcombobox.h
    mmcustomdata.h
    mmcustomdata.cpp
    mmex.cpp
    mmex.h
    mmframe.cpp
    mmframe.h
    mmframereport.cpp
    mmhelppanel.cpp
    mmhelppanel.h
    mmhomepagepanel.cpp
    mmhomepagepanel.h
    mmhomepage.cpp
    mmhomepage.h
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
    mmTreeItemData.cpp
    mmTreeItemData.h
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
    optionsettingshome.cpp
    optionsettingshome.h
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
    primitive.cpp
    primitive.h
    recentfiles.cpp
    recentfiles.h
    relocatecategorydialog.cpp
    relocatecategorydialog.h
    relocatepayeedialog.cpp
    relocatepayeedialog.h
    relocatetagdialog.cpp
    relocatetagdialog.h
    resource.h
    singleton.h
    sharetransactiondialog.cpp
    sharetransactiondialog.h
    splittransactionsdialog.cpp
    splittransactionsdialog.h
    stockdialog.cpp
    stockdialog.h
    stockspanel.cpp
    stockspanel.h
    stocks_list.cpp
    stocks_list.h
    tagdialog.cpp
    tagdialog.h
    themes.cpp
    themes.h
    transdialog.cpp
    transdialog.h
    transactionsupdatedialog.cpp
    transactionsupdatedialog.h
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
    db/DB_Table_Tag_V1.h
    db/DB_Table_Taglink_V1.h
    db/DB_Table_Translink_V1.h
    db/DB_Table_Usage_V1.h

    reports/allreport.h
    reports/budgetcategorysummary.cpp
    reports/budgetcategorysummary.h
    reports/budget.cpp
    reports/budget.h
    reports/budgetingperf.cpp
    reports/budgetingperf.h
    reports/bugreport.h
    reports/cashflow.cpp
    reports/cashflow.h
    reports/categexp.cpp
    reports/categexp.h
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
    model/Model_Tag.cpp
    model/Model_Tag.h
    model/Model_Taglink.cpp
    model/Model_Taglink.h
    model/Model_Translink.cpp
    model/Model_Translink.h
    model/Model_Usage.cpp
    model/Model_Usage.h

    "${CMAKE_CURRENT_BINARY_DIR}/versions.h"
    "platfdep_${MMEX_PLATFDEP}"

    "${MACOSX_APP_ICON_FILE}"
    "${MMEX_RC}")

if(MSVC AND MSVC_VERSION LESS 1800)
    message(SEND_ERROR "MSVC version too old. Please use VS2013 (12.0) or later for required C++11 features.")
endif()

if(";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";cxx_std_11;")
    target_compile_features(${MMEX_EXE} PUBLIC cxx_std_11)
elseif(";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";cxx_range_for;"
        AND ";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";cxx_nullptr;"
        AND ";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";cxx_variadic_templates;")
    target_compile_features(${MMEX_EXE} PUBLIC
        cxx_range_for cxx_nullptr cxx_variadic_templates)
else()
    CHECK_CXX_COMPILER_FLAG("-std=gnu++11" COMPILER_SUPPORTS_GXX11)
    CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
    CHECK_CXX_COMPILER_FLAG("-std=gnu++0x" COMPILER_SUPPORTS_GXX0X)
    CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

    if(COMPILER_SUPPORTS_GXX11)
        target_compile_options(${MMEX_EXE} PUBLIC -std=gnu++11)
    elseif(COMPILER_SUPPORTS_CXX11)
        target_compile_options(${MMEX_EXE} PUBLIC -std=c++11)
    elseif(COMPILER_SUPPORTS_GXX0X)
        target_compile_options(${MMEX_EXE} PUBLIC -std=gnu++0x)
    elseif(COMPILER_SUPPORTS_CXX0X)
        target_compile_options(${MMEX_EXE} PUBLIC -std=c++0x)
    else()
        message(SEND_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support.")
    endif()
endif()

target_include_directories(${MMEX_EXE} PUBLIC . model db)
target_link_libraries(${MMEX_EXE} PUBLIC
    wxSQLite3
    RapidJSON
    HTML-template
    CURL::libcurl
    fmt
    LuaGlue
    Lua)

if(MSVC)
    # Based on this http://stackoverflow.com/a/8294669
    # added solution to compile problems due to
    # conflict between winsock and winsock2
    # Partialy reinvented fix from commit
    # commit 06accae1273e66ced469672151522e45eee685a9
    target_compile_definitions(${MMEX_EXE} PRIVATE WIN32_LEAN_AND_MEAN)
endif()

install(TARGETS ${MMEX_EXE}
    RUNTIME DESTINATION ${MMEX_BIN_DIR}
    BUNDLE  DESTINATION .)
