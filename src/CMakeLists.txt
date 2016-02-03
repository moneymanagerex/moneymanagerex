IF(NOT InMMEX)
	MESSAGE(FATAL_ERROR "Use the top-level CMake script!")
ENDIF(NOT InMMEX)

SET(MMEX_SOURCES
	"aboutdialog.cpp"
	"accountdialog.cpp"
	"appstartdialog.cpp"
	"assetdialog.cpp"
	"assetspanel.cpp"
	"attachmentdialog.cpp"
	"billsdepositsdialog.cpp"
	"billsdepositspanel.cpp"
	"budgetentrydialog.cpp"
	"budgetingpanel.cpp"
	"budgetyeardialog.cpp"
	"budgetyearentrydialog.cpp"
	"categdialog.cpp"
	"constants.cpp"
	"currencydialog.cpp"
	"customfielddialog.cpp"
	"customfieldeditdialog.cpp"
	"customfieldlistdialog.cpp"
	"dbcheck.cpp"
	"dbwrapper.cpp"
	"filtertransdialog.cpp"
	"general_report_manager.cpp"
	"images_list.cpp"
	"maincurrencydialog.cpp"
	"minimal_editor.cpp"
	"mmCalculator.cpp"
	"mmcheckingpanel.cpp"
	"mmex.cpp"
	"mmframe.cpp"
	"mmframereport.cpp"
	"mmhelppanel.cpp"
	"mmhomepagepanel.cpp"
	"mmOptionAttachmentSettings.cpp"
	"mmOptionBaseSettings.cpp"
	"mmOption.cpp"
	"mmOptionGeneralSettings.cpp"
	"mmOptionMiscSettings.cpp"
	"mmOptionNetSettings.cpp"
	"mmOptionViewSettings.cpp"
	"mmpanelbase.cpp"
	"mmreportspanel.cpp"
	"mmSimpleDialogs.cpp"
	"optionsdialog.cpp"
	"paths.cpp"
	"payeedialog.cpp"
	"recentfiles.cpp"
	"relocatecategorydialog.cpp"
	"relocatepayeedialog.cpp"
	"splitdetailsdialog.cpp"
	"splittransactionsdialog.cpp"
	"stockdialog.cpp"
	"stockspanel.cpp"
	"transdialog.cpp"
	"util.cpp"
	"webapp.cpp"
	"webappdialog.cpp"
	"webserver.cpp"
	"wizard_newaccount.cpp"
	"wizard_newdb.cpp"
	"wizard_update.cpp"

	"import_export/export.cpp"
	"import_export/parsers.cpp"
	"import_export/qif_export.cpp"
	"import_export/qif_import.cpp"
	"import_export/qif_import_gui.cpp"
	"import_export/univcsvdialog.cpp"

	"model/Model_Account.cpp"
	"model/Model_Asset.cpp"
	"model/Model_Attachment.cpp"
	"model/Model_Billsdeposits.cpp"
	"model/Model_Budget.cpp"
	"model/Model_Budgetsplittransaction.cpp"
	"model/Model_Budgetyear.cpp"
	"model/Model_Category.cpp"
	"model/Model_Checking.cpp"
	"model/Model_Currency.cpp"
	"model/Model_CurrencyHistory.cpp"
	"model/Model_CustomField.cpp"
	"model/Model_CustomFieldData.cpp"
	"model/Model_Infotable.cpp"
	"model/Model_Payee.cpp"
	"model/Model_Report.cpp"
	"model/Model_Setting.cpp"
	"model/Model_Splittransaction.cpp"
	"model/Model_Stock.cpp"
	"model/Model_StockHistory.cpp"
	"model/Model_Subcategory.cpp"
	"model/Model_Usage.cpp"

	"reports/budgetcategorysummary.cpp"
	"reports/budget.cpp"
	"reports/budgetingperf.cpp"
	"reports/cashflow.cpp"
	"reports/categexp.cpp"
	"reports/categovertimeperf.cpp"
	"reports/htmlbuilder.cpp"
	"reports/incexpenses.cpp"
	"reports/mmDateRange.cpp"
	"reports/myusage.cpp"
	"reports/payee.cpp"
	"reports/reportbase.cpp"
	"reports/summary.cpp"
	"reports/summarystocks.cpp"
	"reports/transactions.cpp"

	"../3rd/cgitemplate/html_template.cpp"
	"../3rd/mongoose/mongoose.c"
)
#OSX
IF (UNIX AND APPLE)
	SET(MMEX_PLATFDEP "mac/platfdep.cpp")
ENDIF ()
#*nix
IF (UNIX AND NOT APPLE)
	SET(MMEX_PLATFDEP "unix/platfdep.cpp")
ENDIF ()
#Windows
IF (WIN32)
	SET(MMEX_PLATFDEP "win/platfdep.cpp")
ENDIF ()

SET(EXECUTABLE_NAME "mmex")
ADD_EXECUTABLE(mmex ${MMEX_SOURCES} ${MMEX_PLATFDEP})
TARGET_LINK_LIBRARIES(mmex ${wxWidgets_LIBRARIES} wxsqlite3 lua)

