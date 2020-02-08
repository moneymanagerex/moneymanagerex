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
    set(MMEX_PLATFDEP mac)
elseif(UNIX)
    set(MMEX_PLATFDEP unix)
elseif(WIN32)
    set(MMEX_PLATFDEP win)
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
    wx_compat.h

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
    reports/bugreport.h
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

    charts/wxareachart.cpp
    charts/wxareachart.h
    charts/wxareachartctrl.cpp
    charts/wxareachartctrl.h
    charts/wxareachartdatasetoptions.cpp
    charts/wxareachartdatasetoptions.h
    charts/wxareachartoptions.cpp
    charts/wxareachartoptions.h
    charts/wxbarchart.cpp
    charts/wxbarchart.h
    charts/wxbarchartctrl.cpp
    charts/wxbarchartctrl.h
    charts/wxbarchartdatasetoptions.cpp
    charts/wxbarchartdatasetoptions.h
    charts/wxbarchartoptions.cpp
    charts/wxbarchartoptions.h
    charts/wxboxplot.cpp
    charts/wxboxplot.h
    charts/wxboxplotctrl.cpp
    charts/wxboxplotctrl.h
    charts/wxboxplotdatasetoptions.cpp
    charts/wxboxplotdatasetoptions.h
    charts/wxboxplotoptions.cpp
    charts/wxboxplotoptions.h
    charts/wxbubblechart.cpp
    charts/wxbubblechart.h
    charts/wxbubblechartctrl.cpp
    charts/wxbubblechartctrl.h
    charts/wxbubblechartoptions.cpp
    charts/wxbubblechartoptions.h
    charts/wxcandlestickchart.cpp
    charts/wxcandlestickchart.h
    charts/wxcandlestickchartctrl.cpp
    charts/wxcandlestickchartctrl.h
    charts/wxcandlestickchartoptions.cpp
    charts/wxcandlestickchartoptions.h
    charts/wxchart.cpp
    charts/wxchart.h
    charts/wxchartbackground.cpp
    charts/wxchartbackground.h
    charts/wxchartbackgroundoptions.cpp
    charts/wxchartbackgroundoptions.h
    charts/wxchartcommonoptions.cpp
    charts/wxchartcommonoptions.h
    charts/wxchartctrl.cpp
    charts/wxchartctrl.h
    charts/wxchartfontoptions.cpp
    charts/wxchartfontoptions.h
    charts/wxchartmultitooltip.cpp
    charts/wxchartmultitooltip.h
    charts/wxchartmultitooltipoptions.cpp
    charts/wxchartmultitooltipoptions.h
    charts/wxchartobservers.h
    charts/wxchartoptions.cpp
    charts/wxchartoptions.h
    charts/wxchartpadding.cpp
    charts/wxchartpadding.h
    charts/wxcharts.h
    charts/wxchartsarc.cpp
    charts/wxchartsarc.h
    charts/wxchartsarcoptions.cpp
    charts/wxchartsarcoptions.h
    charts/wxchartsaxis.cpp
    charts/wxchartsaxis.h
    charts/wxchartsaxisoptions.cpp
    charts/wxchartsaxisoptions.h
    charts/wxchartsbrushoptions.cpp
    charts/wxchartsbrushoptions.h
    charts/wxchartscategoricalaxis.cpp
    charts/wxchartscategoricalaxis.h
    charts/wxchartscategoricaldata.cpp
    charts/wxchartscategoricaldata.h
    charts/wxchartscategoricaldataobserver.cpp
    charts/wxchartscategoricaldataobserver.h
    charts/wxchartscircle.cpp
    charts/wxchartscircle.h
    charts/wxchartscircleoptions.cpp
    charts/wxchartscircleoptions.h
    charts/wxchartsdataset.cpp
    charts/wxchartsdataset.h
    charts/wxchartsdatasetid.cpp
    charts/wxchartsdatasetid.h
    charts/wxchartsdatasettheme.cpp
    charts/wxchartsdatasettheme.h
    charts/wxchartsdoubledataset.cpp
    charts/wxchartsdoubledataset.h
    charts/wxchartselement.cpp
    charts/wxchartselement.h
    charts/wxchartsgrid.cpp
    charts/wxchartsgrid.h
    charts/wxchartsgridmapping.cpp
    charts/wxchartsgridmapping.h
    charts/wxchartsgridoptions.cpp
    charts/wxchartsgridoptions.h
    charts/wxchartslabel.cpp
    charts/wxchartslabel.h
    charts/wxchartslabelgroup.cpp
    charts/wxchartslabelgroup.h
    charts/wxchartslabeloptions.cpp
    charts/wxchartslabeloptions.h
    charts/wxchartslegendctrl.cpp
    charts/wxchartslegendctrl.h
    charts/wxchartslegenditem.cpp
    charts/wxchartslegenditem.h
    charts/wxchartslegendline.cpp
    charts/wxchartslegendline.h
    charts/wxchartslegendlineoptions.cpp
    charts/wxchartslegendlineoptions.h
    charts/wxchartslegendoptions.cpp
    charts/wxchartslegendoptions.h
    charts/wxchartslicedata.cpp
    charts/wxchartslicedata.h
    charts/wxchartsnumericalaxis.cpp
    charts/wxchartsnumericalaxis.h
    charts/wxchartsohlcdata.cpp
    charts/wxchartsohlcdata.h
    charts/wxchartspenoptions.cpp
    charts/wxchartspenoptions.h
    charts/wxchartspoint.cpp
    charts/wxchartspoint.h
    charts/wxchartspointoptions.cpp
    charts/wxchartspointoptions.h
    charts/wxchartsprecisiontheme.cpp
    charts/wxchartsprecisiontheme.h
    charts/wxchartspresentationtheme.cpp
    charts/wxchartspresentationtheme.h
    charts/wxchartsradialgrid.cpp
    charts/wxchartsradialgrid.h
    charts/wxchartsradialgridoptions.cpp
    charts/wxchartsradialgridoptions.h
    charts/wxchartsrectangle.cpp
    charts/wxchartsrectangle.h
    charts/wxchartsrectangleoptions.cpp
    charts/wxchartsrectangleoptions.h
    charts/wxchartstheme.cpp
    charts/wxchartstheme.h
    charts/wxchartsthemefactory.cpp
    charts/wxchartsthemefactory.h
    charts/wxchartsthemeid.cpp
    charts/wxchartsthemeid.h
    charts/wxchartstimeaxis.cpp
    charts/wxchartstimeaxis.h
    charts/wxchartsutilities.cpp
    charts/wxchartsutilities.h
    charts/wxcharttooltip.cpp
    charts/wxcharttooltip.h
    charts/wxcharttooltipoptions.cpp
    charts/wxcharttooltipoptions.h
    charts/wxcharttooltipprovider.cpp
    charts/wxcharttooltipprovider.h
    charts/wxcolumnchart.cpp
    charts/wxcolumnchart.h
    charts/wxcolumnchartctrl.cpp
    charts/wxcolumnchartctrl.h
    charts/wxcolumnchartdatasetoptions.cpp
    charts/wxcolumnchartdatasetoptions.h
    charts/wxcolumnchartoptions.cpp
    charts/wxcolumnchartoptions.h
    charts/wxcombinationchart.cpp
    charts/wxcombinationchart.h
    charts/wxcombinationchartctrl.cpp
    charts/wxcombinationchartctrl.h
    charts/wxcombinationchartoptions.cpp
    charts/wxcombinationchartoptions.h
    charts/wxdoughnutandpiechartbase.cpp
    charts/wxdoughnutandpiechartbase.h
    charts/wxdoughnutandpiechartoptionsbase.cpp
    charts/wxdoughnutandpiechartoptionsbase.h
    charts/wxdoughnutchart.cpp
    charts/wxdoughnutchart.h
    charts/wxdoughnutchartctrl.cpp
    charts/wxdoughnutchartctrl.h
    charts/wxdoughnutchartoptions.cpp
    charts/wxdoughnutchartoptions.h
    charts/wxhistogram.cpp
    charts/wxhistogram.h
    charts/wxhistogramctrl.cpp
    charts/wxhistogramctrl.h
    charts/wxhistogramoptions.cpp
    charts/wxhistogramoptions.h
    charts/wxlinechart.cpp
    charts/wxlinechart.h
    charts/wxlinechartctrl.cpp
    charts/wxlinechartctrl.h
    charts/wxlinechartdatasetoptions.cpp
    charts/wxlinechartdatasetoptions.h
    charts/wxlinechartoptions.cpp
    charts/wxlinechartoptions.h
    charts/wxmath2dplot.cpp
    charts/wxmath2dplot.h
    charts/wxmath2dplotctrl.cpp
    charts/wxmath2dplotctrl.h
    charts/wxmath2dplotoptions.cpp
    charts/wxmath2dplotoptions.h
    charts/wxohlcchart.cpp
    charts/wxohlcchart.h
    charts/wxohlcchartctrl.cpp
    charts/wxohlcchartctrl.h
    charts/wxohlcchartoptions.cpp
    charts/wxohlcchartoptions.h
    charts/wxpiechart.cpp
    charts/wxpiechart.h
    charts/wxpiechartctrl.cpp
    charts/wxpiechartctrl.h
    charts/wxpiechartoptions.cpp
    charts/wxpiechartoptions.h
    charts/wxpolarareachart.cpp
    charts/wxpolarareachart.h
    charts/wxpolarareachartctrl.cpp
    charts/wxpolarareachartctrl.h
    charts/wxpolarareachartoptions.cpp
    charts/wxpolarareachartoptions.h
    charts/wxradarchart.cpp
    charts/wxradarchart.h
    charts/wxradarchartctrl.cpp
    charts/wxradarchartctrl.h
    charts/wxradarchartoptions.cpp
    charts/wxradarchartoptions.h
    charts/wxscatterplot.cpp
    charts/wxscatterplot.h
    charts/wxscatterplotctrl.cpp
    charts/wxscatterplotctrl.h
    charts/wxscatterplotoptions.cpp
    charts/wxscatterplotoptions.h
    charts/wxstackedbarchart.cpp
    charts/wxstackedbarchart.h
    charts/wxstackedbarchartctrl.cpp
    charts/wxstackedbarchartctrl.h
    charts/wxstackedbarchartdatasetoptions.cpp
    charts/wxstackedbarchartdatasetoptions.h
    charts/wxstackedbarchartoptions.cpp
    charts/wxstackedbarchartoptions.h
    charts/wxstackedcolumnchart.cpp
    charts/wxstackedcolumnchart.h
    charts/wxstackedcolumnchartctrl.cpp
    charts/wxstackedcolumnchartctrl.h
    charts/wxstackedcolumnchartdatasetoptions.cpp
    charts/wxstackedcolumnchartdatasetoptions.h
    charts/wxstackedcolumnchartoptions.cpp
    charts/wxstackedcolumnchartoptions.h
    charts/wxtimeserieschart.cpp
    charts/wxtimeserieschart.h
    charts/wxtimeserieschartctrl.cpp
    charts/wxtimeserieschartctrl.h
    charts/wxtimeserieschartoptions.cpp
    charts/wxtimeserieschartoptions.h

    "${CMAKE_CURRENT_BINARY_DIR}/versions.h"
    "platfdep_${MMEX_PLATFDEP}.cpp"
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
    LuaGlue
    Lua)

if(MSVC)
    # Based on this http://stackoverflow.com/a/8294669
    # added solution to compile problems due to
    # conflict between winsock and winsock2
    # Partially reinvented fix from commit
    # commit 06accae1273e66ced469672151522e45eee685a9
    target_compile_definitions(${MMEX_EXE} PRIVATE WIN32_LEAN_AND_MEAN)
endif()

install(TARGETS ${MMEX_EXE}
    RUNTIME DESTINATION ${MMEX_BIN_DIR}
    BUNDLE  DESTINATION .)
