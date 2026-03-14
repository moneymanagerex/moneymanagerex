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
    add_compile_options(-Wall -Wextra -Wno-unknown-pragmas -Wshadow -Wformat=2
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
    build.h
    mmcombobox.h
    mmex.cpp
    mmex.h
    mmframe.cpp
    mmframe.h
    mmframereport.cpp
    resource.h

    base/constants.cpp
    base/constants.h
    base/defs.h
    base/images_list.cpp
    base/images_list.h
    base/paths.cpp
    base/paths.h
    base/platfdep.h
    base/types.h

    util/_primitive.cpp
    util/_primitive.h
    util/_simple.cpp
    util/_simple.h
    util/_util.cpp
    util/_util.h
    util/mmCache.h
    util/mmCache.tpp
    util/mmChoice.cpp
    util/mmChoice.h
    util/mmDate.cpp
    util/mmDate.h
    util/mmDateRange.cpp
    util/mmDateRange.h
    util/mmDateRange2.cpp
    util/mmDateRange2.h
    util/mmDateTime.cpp
    util/mmDateTime.h
    util/mmFileHistory.cpp
    util/mmFileHistory.h
    util/mmMiniEditor.cpp
    util/mmMiniEditor.h
    util/mmSQLite3Hook.h
    util/mmSingleton.h
    util/mmTextCtrl.cpp
    util/mmTextCtrl.h
    util/mmTips.h
    util/mmTreeItemData.cpp
    util/mmTreeItemData.h
    util/mmCalcValidator.h

    table/AccountTable.cpp
    table/AccountTable.h
    table/AssetTable.cpp
    table/AssetTable.h
    table/AttachmentTable.cpp
    table/AttachmentTable.h
    table/BudgetPeriodTable.cpp
    table/BudgetPeriodTable.h
    table/BudgetTable.cpp
    table/BudgetTable.h
    table/CategoryTable.cpp
    table/CategoryTable.h
    table/CurrencyHistoryTable.cpp
    table/CurrencyHistoryTable.h
    table/CurrencyTable.cpp
    table/CurrencyTable.h
    table/FieldTable.cpp
    table/FieldTable.h
    table/FieldValueTable.cpp
    table/FieldValueTable.h
    table/InfoTable.cpp
    table/InfoTable.h
    table/PayeeTable.cpp
    table/PayeeTable.h
    table/ReportTable.cpp
    table/ReportTable.h
    table/SchedSplitTable.cpp
    table/SchedSplitTable.h
    table/SchedTable.cpp
    table/SchedTable.h
    table/SettingTable.cpp
    table/SettingTable.h
    table/StockHistoryTable.cpp
    table/StockHistoryTable.h
    table/StockTable.cpp
    table/StockTable.h
    table/TagLinkTable.cpp
    table/TagLinkTable.h
    table/TagTable.cpp
    table/TagTable.h
    table/TrxLinkTable.cpp
    table/TrxLinkTable.h
    table/TrxShareTable.cpp
    table/TrxShareTable.h
    table/TrxSplitTable.cpp
    table/TrxSplitTable.h
    table/TrxTable.cpp
    table/TrxTable.h
    table/UsageTable.cpp
    table/UsageTable.h
    table/_TableBase.cpp
    table/_TableBase.h
    table/_TableFactory.h
    table/_TableFactory.tpp
    table/_TableUpgrade.h

    data/AccountData.cpp
    data/AccountData.h
    data/AssetData.cpp
    data/AssetData.h
    data/AttachmentData.cpp
    data/AttachmentData.h
    data/BudgetData.cpp
    data/BudgetData.h
    data/BudgetPeriodData.cpp
    data/BudgetPeriodData.h
    data/CategoryData.cpp
    data/CategoryData.h
    data/CurrencyData.cpp
    data/CurrencyData.h
    data/CurrencyHistoryData.cpp
    data/CurrencyHistoryData.h
    data/FieldData.cpp
    data/FieldData.h
    data/FieldValueData.cpp
    data/FieldValueData.h
    data/InfoData.cpp
    data/InfoData.h
    data/PayeeData.cpp
    data/PayeeData.h
    data/ReportData.cpp
    data/ReportData.h
    data/SchedData.cpp
    data/SchedData.h
    data/SchedSplitData.cpp
    data/SchedSplitData.h
    data/SettingData.cpp
    data/SettingData.h
    data/StockData.cpp
    data/StockData.h
    data/StockHistoryData.cpp
    data/StockHistoryData.h
    data/TagData.cpp
    data/TagData.h
    data/TagLinkData.cpp
    data/TagLinkData.h
    data/TrxData.cpp
    data/TrxData.h
    data/TrxLinkData.cpp
    data/TrxLinkData.h
    data/TrxShareData.cpp
    data/TrxShareData.h
    data/TrxSplitData.cpp
    data/TrxSplitData.h
    data/UsageData.cpp
    data/UsageData.h
    data/_DataEnum.cpp
    data/_DataEnum.h

    model/AccountModel.cpp
    model/AccountModel.h
    model/AssetModel.cpp
    model/AssetModel.h
    model/AttachmentModel.cpp
    model/AttachmentModel.h
    model/BudgetModel.cpp
    model/BudgetModel.h
    model/BudgetPeriodModel.cpp
    model/BudgetPeriodModel.h
    model/CategoryModel.cpp
    model/CategoryModel.h
    model/CurrencyHistoryModel.cpp
    model/CurrencyHistoryModel.h
    model/CurrencyModel.cpp
    model/CurrencyModel.h
    model/FieldModel.cpp
    model/FieldModel.h
    model/FieldValueModel.cpp
    model/FieldValueModel.h
    model/InfoModel.cpp
    model/InfoModel.h
    model/Journal.cpp
    model/Journal.h
    model/PayeeModel.cpp
    model/PayeeModel.h
    model/PrefModel.cpp
    model/PrefModel.h
    model/ReportModel.cpp
    model/ReportModel.h
    model/SchedModel.cpp
    model/SchedModel.h
    model/SchedSplitModel.cpp
    model/SchedSplitModel.h
    model/SettingModel.cpp
    model/SettingModel.h
    model/StockHistoryModel.cpp
    model/StockHistoryModel.h
    model/StockModel.cpp
    model/StockModel.h
    model/TagLinkModel.cpp
    model/TagLinkModel.h
    model/TagModel.cpp
    model/TagModel.h
    model/TrxFilter.cpp
    model/TrxFilter.h
    model/TrxLinkModel.cpp
    model/TrxLinkModel.h
    model/TrxModel.cpp
    model/TrxModel.h
    model/TrxShareModel.cpp
    model/TrxShareModel.h
    model/TrxSplitModel.cpp
    model/TrxSplitModel.h
    model/UsageModel.cpp
    model/UsageModel.h
    model/_ModelBase.cpp
    model/_ModelBase.h
    model/_all.h

    panel/AssetPanel.cpp
    panel/AssetPanel.h
    panel/BudgetPanel.cpp
    panel/BudgetPanel.h
    panel/DashboardPanel.cpp
    panel/DashboardPanel.h
    panel/DashboardWidget.cpp
    panel/DashboardWidget.h
    panel/HelpPanel.cpp
    panel/HelpPanel.h
    panel/JournalList.cpp
    panel/JournalList.h
    panel/JournalPanel.cpp
    panel/JournalPanel.h
    panel/ReportPanel.cpp
    panel/ReportPanel.h
    panel/SchedPanel.cpp
    panel/SchedPanel.h
    panel/StockList.cpp
    panel/StockList.h
    panel/StockPanel.cpp
    panel/StockPanel.h
    panel/_PanelBase.cpp
    panel/_PanelBase.h

    manager/CategoryManager.cpp
    manager/CategoryManager.h
    manager/CurrencyManager.cpp
    manager/CurrencyManager.h
    manager/DateRangeManager.cpp
    manager/DateRangeManager.h
    manager/FieldManager.cpp
    manager/FieldManager.h
    manager/GeneralReportManager.cpp
    manager/GeneralReportManager.h
    manager/PayeeManager.cpp
    manager/PayeeManager.h
    manager/PrefManager.cpp
    manager/PrefManager.h
    manager/TagManager.cpp
    manager/TagManager.h
    manager/ThemeManager.cpp
    manager/ThemeManager.h

    dialog/AboutDialog.cpp
    dialog/AboutDialog.h
    dialog/AccountDialog.cpp
    dialog/AccountDialog.h
    dialog/AssetDialog.cpp
    dialog/AssetDialog.h
    dialog/AttachmentDialog.cpp
    dialog/AttachmentDialog.h
    dialog/BudgetEntryDialog.cpp
    dialog/BudgetEntryDialog.h
    dialog/BudgetYearDialog.cpp
    dialog/BudgetYearDialog.h
    dialog/BudgetYearEntryDialog.cpp
    dialog/BudgetYearEntryDialog.h
    dialog/CurrencyChoiceDialog.cpp
    dialog/CurrencyChoiceDialog.h
    dialog/DateRangeDialog.cpp
    dialog/DateRangeDialog.h
    dialog/DiagnosticsDialog.cpp
    dialog/DiagnosticsDialog.h
    dialog/FieldDialog.cpp
    dialog/FieldDialog.h
    dialog/FieldValueDialog.cpp
    dialog/FieldValueDialog.h
    dialog/MergeCategoryDialog.cpp
    dialog/MergeCategoryDialog.h
    dialog/MergePayeeDialog.cpp
    dialog/MergePayeeDialog.h
    dialog/MergeTagDialog.cpp
    dialog/MergeTagDialog.h
    dialog/SchedDialog.cpp
    dialog/SchedDialog.h
    dialog/SplitDialog.cpp
    dialog/SplitDialog.h
    dialog/StartupDialog.cpp
    dialog/StartupDialog.h
    dialog/StockDialog.cpp
    dialog/StockDialog.h
    dialog/TrxDialog.cpp
    dialog/TrxDialog.h
    dialog/TrxFilterDialog.cpp
    dialog/TrxFilterDialog.h
    dialog/TrxLinkDialog.cpp
    dialog/TrxLinkDialog.h
    dialog/TrxShareDialog.cpp
    dialog/TrxShareDialog.h
    dialog/TrxUpdateDialog.cpp
    dialog/TrxUpdateDialog.h

    pref/AttachmentPref.cpp
    pref/AttachmentPref.h
    pref/DashboardPref.cpp
    pref/DashboardPref.h
    pref/GeneralPref.cpp
    pref/GeneralPref.h
    pref/NetworkPref.cpp
    pref/NetworkPref.h
    pref/OtherPref.cpp
    pref/OtherPref.h
    pref/TrxPref.cpp
    pref/TrxPref.h
    pref/ViewPref.cpp
    pref/ViewPref.h
    pref/_PrefBase.cpp
    pref/_PrefBase.h

    report/BalanceReport.cpp
    report/BalanceReport.h
    report/CategoryReport.cpp
    report/CategoryReport.h
    report/FlowReport.cpp
    report/FlowReport.h
    report/ForecastReport.cpp
    report/ForecastReport.h
    report/InExReport.cpp
    report/InExReport.h
    report/PayeeReport.cpp
    report/PayeeReport.h
    report/StocksReport.cpp
    report/StocksReport.h
    report/TrxReport.cpp
    report/TrxReport.h
    report/UsageReport.cpp
    report/UsageReport.h
    report/_ReportBase.cpp
    report/_ReportBase.h
    report/_all.h
    report/budget.cpp
    report/budget.h
    report/budgetcategorysummary.cpp
    report/budgetcategorysummary.h
    report/budgetingperf.cpp
    report/budgetingperf.h
    report/bugreport.h
    report/htmlbuilder.cpp
    report/htmlbuilder.h

    uicontrols/reconciledialog.cpp
    uicontrols/reconciledialog.h
    uicontrols/navigatordialog.cpp
    uicontrols/navigatordialog.h
    uicontrols/navigatoreditdialog.cpp
    uicontrols/navigatoreditdialog.h
    uicontrols/navigatortypes.cpp
    uicontrols/navigatortypes.h
    uicontrols/toolbardialog.cpp
    uicontrols/toolbardialog.h
    uicontrols/toolbartypes.cpp
    uicontrols/toolbartypes.h

    db/dbcheck.cpp
    db/dbcheck.h
    db/dbupgrade.cpp
    db/dbupgrade.h
    db/dbwrapper.cpp
    db/dbwrapper.h

    import_export/export.cpp
    import_export/export.h
    import_export/ofx_import_gui.cpp
    import_export/ofx_import_gui.h
    import_export/parsers.cpp
    import_export/parsers.h
    import_export/payeematchandmerge.cpp
    import_export/payeematchandmerge.h
    import_export/qif_export.cpp
    import_export/qif_export.h
    import_export/qif_import.cpp
    import_export/qif_import.h
    import_export/qif_import_gui.cpp
    import_export/qif_import_gui.h
    import_export/univcsvdialog.cpp
    import_export/univcsvdialog.h
    import_export/webapp.cpp
    import_export/webapp.h
    import_export/webappdialog.cpp
    import_export/webappdialog.h

    generic/generictreelistdialog.cpp
    generic/generictreelistdialog.h

    wizard/wizard_newaccount.cpp
    wizard/wizard_newaccount.h
    wizard/wizard_newdb.cpp
    wizard/wizard_newdb.h
    wizard/wizard_update.cpp
    wizard/wizard_update.h

    "${CMAKE_CURRENT_BINARY_DIR}/versions.h"
    "base/platfdep_${MMEX_PLATFDEP}"

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

target_include_directories(${MMEX_EXE} PUBLIC .)
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
