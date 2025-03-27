/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2012 Stefano Giorgio
 Copyright (C) 2013 - 2022 Nikolay Akimov
 Copyright (C) 2014 James Higley
 Copyright (C) 2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2021, 2022, 2024 Mark Whalley (mark@ipx.co.uk)

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ********************************************************/

#include "mmframe.h"

#include "aboutdialog.h"
#include "accountdialog.h"
#include "appstartdialog.h"
#include "assetspanel.h"
#include "attachmentdialog.h"
#include "billsdepositsdialog.h"
#include "billsdepositspanel.h"
#include "budgetingpanel.h"
#include "budgetyeardialog.h"
#include "categdialog.h"
#include "constants.h"
#include "customfieldlistdialog.h"
#include "dbcheck.h"
#include "dbupgrade.h"
#include "dbwrapper.h"
#include "diagnostics.h"
#include "filtertransdialog.h"
#include "general_report_manager.h"
#include "images_list.h"
#include "maincurrencydialog.h"
#include "mmcheckingpanel.h"
#include "mmchecking_list.h"
#include "mmex.h"
#include "mmhelppanel.h"
#include "mmhomepagepanel.h"
#include "mmreportspanel.h"
#include "mmSimpleDialogs.h"
#include "mmHook.h"
#include "optiondialog.h"
#include "payeedialog.h"
#include "relocatecategorydialog.h"
#include "relocatepayeedialog.h"
#include "relocatetagdialog.h"
#include "recentfiles.h"
#include "stockspanel.h"
#include "tagdialog.h"
#include "themes.h"
#include "transdialog.h"
#include "util.h"
#include "mmTreeItemData.h"
#include "webapp.h"
#include "webappdialog.h"
#include "wizard_newdb.h"
#include "wizard_newaccount.h"
#include "wizard_update.h"

#include "reports/allreport.h"
#include "reports/bugreport.h"

#include "import_export/qif_export.h"
#include "import_export/qif_import_gui.h"
#include "import_export/univcsvdialog.h"

#include "model/allmodel.h"

#include <wx/fs_mem.h>
#include <wx/busyinfo.h>
#include <stack>
#include <unordered_set>

//----------------------------------------------------------------------------

int REPEAT_TRANS_DELAY_TIME = 3000; // 3 seconds
//----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(mmGUIFrame, wxFrame)
EVT_MENU(MENU_NEW, mmGUIFrame::OnNew)
EVT_MENU(MENU_OPEN, mmGUIFrame::OnOpen)
EVT_MENU(MENU_SAVE_AS, mmGUIFrame::OnSaveAs)
EVT_MENU(MENU_EXPORT_CSV, mmGUIFrame::OnExportToCSV)
EVT_MENU(MENU_EXPORT_XML, mmGUIFrame::OnExportToXML)
EVT_MENU(MENU_EXPORT_QIF, mmGUIFrame::OnExportToQIF)
EVT_MENU(MENU_EXPORT_JSON, mmGUIFrame::OnExportToJSON)
EVT_MENU(MENU_EXPORT_MMEX, mmGUIFrame::OnExportToMMEX)
EVT_MENU(MENU_IMPORT_QIF, mmGUIFrame::OnImportQIF)
EVT_MENU(MENU_IMPORT_UNIVCSV, mmGUIFrame::OnImportUniversalCSV)
EVT_MENU(MENU_IMPORT_XML, mmGUIFrame::OnImportXML)
EVT_MENU(MENU_IMPORT_WEBAPP, mmGUIFrame::OnImportWebApp)
EVT_MENU(wxID_EXIT, mmGUIFrame::OnQuit)
EVT_MENU(MENU_NEWACCT, mmGUIFrame::OnNewAccount)
EVT_MENU(MENU_HOMEPAGE, mmGUIFrame::OnAccountList)
EVT_MENU(MENU_ACCTEDIT, mmGUIFrame::OnEditAccount)
EVT_MENU(MENU_ACCTDELETE, mmGUIFrame::OnDeleteAccount)
EVT_MENU(MENU_ACCOUNT_REALLOCATE, mmGUIFrame::OnReallocateAccount)
EVT_MENU(MENU_ORGCATEGS, mmGUIFrame::OnOrgCategories)
EVT_MENU(MENU_ORGPAYEE, mmGUIFrame::OnOrgPayees)
EVT_MENU(MENU_ORGTAGS, mmGUIFrame::OnOrgTags)
EVT_MENU(wxID_PREFERENCES, mmGUIFrame::OnOptions)
EVT_MENU(wxID_NEW, mmGUIFrame::OnNewTransaction)
EVT_MENU(wxID_REFRESH, mmGUIFrame::refreshPanelData)
EVT_MENU(MENU_BUDGETSETUPDIALOG, mmGUIFrame::OnBudgetSetupDialog)
EVT_MENU(wxID_HELP, mmGUIFrame::OnHelp)
EVT_MENU(MENU_CHECKUPDATE, mmGUIFrame::OnCheckUpdate)
EVT_MENU(MENU_ANNOUNCEMENTMAILING, mmGUIFrame::OnBeNotified)
EVT_MENU_RANGE(MENU_FACEBOOK, MENU_TWITTER, mmGUIFrame::OnSimpleURLOpen)
EVT_MENU(MENU_REPORT_BUG, mmGUIFrame::OnReportBug)
EVT_MENU(MENU_DIAGNOSTICS, mmGUIFrame::OnDiagnostics)
EVT_MENU(wxID_ABOUT, mmGUIFrame::OnAbout)
EVT_MENU(wxID_PRINT, mmGUIFrame::OnPrintPage)
EVT_MENU(MENU_SHOW_APPSTART, mmGUIFrame::OnShowAppStartDialog)
EVT_MENU(MENU_EXPORT_HTML, mmGUIFrame::OnExportToHtml)
EVT_MENU(MENU_BILLSDEPOSITS, mmGUIFrame::OnBillsDeposits)
EVT_MENU(MENU_CONVERT_ENC_DB, mmGUIFrame::OnConvertEncryptedDB)
EVT_MENU(MENU_CHANGE_ENCRYPT_PASSWORD, mmGUIFrame::OnChangeEncryptPassword)
EVT_MENU(MENU_DB_VACUUM, mmGUIFrame::OnVacuumDB)
EVT_MENU(MENU_DB_DEBUG, mmGUIFrame::OnDebugDB)

EVT_MENU(MENU_ASSETS, mmGUIFrame::OnAssets)
EVT_MENU(MENU_CURRENCY, mmGUIFrame::OnCurrency)
EVT_MENU(MENU_RATES, mmGUIFrame::OnRates)
EVT_MENU(MENU_TRANSACTIONREPORT, mmGUIFrame::OnTransactionReport)
EVT_MENU(MENU_REFRESH_WEBAPP, mmGUIFrame::OnRefreshWebApp)
EVT_MENU(wxID_BROWSE, mmGUIFrame::OnCustomFieldsManager)
EVT_MENU(wxID_VIEW_LIST, mmGUIFrame::OnGeneralReportManager)
EVT_MENU(MENU_THEME_MANAGER, mmGUIFrame::OnThemeManager)
EVT_MENU(MENU_TREEPOPUP_LAUNCHWEBSITE, mmGUIFrame::OnLaunchAccountWebsite)
EVT_MENU(MENU_TREEPOPUP_ACCOUNTATTACHMENTS, mmGUIFrame::OnAccountAttachments)
EVT_MENU(MENU_VIEW_TOOLBAR, mmGUIFrame::OnViewToolbar)
EVT_MENU(MENU_VIEW_LINKS, mmGUIFrame::OnViewLinks)
EVT_MENU(MENU_VIEW_HIDE_SHARE_ACCOUNTS, mmGUIFrame::OnHideShareAccounts)
EVT_MENU(MENU_VIEW_HIDE_DELETED_TRANSACTIONS, mmGUIFrame::OnHideDeletedTransactions)
EVT_MENU(MENU_VIEW_BUDGET_FINANCIAL_YEARS, mmGUIFrame::OnViewBudgetFinancialYears)
EVT_MENU(MENU_VIEW_BUDGET_TRANSFER_TOTAL, mmGUIFrame::OnViewBudgetTransferTotal)
EVT_MENU(MENU_VIEW_BUDGET_CATEGORY_SUMMARY, mmGUIFrame::OnViewBudgetCategorySummary)
EVT_MENU(MENU_VIEW_IGNORE_FUTURE_TRANSACTIONS, mmGUIFrame::OnViewIgnoreFutureTransactions)
EVT_MENU(MENU_VIEW_SHOW_TOOLTIPS, mmGUIFrame::OnViewShowToolTips)
EVT_MENU(MENU_VIEW_SHOW_MONEYTIPS, mmGUIFrame::OnViewShowMoneyTips)

EVT_MENU(MENU_CATEGORY_RELOCATION, mmGUIFrame::OnCategoryRelocation)
EVT_MENU(MENU_PAYEE_RELOCATION, mmGUIFrame::OnPayeeRelocation)
EVT_MENU(MENU_TAG_RELOCATION, mmGUIFrame::OnTagRelocation)

EVT_UPDATE_UI(MENU_VIEW_TOOLBAR, mmGUIFrame::OnViewToolbarUpdateUI)
EVT_UPDATE_UI(MENU_VIEW_LINKS, mmGUIFrame::OnViewLinksUpdateUI)
EVT_MENU(MENU_TREEPOPUP_NEW, mmGUIFrame::OnNewTransaction)
EVT_MENU(MENU_TREEPOPUP_EDIT, mmGUIFrame::OnPopupEditAccount)
EVT_MENU(MENU_TREEPOPUP_REALLOCATE, mmGUIFrame::OnPopupReallocateAccount)
EVT_MENU(MENU_TREEPOPUP_DELETE, mmGUIFrame::OnPopupDeleteAccount)

EVT_MENU(MENU_TREEPOPUP_FILTER_DELETE, mmGUIFrame::OnPopupDeleteFilter)
EVT_MENU(MENU_TREEPOPUP_FILTER_RENAME, mmGUIFrame::OnPopupRenameFilter)
EVT_MENU(MENU_TREEPOPUP_FILTER_EDIT, mmGUIFrame::OnPopupEditFilter)

EVT_TREE_ITEM_MENU(wxID_ANY, mmGUIFrame::OnItemMenu)
EVT_TREE_ITEM_ACTIVATED(ID_NAVTREECTRL, mmGUIFrame::OnItemRightClick)
EVT_TREE_ITEM_EXPANDED(ID_NAVTREECTRL, mmGUIFrame::OnTreeItemExpanded)
EVT_TREE_ITEM_COLLAPSING(ID_NAVTREECTRL, mmGUIFrame::OnTreeItemCollapsing)
EVT_TREE_ITEM_COLLAPSED(ID_NAVTREECTRL, mmGUIFrame::OnTreeItemCollapsed)
EVT_TREE_KEY_DOWN(wxID_ANY, mmGUIFrame::OnKeyDown)

EVT_DROP_FILES(mmGUIFrame::OnDropFiles)

EVT_MENU(MENU_GOTOACCOUNT, mmGUIFrame::OnGotoAccount)
EVT_MENU(MENU_STOCKS, mmGUIFrame::OnGotoStocksAccount)

/* Navigation Panel */
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_NEW, mmGUIFrame::OnNewAccount)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_DELETE, mmGUIFrame::OnDeleteAccount)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_EDIT, mmGUIFrame::OnEditAccount)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_LIST, mmGUIFrame::OnAccountList)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_EXPORT2CSV, mmGUIFrame::OnExportToCSV)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_EXPORT2MMEX, mmGUIFrame::OnExportToMMEX)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_EXPORT2XML, mmGUIFrame::OnExportToXML)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_EXPORT2QIF, mmGUIFrame::OnExportToQIF)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_EXPORT2JSON, mmGUIFrame::OnExportToJSON)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_IMPORTUNIVCSV, mmGUIFrame::OnImportUniversalCSV)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_IMPORTXML, mmGUIFrame::OnImportXML)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_IMPORTQIF, mmGUIFrame::OnImportQIF)
EVT_MENU_RANGE(MENU_TREEPOPUP_ACCOUNT_VIEWALL, MENU_TREEPOPUP_ACCOUNT_VIEWCLOSED, mmGUIFrame::OnViewAccountsTemporaryChange)
EVT_MENU_RANGE(MENU_LANG + 1, MENU_LANG_MAX, mmGUIFrame::OnChangeGUILanguage)

/*Automatic processing of repeat transactions*/
EVT_TIMER(AUTO_REPEAT_TRANSACTIONS_TIMER_ID, mmGUIFrame::OnAutoRepeatTransactionsTimer)

/* Recent Files */
EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, mmGUIFrame::OnRecentFiles)
EVT_MENU(MENU_RECENT_FILES_CLEAR, mmGUIFrame::OnClearRecentFiles)
EVT_MENU(MENU_VIEW_TOGGLE_FULLSCREEN, mmGUIFrame::OnToggleFullScreen)
EVT_MENU(MENU_VIEW_RESET, mmGUIFrame::OnResetView)
EVT_CLOSE(mmGUIFrame::OnClose)

wxEND_EVENT_TABLE()
//----------------------------------------------------------------------------

const std::vector<std::pair<Model_Account::TYPE_ID, wxString> > mmGUIFrame::ACCOUNT_SECTION_TABLE =
{
    { Model_Account::TYPE_ID_CASH,        _n("Cash Accounts") },
    { Model_Account::TYPE_ID_CHECKING,    _n("Bank Accounts") },
    { Model_Account::TYPE_ID_CREDIT_CARD, _n("Credit Card Accounts") },
    { Model_Account::TYPE_ID_LOAN,        _n("Loan Accounts") },
    { Model_Account::TYPE_ID_TERM,        _n("Term Accounts") },
    { Model_Account::TYPE_ID_INVESTMENT,  _n("Stock Portfolios") },
    { Model_Account::TYPE_ID_ASSET,       _n("Assets") },
    { Model_Account::TYPE_ID_SHARES,      _n("Share Accounts") },
};
wxArrayString mmGUIFrame::ACCOUNT_SECTION = account_section_all();

wxArrayString mmGUIFrame::account_section_all()
{
    wxArrayString type_section;
    int i = 0;
    for (const auto& item : ACCOUNT_SECTION_TABLE) {
        wxASSERT_MSG(item.first == i++, "Wrong order in mmGUIFrame::ACCOUNT_SECTION_TABLE");
        type_section.Add(item.second);
    }
    return type_section;
}
//----------------------------------------------------------------------------

mmGUIFrame::mmGUIFrame(
    mmGUIApp* app,
    const wxString& title,
    const wxPoint& pos,
    const wxSize& size
) :
    wxFrame(nullptr, wxID_ANY, title, pos, size),
    m_app(app),
    autoRepeatTransactionsTimer_(this, AUTO_REPEAT_TRANSACTIONS_TIMER_ID)
{
    // tell wxAuiManager to manage this frame
    m_mgr.SetManagedWindow(this);
    SetIcon(mmex::getProgramIcon());
    SetMinSize(wxSize(480, 275));

#if (wxMAJOR_VERSION == 3 && wxMINOR_VERSION >= 1)
    // Initialize code to turn on Mac OS X Fullscreen capabilities (Lion and up only)
    // code is a noop for all other systems
    EnableFullScreenView(true);
#endif
    // decide if we need to show app start dialog
    bool from_scratch = false;
    wxFileName dbpath = m_app->GetOptParam();
    if (!dbpath.IsOk()) {
        from_scratch = Model_Setting::instance().getBool("SHOWBEGINAPP", true);
        if (from_scratch)
            dbpath = wxGetEmptyString();
        else
            dbpath = Model_Setting::instance().getLastDbPath();
    }

    //Read news, if checking enabled
    if (Option::instance().getCheckNews())
        getNewsRSS(websiteNewsArray_);

    /* Create the Controls for the frame */
    mmFontSize(this);
    LoadTheme();
    createMenu();    
    createControls();
    createToolBar();

#if wxUSE_STATUSBAR
    CreateStatusBar();
#endif // wxUSE_STATUSBAR
    m_recentFiles = new mmFileHistory(); // TODO Max files
    m_recentFiles->SetMenuPathStyle(wxFH_PATH_SHOW_ALWAYS);
    m_recentFiles->UseMenu(m_menuRecentFiles);
    m_recentFiles->LoadHistory();

    // Load perspective
    const wxString auiPerspective = Model_Setting::instance().getString("AUIPERSPECTIVE", wxEmptyString);
    m_mgr.LoadPerspective(auiPerspective);

    // add the toolbars to the manager
    m_mgr.AddPane(toolBar_, wxAuiPaneInfo()
        .Name("toolbar").ToolbarPane().Top()
        .LeftDockable(false).RightDockable(false)
        .Show(Model_Setting::instance().getBool("SHOWTOOLBAR", true))
        .DockFixed(false)                                               
        .Top()
        .MinSize(wxSize(-1, toolBar_->GetSize().GetHeight())) 
        .MaxSize(wxSize(-1, toolBar_->GetSize().GetHeight())) 
        .Position(0)
        .Row(0)
        .Layer(0)
        .Resizable(true)
        );                          

    // change look and feel of wxAuiManager
    m_mgr.GetArtProvider()->SetMetric(16, 0);
    m_mgr.GetArtProvider()->SetMetric(3, 1);

    // "commit" all changes made to wxAuiManager
    m_mgr.GetPane("Navigation").Caption(_t("Navigator"));
    m_mgr.GetPane("toolbar").Caption(_t("Toolbar"));
    m_mgr.Update();

    // Show license agreement at first open
    if (Model_Setting::instance().getString(INIDB_SEND_USAGE_STATS, "") == "") {
        mmAboutDialog(this, 4).ShowModal();
    }

    //Check for new version at startup
    if (Model_Setting::instance().getBool("UPDATECHECK", true))
        mmUpdate::checkUpdates(this, true);

    //Show appstart
    if (from_scratch || !dbpath.IsOk()) {
        menuEnableItems(false);
        showBeginAppDialog(dbpath.GetFullName().IsEmpty());
    }
    else {
        if (openFile(dbpath.GetFullPath(), false)) {
            DoRecreateNavTreeControl(true);
            //setHomePageActive(false);
            mmLoadColorsFromDatabase();
        }
        else {
            resetNavTreeControl();
            cleanupHomePanel();
            showBeginAppDialog(true);
        }
    }

    const wxAcceleratorEntry entries[] = {
        wxAcceleratorEntry(wxACCEL_NORMAL, WXK_F9, wxID_NEW),
        wxAcceleratorEntry(wxACCEL_NORMAL, WXK_F5, wxID_REFRESH),
    };

    wxAcceleratorTable tab(sizeof(entries) / sizeof(*entries), entries);
    SetAcceleratorTable(tab);

    if (m_db) {
        // Clean up deleted transactions
        autocleanDeletedTransactions();

        // Refresh stock quotes
        if (!Model_Stock::instance().all().empty() &&
            Model_Setting::instance().getBool("REFRESH_STOCK_QUOTES_ON_OPEN", false)
        ) {
            wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_RATES);
            this->GetEventHandler()->AddPendingEvent(evt);
        }
    }
}
//----------------------------------------------------------------------------

mmGUIFrame::~mmGUIFrame()
{
    try {
        cleanup();
    }
    catch (...) {
        wxASSERT(false);
    }

    // Report database statistics
    for (const auto & model : this->m_all_models) {
        model->show_statistics();
        Model_Usage::instance().AppendToCache(model->GetTableStatsAsJson());
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::cleanup()
{
    autoRepeatTransactionsTimer_.Stop();

    if (!m_filename.IsEmpty()) // Exiting before file is opened
        saveSettings();

    wxTreeItemId rootitem = m_nav_tree_ctrl->GetRootItem();
    cleanupNavTreeControl(rootitem);
    m_mgr.UnInit();

    /* Delete the GUI */
    cleanupHomePanel(false);
    ShutdownDatabase();
    CloseTheme();

    // Backup the database according to user requirements
    if (Option::instance().getDatabaseUpdated() &&
        Model_Setting::instance().getBool("BACKUPDB_UPDATE", false)
    ) {
        dbUpgrade::BackupDB(
            m_filename,
            dbUpgrade::BACKUPTYPE::CLOSE,
            Model_Setting::instance().getInt("MAX_BACKUP_FILES", 4)
        );
    }
}

void mmGUIFrame::ShutdownDatabase()
{
    if (!m_db)
        return;

    if (!Model_Infotable::instance().cache_.empty()) { //Cache empty on InfoTable means instance never initialized
        if (!db_lockInPlace)
            Model_Infotable::instance().setBool("ISUSED", false);
    }
    m_db->SetCommitHook(nullptr);
    m_db->Close();
    m_db.reset();
    for (auto& model : m_all_models)
        model->destroyCache();
}

void mmGUIFrame::resetNavTreeControl()
{
    wxTreeItemId root = m_nav_tree_ctrl->GetRootItem();
    cleanupNavTreeControl(root);
    m_nav_tree_ctrl->DeleteAllItems();
}

void mmGUIFrame::cleanupNavTreeControl(wxTreeItemId& item)
{
    while (item.IsOk()) {
        if (m_nav_tree_ctrl->ItemHasChildren(item)) {
            wxTreeItemIdValue cookie;
            wxTreeItemId childitem = m_nav_tree_ctrl->GetFirstChild(item, cookie);
            cleanupNavTreeControl(childitem);
        }
        mmTreeItemData* iData = dynamic_cast<mmTreeItemData*>(m_nav_tree_ctrl->GetItemData(item));
        m_nav_tree_ctrl->SetItemData(item, nullptr);
        if (iData)
            delete iData;

        if (item != m_nav_tree_ctrl->GetRootItem())
            item = m_nav_tree_ctrl->GetNextSibling(item);
        else
            break;
    }
}

//----------------------------------------------------------------------------
// process all events waiting in the event queue if any.
void mmGUIFrame::processPendingEvents()
{
    while (m_app->Pending())
        m_app->Dispatch();
}

//----------------------------------------------------------------------------
wxTreeItemId mmGUIFrame::getNavTreeChild(const wxTreeItemId& section, const wxString& childName) const
{
    wxTreeItemIdValue cookie;
    wxTreeItemId child = m_nav_tree_ctrl->GetFirstChild(section, cookie);
    while (child.IsOk()) {
        if (m_nav_tree_ctrl->GetItemText(child) == childName)
            break;
        child = m_nav_tree_ctrl->GetNextChild(section, cookie);
    }
    return child;
}

bool mmGUIFrame::setNavTreeSection(const wxString &sectionName)
{
    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);
    bool found = false;
    wxTreeItemId section = getNavTreeChild(m_nav_tree_ctrl->GetRootItem(), sectionName);
    if (section.IsOk()) {
        // Set the NavTreeCtrl and prevent any event code being executed for now.
        m_nav_tree_ctrl->SelectItem(section);
        //processPendingEvents();
        found = true;
    }
    m_nav_tree_ctrl->SetEvtHandlerEnabled(true);
    return found;
}

bool mmGUIFrame::setNavTreeSectionChild(const wxString& sectionName, const wxString& childName)
{
    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);
    bool found = false;
    wxTreeItemId section = getNavTreeChild(
        m_nav_tree_ctrl->GetRootItem(),
        sectionName
    );
    if (section.IsOk() && m_nav_tree_ctrl->ItemHasChildren(section)) {
        m_nav_tree_ctrl->ExpandAllChildren(section);
        wxTreeItemId child = getNavTreeChild(section, childName);
        if (child.IsOk()) {
            // Set the NavTreeCtrl and prevent any event code being executed for now.
            m_nav_tree_ctrl->SelectItem(child);
            //processPendingEvents();
            found = true;
        }
    }
    m_nav_tree_ctrl->SetEvtHandlerEnabled(true);
    return found;
}

void mmGUIFrame::setNavTreeAccount(const wxString& accountName)
{
    if (setNavTreeSectionChild(_t("Favorites"), accountName))
        return;
    static const std::vector<Model_Account::TYPE_ID> account_types = {
        Model_Account::TYPE_ID_CHECKING,
        Model_Account::TYPE_ID_CREDIT_CARD,
        Model_Account::TYPE_ID_CASH,
        Model_Account::TYPE_ID_LOAN,
        Model_Account::TYPE_ID_TERM,
        Model_Account::TYPE_ID_INVESTMENT,
        Model_Account::TYPE_ID_ASSET,
    };
    for (Model_Account::TYPE_ID account_type : account_types) {
        const wxString sectionName = wxGetTranslation(ACCOUNT_SECTION[account_type]);
        if (setNavTreeSectionChild(sectionName, accountName))
            return;
    }
}

//----------------------------------------------------------------------------
void mmGUIFrame::OnAutoRepeatTransactionsTimer(wxTimerEvent& /*event*/)
{
    //WebApp check
    if (mmWebApp::WebApp_CheckEnabled()) {
        if (OnRefreshWebApp(true)) {
            mmWebAppDialog dlg(this, true);
            dlg.ShowModal();
            if (dlg.getRefreshRequested())
                refreshPanelData();
        }
    }

    //Auto scheduled transaction
    bool continueExecution = false;

    Model_Billsdeposits& bills = Model_Billsdeposits::instance();
    for (const auto& q1 : bills.all()) {
        bills.decode_fields(q1);
        if (bills.autoExecuteManual() && bills.requireExecution()) {
            if (bills.allowExecution() && bills.AllowTransaction(q1)) {
                continueExecution = true;
                mmBDDialog repeatTransactionsDlg(this, q1.BDID, false, true);
                repeatTransactionsDlg.SetDialogHeader(_t("Auto Repeat Transactions"));
                if (repeatTransactionsDlg.ShowModal() == wxID_OK) {
                    refreshPanelData();
                }
                else // stop repeat executions from occuring
                    continueExecution = false;
            }
        }

        if (bills.autoExecuteSilent() && bills.requireExecution()) {
            if (bills.allowExecution() && bills.AllowTransaction(q1)) {
                continueExecution = true;
                Model_Checking::Data* tran = Model_Checking::instance().create();

                tran->ACCOUNTID = q1.ACCOUNTID;
                tran->TOACCOUNTID = q1.TOACCOUNTID;
                tran->PAYEEID = q1.PAYEEID;
                tran->TRANSCODE = q1.TRANSCODE;
                tran->TRANSAMOUNT = q1.TRANSAMOUNT;
                tran->TOTRANSAMOUNT = q1.TOTRANSAMOUNT;
                tran->STATUS = q1.STATUS;
                tran->TRANSACTIONNUMBER = q1.TRANSACTIONNUMBER;
                tran->NOTES = q1.NOTES;
                tran->CATEGID = q1.CATEGID;
                tran->FOLLOWUPID = q1.FOLLOWUPID;
                const wxDateTime payment_date = bills.TRANSDATE(q1);
                tran->TRANSDATE = payment_date.FormatISOCombined();
                tran->COLOR = q1.COLOR;
                int64 transID = Model_Checking::instance().save(tran);

                Model_Splittransaction::Cache checking_splits;
                std::vector<wxArrayInt64> splitTags;
                for (const auto &item : Model_Billsdeposits::split(q1)) {
                    Model_Splittransaction::Data *split = Model_Splittransaction::instance().create();
                    split->TRANSID = transID;
                    split->CATEGID = item.CATEGID;
                    split->SPLITTRANSAMOUNT = item.SPLITTRANSAMOUNT;
                    split->NOTES = item.NOTES;
                    checking_splits.push_back(split);
                    wxArrayInt64 tags;
                    for (const auto& tag : Model_Taglink::instance().find(
                        Model_Taglink::REFTYPE(Model_Attachment::REFTYPE_NAME_BILLSDEPOSITSPLIT),
                        Model_Taglink::REFID(item.SPLITTRANSID)
                    ))
                        tags.push_back(tag.TAGID);
                    splitTags.push_back(tags);
                }
                Model_Splittransaction::instance().save(checking_splits);

                // Save split tags
                const wxString& splitRefType = Model_Attachment::REFTYPE_NAME_TRANSACTIONSPLIT;

                for (size_t i = 0; i < checking_splits.size(); i++) {
                    Model_Taglink::Data_Set splitTaglinks;
                    for (const auto& tagId : splitTags.at(i)) {
                        Model_Taglink::Data* t = Model_Taglink::instance().create();
                        t->REFTYPE = splitRefType;
                        t->REFID = checking_splits[i]->SPLITTRANSID;
                        t->TAGID = tagId;
                        splitTaglinks.push_back(*t);
                    }
                    Model_Taglink::instance().update(splitTaglinks, splitRefType, checking_splits.at(i)->SPLITTRANSID);
                }

                // Copy the custom fields to the newly created transaction
                const auto& customDataSet = Model_CustomFieldData::instance().find(
                    Model_CustomFieldData::REFID(-q1.BDID)
                );
                Model_CustomFieldData::instance().Savepoint();
                for (const auto& entry : customDataSet) {
                    Model_CustomFieldData::Data* fieldData = Model_CustomFieldData::instance().create();
                    fieldData->FIELDID = entry.FIELDID;
                    fieldData->REFID = transID;
                    fieldData->CONTENT = entry.CONTENT;
                    Model_CustomFieldData::instance().save(fieldData);
                }
                Model_CustomFieldData::instance().ReleaseSavepoint();
                
                // Save base transaction tags
                Model_Taglink::Data_Set taglinks;
                const wxString& txnRefType = Model_Attachment::REFTYPE_NAME_TRANSACTION;
                for (const auto& tag : Model_Taglink::instance().find(
                    Model_Taglink::REFTYPE(Model_Attachment::REFTYPE_NAME_BILLSDEPOSIT),
                    Model_Taglink::REFID(q1.BDID)
                )) {
                    Model_Taglink::Data* t = Model_Taglink::instance().create();
                    t->REFTYPE = txnRefType;
                    t->REFID = transID;
                    t->TAGID = tag.TAGID;
                    taglinks.push_back(*t);
                }
                Model_Taglink::instance().update(taglinks, txnRefType, transID);
            }
            Model_Billsdeposits::instance().completeBDInSeries(q1.BDID);
        }
    }

    if (continueExecution)
        autoRepeatTransactionsTimer_.Start(5, wxTIMER_ONE_SHOT);
    else
        refreshPanelData();
}
//----------------------------------------------------------------------------

// Save our settings to ini db.
void mmGUIFrame::saveSettings()
{
    Model_Setting::instance().Savepoint();
    if (!m_filename.IsEmpty()) {
        wxFileName fname(m_filename);
        Model_Setting::instance().setString("LASTFILENAME", fname.GetFullPath());
    }
    /* Aui Settings */
    Model_Setting::instance().setString("AUIPERSPECTIVE", m_mgr.SavePerspective());

    // prevent values being saved while window is in an iconised state.
    if (this->IsIconized()) this->Restore();

    int value_x = 0, value_y = 0;
    this->GetPosition(&value_x, &value_y);
    Model_Setting::instance().setInt("ORIGINX", value_x);
    Model_Setting::instance().setInt("ORIGINY", value_y);

    int value_w = 0, value_h = 0;
    this->GetSize(&value_w, &value_h);
    Model_Setting::instance().setInt("SIZEW", value_w);
    Model_Setting::instance().setInt("SIZEH", value_h);
    Model_Setting::instance().setBool("ISMAXIMIZED", this->IsMaximized());
    Model_Setting::instance().ReleaseSavepoint();
}
//----------------------------------------------------------------------------

void mmGUIFrame::menuEnableItems(bool enable)
{
    menuBar_->FindItem(MENU_SAVE_AS)->Enable(enable);
    menuBar_->FindItem(MENU_EXPORT)->Enable(enable);
    menuBar_->FindItem(MENU_NEWACCT)->Enable(enable);
    menuBar_->FindItem(MENU_HOMEPAGE)->Enable(enable);
    menuBar_->FindItem(MENU_ACCTEDIT)->Enable(enable);
    menuBar_->FindItem(MENU_ACCOUNT_REALLOCATE)->Enable(enable);
    menuBar_->FindItem(MENU_ACCTDELETE)->Enable(enable);

    menuBar_->FindItem(MENU_ORGCATEGS)->Enable(enable);
    menuBar_->FindItem(MENU_ORGPAYEE)->Enable(enable);
    menuBar_->FindItem(MENU_CATEGORY_RELOCATION)->Enable(enable);
    menuBar_->FindItem(MENU_PAYEE_RELOCATION)->Enable(enable);
    menuBar_->FindItem(MENU_TAG_RELOCATION)->Enable(enable);
    menuBar_->FindItem(wxID_VIEW_LIST)->Enable(enable);
    menuBar_->FindItem(wxID_BROWSE)->Enable(enable);
    menuBar_->FindItem(MENU_CONVERT_ENC_DB)->Enable(enable);
    menuBar_->FindItem(MENU_RATES)->Enable(enable);
    menuBar_->FindItem(MENU_ORGTAGS)->Enable(enable);
    menuBar_->FindItem(MENU_THEME_MANAGER)->Enable(enable);

    menuBar_->FindItem(MENU_IMPORT)->Enable(enable);
    menuBar_->FindItem(wxID_PRINT)->Enable(enable);
    menuBar_->FindItem(wxID_PREFERENCES)->Enable(enable);
    menuBar_->FindItem(MENU_BILLSDEPOSITS)->Enable(enable);
    menuBar_->FindItem(MENU_CURRENCY)->Enable(enable);
    menuBar_->FindItem(MENU_ASSETS)->Enable(enable);
    menuBar_->FindItem(MENU_BUDGETSETUPDIALOG)->Enable(enable);
    menuBar_->FindItem(MENU_TRANSACTIONREPORT)->Enable(enable);
    menuBar_->FindItem(MENU_REFRESH_WEBAPP)->Enable(enable && mmWebApp::WebApp_CheckEnabled());

    menuBar_->FindItem(MENU_VIEW_HIDE_SHARE_ACCOUNTS)->Enable(enable);
    menuBar_->FindItem(MENU_VIEW_HIDE_DELETED_TRANSACTIONS)->Enable(enable);
    menuBar_->FindItem(MENU_VIEW_BUDGET_FINANCIAL_YEARS)->Enable(enable);
    menuBar_->FindItem(MENU_VIEW_BUDGET_TRANSFER_TOTAL)->Enable(enable);
    menuBar_->FindItem(MENU_VIEW_BUDGET_CATEGORY_SUMMARY)->Enable(enable);
    menuBar_->FindItem(MENU_VIEW_IGNORE_FUTURE_TRANSACTIONS)->Enable(enable);
    menuBar_->FindItem(MENU_VIEW_SHOW_TOOLTIPS)->Enable(enable);
    menuBar_->FindItem(MENU_VIEW_SHOW_MONEYTIPS)->Enable(enable);

    menuBar_->FindItem(MENU_DB_VACUUM)->Enable(enable);
    menuBar_->FindItem(MENU_DB_DEBUG)->Enable(enable);

    toolBar_->EnableTool(MENU_NEWACCT, enable);
    toolBar_->EnableTool(MENU_HOMEPAGE, enable);
    toolBar_->EnableTool(MENU_ORGPAYEE, enable);
    toolBar_->EnableTool(MENU_ORGCATEGS, enable);
    toolBar_->EnableTool(MENU_CURRENCY, enable);
    toolBar_->EnableTool(wxID_VIEW_LIST, enable);
    toolBar_->EnableTool(wxID_BROWSE, enable);
    toolBar_->EnableTool(MENU_TRANSACTIONREPORT, enable);
    toolBar_->EnableTool(wxID_PREFERENCES, enable);
    toolBar_->EnableTool(wxID_NEW, enable);
    toolBar_->EnableTool(wxID_PRINT, enable);
    toolBar_->EnableTool(MENU_ORGTAGS, enable);
    toolBar_->EnableTool(MENU_RATES, enable);
    toolBar_->Refresh();
    toolBar_->Update();
}
//----------------------------------------------------------------------------

void mmGUIFrame::menuPrintingEnable(bool enable)
{
    menuBar_->FindItem(wxID_PRINT)->Enable(enable);
    toolBar_->EnableTool(wxID_PRINT, enable);
    menuBar_->FindItem(MENU_EXPORT_HTML)->Enable(enable);
}
//----------------------------------------------------------------------------

void mmGUIFrame::createControls()
{
    m_nav_tree_ctrl = new wxTreeCtrl(
        this, ID_NAVTREECTRL, wxDefaultPosition, wxDefaultSize,
        wxTR_SINGLE | wxTR_HAS_BUTTONS | wxTR_NO_LINES | wxTR_TWIST_BUTTONS | wxTR_HIDE_ROOT | wxTR_LINES_AT_ROOT
    );

    m_nav_tree_ctrl->SetMinSize(wxSize(100, 100));
    mmThemeMetaColour(m_nav_tree_ctrl, meta::COLOR_NAVPANEL);
    mmThemeMetaColour(m_nav_tree_ctrl, meta::COLOR_NAVPANEL_FONT, true);

    const auto navIconSize = Option::instance().getNavigationIconSize();
    m_nav_tree_ctrl->SetImages(navtree_images_list(navIconSize));
    m_nav_tree_ctrl->SetIndent(10);

    m_nav_tree_ctrl->Connect(
        ID_NAVTREECTRL, wxEVT_TREE_SEL_CHANGED,
        wxTreeEventHandler(mmGUIFrame::OnSelChanged),
        nullptr, this
    );
    m_nav_tree_ctrl->Connect(
        ID_NAVTREECTRL, wxEVT_TREE_ITEM_RIGHT_CLICK,
        wxTreeEventHandler(mmGUIFrame::OnSelChanged),
        nullptr, this
    );
    
    homePanel_ = new wxPanel(
        this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxTAB_TRAVERSAL | wxTR_SINGLE | wxNO_BORDER
    );

    m_mgr.AddPane(m_nav_tree_ctrl, wxAuiPaneInfo()
        .Name("Navigation")
        .BestSize(wxSize(200, 100)).MinSize(wxSize(100, 100))
        .Left()
    );

    m_mgr.AddPane(homePanel_, wxAuiPaneInfo()
        .Name("Home").Caption("Home")
        .CenterPane().PaneBorder(false)
    );
}
//----------------------------------------------------------------------------

wxTreeItemId mmGUIFrame::addNavTreeSection(
    const wxTreeItemId& root, const wxString& sectionName, int sectionImg,
    int dataType, int64 dataId
) {
    wxTreeItemId section = m_nav_tree_ctrl->AppendItem(
        root, wxGetTranslation(sectionName), sectionImg, sectionImg
    );
    m_nav_tree_ctrl->SetItemData(
        section,
        new mmTreeItemData(dataType, dataId, sectionName)
    );
    m_nav_tree_ctrl->SetItemBold(section, true);
    return section;
}

void mmGUIFrame::DoRecreateNavTreeControl(bool home_page)
{
    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);
    if (home_page) {
        createHomePage();
    }

    DoWindowsFreezeThaw(m_nav_tree_ctrl);
    resetNavTreeControl();

    wxTreeItemId root = m_nav_tree_ctrl->AddRoot("Root");

    wxTreeItemId dashboard = addNavTreeSection(
        root, "Dashboard", img::HOUSE_PNG,
        mmTreeItemData::HOME_PAGE
    );

    addNavTreeSection(
        root, "All Transactions", img::ALLTRANSACTIONS_PNG,
        mmTreeItemData::CHECKING, -1
    );

    addNavTreeSection(
        root, "Scheduled Transactions", img::SCHEDULE_PNG,
        mmTreeItemData::BILLS
    );

    wxTreeItemId favorites = addNavTreeSection(
        root, "Favorites", img::FAVOURITE_PNG,
        mmTreeItemData::CHECKING, -3
    );

    wxTreeItemId accountSection[Model_Account::TYPE_ID_size];
    static const std::tuple<Model_Account::TYPE_ID, int> ACCOUNT_IMG_TABLE[] = {
        { Model_Account::TYPE_ID_CHECKING,    img::SAVINGS_ACC_NORMAL_PNG },
        { Model_Account::TYPE_ID_CREDIT_CARD, img::CARD_ACC_NORMAL_PNG },
        { Model_Account::TYPE_ID_CASH,        img::CASH_ACC_NORMAL_PNG },
        { Model_Account::TYPE_ID_LOAN,        img::LOAN_ACC_NORMAL_PNG },
        { Model_Account::TYPE_ID_TERM,        img::TERMACCOUNT_NORMAL_PNG },
        { Model_Account::TYPE_ID_INVESTMENT,  img::STOCK_ACC_NORMAL_PNG },
        { Model_Account::TYPE_ID_SHARES,      img::STOCK_ACC_NORMAL_PNG },
        { Model_Account::TYPE_ID_ASSET,       img::ASSET_NORMAL_PNG },
    };
    for (const auto& item : ACCOUNT_IMG_TABLE) {
        Model_Account::TYPE_ID itemId = std::get<0>(item);
        int itemImg = std::get<1>(item);
        int dataType =
            itemId == Model_Account::TYPE_ID_INVESTMENT ? mmTreeItemData::HELP_PAGE_STOCKS :
            itemId == Model_Account::TYPE_ID_ASSET      ? mmTreeItemData::ASSETS :
            mmTreeItemData::CHECKING;
        int64 dataId = dataType == mmTreeItemData::CHECKING ? -(4+itemId) : -1;
        accountSection[itemId] = addNavTreeSection(
            root, ACCOUNT_SECTION[itemId], itemImg,
            dataType, dataId
        );
    }

    // TODO: check mismatch between section name and search data
    wxTreeItemId budgeting = m_nav_tree_ctrl->AppendItem(
        root,
        wxGetTranslation("Budget Planner"),
        img::CALENDAR_PNG,
        img::CALENDAR_PNG
    );
    m_nav_tree_ctrl->SetItemData(
        budgeting,
        new mmTreeItemData(mmTreeItemData::HELP_BUDGET, "Budget Setup")
    );
    m_nav_tree_ctrl->SetItemBold(budgeting, true);
    this->DoUpdateBudgetNavigation(budgeting);

    wxTreeItemId transactionFilter = addNavTreeSection(
        root, "Transaction Report", img::FILTER_PNG,
        mmTreeItemData::FILTER
    );
    this->DoUpdateFilterNavigation(transactionFilter);

    wxTreeItemId reports = addNavTreeSection(
        root, "Reports", img::PIECHART_PNG,
        mmTreeItemData::HELP_REPORT
    );
    this->DoUpdateReportNavigation(reports);

    wxTreeItemId grm = addNavTreeSection(
        root, "General Report Manager", img::CUSTOMSQL_GRP_PNG,
        mmTreeItemData::HELP_PAGE_GRM
    );
    this->DoUpdateGRMNavigation(grm);

    wxTreeItemId trash = addNavTreeSection(
        root, "Deleted Transactions", img::TRASH_PNG,
        mmTreeItemData::CHECKING, -2
    );

    addNavTreeSection(
        root, "Help", img::HELP_PNG,
        mmTreeItemData::HELP_PAGE_MAIN
    );

    ///////////////////////////////////////////////////////////////////

    bool hideShareAccounts = Option::instance().getHideShareAccounts();

    if (m_db) {
        /* Start Populating the dynamic data */
        m_temp_view = Model_Setting::instance().getViewAccounts();
        wxASSERT(
            m_temp_view == VIEW_ACCOUNTS_ALL_STR ||
            m_temp_view == VIEW_ACCOUNTS_FAVORITES_STR ||
            m_temp_view == VIEW_ACCOUNTS_OPEN_STR ||
            m_temp_view == VIEW_ACCOUNTS_CLOSED_STR
        );

        for (const auto& account : Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME)) {
            if (m_temp_view == VIEW_ACCOUNTS_OPEN_STR &&
                Model_Account::status_id(account) != Model_Account::STATUS_ID_OPEN
            )
                continue;
            if (m_temp_view == VIEW_ACCOUNTS_CLOSED_STR &&
                Model_Account::status_id(account) == Model_Account::STATUS_ID_OPEN
            )
                continue;
            if (m_temp_view == VIEW_ACCOUNTS_FAVORITES_STR &&
                !Model_Account::FAVORITEACCT(account)
            )
                continue;

            Model_Account::TYPE_ID account_type = Model_Account::type_id(account);
            if (account_type == Model_Account::TYPE_ID_SHARES && hideShareAccounts)
                continue;

            int accountImg = Option::instance().AccountImageId(account.ACCOUNTID, false);

            wxTreeItemId accountItem;
            if (Model_Account::FAVORITEACCT(account) &&
                Model_Account::status_id(account) == Model_Account::STATUS_ID_OPEN &&
                Model_Account::type_id(account) != Model_Account::TYPE_ID_INVESTMENT
            ) {
                accountItem = m_nav_tree_ctrl->AppendItem(
                    favorites, account.ACCOUNTNAME, accountImg, accountImg
                );
                m_nav_tree_ctrl->SetItemData(
                    accountItem,
                    new mmTreeItemData(mmTreeItemData::CHECKING, account.ACCOUNTID)
                );
            }

            switch (account_type) {
            case Model_Account::TYPE_ID_CHECKING:
            case Model_Account::TYPE_ID_CREDIT_CARD:
            case Model_Account::TYPE_ID_CASH:
            case Model_Account::TYPE_ID_LOAN:
            case Model_Account::TYPE_ID_TERM:
            case Model_Account::TYPE_ID_SHARES:
            case Model_Account::TYPE_ID_ASSET:
                accountItem = m_nav_tree_ctrl->AppendItem(
                    accountSection[account_type],
                    account.ACCOUNTNAME,
                    accountImg, accountImg
                );
                m_nav_tree_ctrl->SetItemData(
                    accountItem,
                    new mmTreeItemData(mmTreeItemData::CHECKING, account.ACCOUNTID)
                );
                break;

            case Model_Account::TYPE_ID_INVESTMENT: {
                accountItem = m_nav_tree_ctrl->AppendItem(
                    accountSection[account_type],
                    account.ACCOUNTNAME,
                    accountImg, accountImg
                );
                m_nav_tree_ctrl->SetItemData(
                    accountItem,
                    new mmTreeItemData(mmTreeItemData::INVESTMENT, account.ACCOUNTID)
                );

                // Cash Ledger
                wxTreeItemId stockItem = m_nav_tree_ctrl->AppendItem(accountItem, _n("Cash Ledger"), accountImg, accountImg);
                m_nav_tree_ctrl->SetItemData(stockItem, new mmTreeItemData(mmTreeItemData::CHECKING, account.ACCOUNTID));

                // find all the accounts associated with this stock portfolio
                // just to keep compatibility for legacy Shares account data
                Model_Stock::Data_Set stocks = Model_Stock::instance().find(Model_Stock::HELDAT(account.ACCOUNTID));
                std::sort(stocks.begin(), stocks.end(), SorterBySTOCKNAME());

                // Put the names of the Stock_entry names as children of the stock account.
                std::unordered_set<wxString> processedStockNames;
                for (const auto& stock : stocks)
                {
                    if (!processedStockNames.insert(stock.STOCKNAME).second)
                        continue;
                    Model_Account::Data* share_account = Model_Account::instance().get(stock.STOCKNAME);
                    if (!share_account)
                        continue;
                    wxTreeItemId stockItem = m_nav_tree_ctrl->AppendItem(accountItem, stock.STOCKNAME, accountImg, accountImg);
                    m_nav_tree_ctrl->SetItemData(stockItem, new mmTreeItemData(mmTreeItemData::CHECKING, share_account->ACCOUNTID));
                }
                break;
            }

            default:
                wxASSERT(0);
                break;
            }
        }

        loadNavigationTreeItemsStatusFromJson();

        if (!m_nav_tree_ctrl->ItemHasChildren(favorites)) {
            m_nav_tree_ctrl->Delete(favorites);
        }
        for (const auto& item : ACCOUNT_IMG_TABLE) {
            Model_Account::TYPE_ID itemId = std::get<0>(item);
            if (itemId == Model_Account::TYPE_ID_ASSET)
                continue;
            if (!m_nav_tree_ctrl->ItemHasChildren(accountSection[itemId]) ||
                (itemId == Model_Account::TYPE_ID_SHARES && hideShareAccounts)
            )
                m_nav_tree_ctrl->Delete(accountSection[itemId]);
        }
        if (
            Model_Checking::instance().find(
                Model_Checking::DELETEDTIME(wxEmptyString, NOT_EQUAL)
            ).empty() ||
            Option::instance().getHideDeletedTransactions()
        ) {
            m_nav_tree_ctrl->Delete(trash);
            if (panelCurrent_ && panelCurrent_->GetId() == mmID_CHECKING) {
                mmCheckingPanel* cp = wxDynamicCast(panelCurrent_, mmCheckingPanel);
                if (cp->isDeletedTrans()) {
                    wxCommandEvent event(wxEVT_MENU, MENU_HOMEPAGE);
                    GetEventHandler()->AddPendingEvent(event);
                }
            }
        }
    }
    m_nav_tree_ctrl->EnsureVisible(dashboard);
    if (home_page) m_nav_tree_ctrl->SelectItem(dashboard);
    m_nav_tree_ctrl->SetEvtHandlerEnabled(true);
    m_nav_tree_ctrl->Refresh();
    m_nav_tree_ctrl->Update();

    /* issue #4778 */
#if !defined(__WXMSW__) 
    m_nav_tree_ctrl->SetFocus();
#endif

    DoWindowsFreezeThaw(m_nav_tree_ctrl);
}

void mmGUIFrame::loadNavigationTreeItemsStatusFromJson()
{
    /* Load Nav Tree Control */
    SetEvtHandlerEnabled(false);
    wxTreeItemId root = m_nav_tree_ctrl->GetRootItem();
    //m_nav_tree_ctrl->Expand(root);

    const wxString& str = Model_Infotable::instance().getString("NAV_TREE_STATUS", "");
    Document json_doc;
    if (json_doc.Parse(str.utf8_str()).HasParseError()) {
        json_doc.Parse("{}");
    }

    std::stack<wxTreeItemId> items;
    if (root.IsOk()) {
        items.push(root);
    }

    while (!items.empty()) {
        wxTreeItemId next = items.top();
        items.pop();

        wxTreeItemIdValue cookie;
        wxTreeItemId nextChild = m_nav_tree_ctrl->GetFirstChild(next, cookie);
        while (nextChild.IsOk()) {
            if (m_nav_tree_ctrl->HasChildren(nextChild)) items.push(nextChild);
            nextChild = m_nav_tree_ctrl->GetNextSibling(nextChild);
        }

        if (next == root)
            continue;
        mmTreeItemData* iData =
            dynamic_cast<mmTreeItemData*>(m_nav_tree_ctrl->GetItemData(next));
        if (!iData)
            continue;

        const wxString nav_key = iData->getString();
        if (json_doc.HasMember(nav_key.utf8_str())) {
            Value json_key(nav_key.utf8_str(), json_doc.GetAllocator());
            if (json_doc[json_key].IsBool() && json_doc[json_key].GetBool())
                m_nav_tree_ctrl->Expand(next);
        }
    }

    SetEvtHandlerEnabled(true);
}

void mmGUIFrame::OnTreeItemExpanded(wxTreeEvent& event)
{
    mmTreeItemData* iData =
        dynamic_cast<mmTreeItemData*>(m_nav_tree_ctrl->GetItemData(event.GetItem()));
    if (!iData) return;
    navTreeStateToJson();
}

void mmGUIFrame::OnTreeItemCollapsing(wxTreeEvent& /*event*/)
{
    /*
    mmTreeItemData* iData =
        dynamic_cast<mmTreeItemData*>(m_nav_tree_ctrl->GetItemData(event.GetItem()));
*/
    // disallow collapsing of HOME item
    //if (mmTreeItemData::HOME_PAGE == iData->getType())
    //    event.Veto();
}

void mmGUIFrame::OnTreeItemCollapsed(wxTreeEvent& event)
{
    mmTreeItemData* iData =
        dynamic_cast<mmTreeItemData*>(m_nav_tree_ctrl->GetItemData(event.GetItem()));
    if (!iData) return;
    navTreeStateToJson();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnDropFiles(wxDropFilesEvent& event)
{
    int id = panelCurrent_->GetId();
    if (id != mmID_CHECKING)
        return;
    mmCheckingPanel* cp = wxDynamicCast(panelCurrent_, mmCheckingPanel);
    if (!cp->isAllTrans() && !cp->isAccount())
        return;
    if (event.GetNumberOfFiles() <= 0)
        return;

    wxString* dropped = event.GetFiles();
    wxASSERT(dropped);
    wxArrayString files;
    for (int i = 0; i < event.GetNumberOfFiles(); i++) {
        wxString name = dropped[i];
        if (wxFileExists(name))
            files.push_back(name);
        else if (wxDirExists(name))
            wxDir::GetAllFiles(name, &files);
    }

    for (size_t i = 0; i < files.size(); i++) {
        wxString ext = wxFileName(files[i]).GetExt().MakeLower();
        if (ext == "csv" || ext == "xml") {
            mmUnivCSVDialog(
                this, (ext == "csv" ?
                    mmUnivCSVDialog::DIALOG_TYPE_IMPORT_CSV :
                    mmUnivCSVDialog::DIALOG_TYPE_IMPORT_XML
                ),
                gotoAccountID_, files[i]
            ).ShowModal();
        }
        else if (ext == "qif") {
            mmQIFImportDialog(this, gotoAccountID_, files[i]).ShowModal();
        }
    }
    refreshPanelData();
}

void mmGUIFrame::navTreeStateToJson()
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();

    wxTreeItemId root = m_nav_tree_ctrl->GetRootItem();
    std::stack<wxTreeItemId> items;
    if (root.IsOk())
        items.push(root);

    while (!items.empty()) {
        wxTreeItemId next = items.top();
        items.pop();

        wxTreeItemIdValue cookie;
        wxTreeItemId nextChild = m_nav_tree_ctrl->GetFirstChild(next, cookie);
        while (nextChild.IsOk()) {
            if (m_nav_tree_ctrl->HasChildren(nextChild)) items.push(nextChild);
            nextChild = m_nav_tree_ctrl->GetNextSibling(nextChild);
        }

        if (next == root)
            continue;
        mmTreeItemData* iData = dynamic_cast<mmTreeItemData*>(m_nav_tree_ctrl->GetItemData(next));
        if (iData && !iData->getString().empty() && m_nav_tree_ctrl->IsExpanded(next)) {
            json_writer.Key(iData->getString().utf8_str());
            json_writer.Bool(m_nav_tree_ctrl->IsExpanded(next));
        }
    };
    json_writer.EndObject();

    const wxString nav_tree_status = wxString::FromUTF8(json_buffer.GetString());
    wxLogDebug("=========== navTreeStateToJson =============================");
    wxLogDebug(nav_tree_status);
    Model_Infotable::instance().setString("NAV_TREE_STATUS", nav_tree_status);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnSelChanged(wxTreeEvent& event)
{
    if (!m_db)
        return;

    wxTreeItemId selectedItem = event.GetItem();
    if (!selectedItem)
        return;

    mmTreeItemData* iData = dynamic_cast<mmTreeItemData*>(m_nav_tree_ctrl->GetItemData(selectedItem));
    if (!iData)
        return;

    menuPrintingEnable(false);
    m_nav_tree_ctrl->Update();
    selectedItemData_ = iData;
    activeReport_ = false;
    gotoAccountID_ = -1;
    DragAcceptFiles(false);
    wxCommandEvent e;

    switch (iData->getType()) {
    case mmTreeItemData::DO_NOTHING:
        return;

    case mmTreeItemData::HOME_PAGE:
        return createHomePage();
    case mmTreeItemData::HELP_PAGE_MAIN:
        return createHelpPage();
    case mmTreeItemData::HELP_PAGE_STOCKS:
        return createHelpPage(mmex::HTML_INVESTMENT);
    case mmTreeItemData::HELP_PAGE_GRM:
        return createHelpPage(mmex::HTML_CUSTOM_SQL);
    case mmTreeItemData::HELP_BUDGET:
        return createHelpPage(mmex::HTML_BUDGET);
    case mmTreeItemData::HELP_REPORT:
        //createHelpPage(mmex::HTML_REPORTS);
        return;

    case  mmTreeItemData::CHECKING: {
        int64 id = iData->getId();
        std::vector<int64> group_ids = {};
        if (id <= -3) { // isGroup
            wxTreeItemIdValue cookie;
            wxTreeItemId child = m_nav_tree_ctrl->GetFirstChild(selectedItem, cookie);
            while (child.IsOk()) {
                mmTreeItemData* data = dynamic_cast<mmTreeItemData*>(
                    m_nav_tree_ctrl->GetItemData(child)
                );
                if (data->getType() == mmTreeItemData::CHECKING) {
                    int64 dataId = data->getId();
                    if (dataId >= 1) // isAccount
                        group_ids.push_back(dataId);
                }
                child = m_nav_tree_ctrl->GetNextChild(selectedItem, cookie);
            }
        }
        if (id == -1 || id >= 1) // isAllTrans, isAccount
            DragAcceptFiles(true);
        if (id >= 1) // isAccount
            gotoAccountID_ = id;
        return createCheckingPage(id, group_ids);
    }

    case mmTreeItemData::INVESTMENT: {
        Model_Account::Data* account = Model_Account::instance().get(iData->getId());
        gotoAccountID_ = account->ACCOUNTID;
        return createStocksAccountPage(gotoAccountID_);
    }
    case mmTreeItemData::ASSETS:
        return OnAssets(e);

    case mmTreeItemData::BILLS:
        return OnBillsDeposits(e);
    case mmTreeItemData::FILTER:
        return OnTransactionReport(e);
    case mmTreeItemData::FILTER_REPORT: {
        activeReport_ = true;
        wxSharedPtr<mmFilterTransactionsDialog> dlg(
            new mmFilterTransactionsDialog(this, iData->getString())
        );
        mmReportTransactions* rs = new mmReportTransactions(dlg);
        return createReportsPage(rs, true);
    }
    case mmTreeItemData::BUDGET:
        return createBudgetingPage(iData->getId());
    case mmTreeItemData::REPORT:
        activeReport_ = true;
        return createReportsPage(iData->getReport(), false);
    case mmTreeItemData::GRM:
        activeReport_ = true;
        return createReportsPage(iData->getReport(), false);
    }
    wxLogDebug("");
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnLaunchAccountWebsite(wxCommandEvent& /*event*/)
{
    if (!selectedItemData_)
        return;

    int64 id = selectedItemData_->getId();
    Model_Account::Data* account = Model_Account::instance().get(id);
    if (!account)
        return;

    wxString website = account->WEBSITE;
    if (!website.IsEmpty())
        wxLaunchDefaultBrowser(website);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnAccountAttachments(wxCommandEvent& /*event*/)
{
    if (!selectedItemData_)
        return;

    wxString refType = Model_Attachment::REFTYPE_NAME_BANKACCOUNT;
    int64 refId = selectedItemData_->getId();
    mmAttachmentDialog dlg(this, refType, refId);
    dlg.ShowModal();
}

//----------------------------------------------------------------------------
void mmGUIFrame::OnPopupEditAccount(wxCommandEvent& /*event*/)
{
    if (!selectedItemData_)
        return;

    int64 id = selectedItemData_->getId();
    Model_Account::Data* account = Model_Account::instance().get(id);
    if (!account)
        return;

    mmNewAcctDialog dlg(account, this);
    if (dlg.ShowModal() == wxID_OK)
        RefreshNavigationTree();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnPopupReallocateAccount(wxCommandEvent& WXUNUSED(event))
{
    if (!selectedItemData_)
        return;

    int64 id = selectedItemData_->getId();
    ReallocateAccount(id);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnPopupDeleteFilter(wxCommandEvent& /*event*/)
{
    if (!m_db)
        return;

    wxString data = selectedItemData_->getString();
    Document j_doc;
    if (j_doc.Parse(data.utf8_str()).HasParseError())
        j_doc.Parse("{}");
    Value& j_label = GetValueByPointerWithDefault(j_doc, "/LABEL", "");
    wxString selected = j_label.IsString() ? wxString::FromUTF8(j_label.GetString()) : "";

    if (wxMessageBox(
        _t("The selected item will be deleted.") + "\n\n" + _t("Do you want to continue?"),
        _t("Settings item deletion"),
        wxYES_NO | wxICON_WARNING
    ) == wxNO)
        return;

    int i = Model_Infotable::instance().findArrayItem("TRANSACTIONS_FILTER", selected);
    if (i != wxNOT_FOUND) {
        Model_Infotable::instance().eraseArrayItem("TRANSACTIONS_FILTER", i);
        RefreshNavigationTree();
    }
}
//--------------------------------------------------------------------------

void mmGUIFrame::OnPopupRenameFilter(wxCommandEvent& /*event*/)
{
    if (!m_db)
        return;

    wxString data = selectedItemData_->getString();
    Document j_doc;
    if (j_doc.Parse(data.utf8_str()).HasParseError())
        j_doc.Parse("{}");
    Value& j_label = GetValueByPointerWithDefault(j_doc, "/LABEL", "");
    wxString selected = j_label.IsString() ? wxString::FromUTF8(j_label.GetString()) : "";

    wxString new_name;
    bool nameOK = false;
    while (!nameOK) {
        new_name = wxGetTextFromUser(_t("Setting Name"), _t("Please Enter"), selected);
        if (new_name.empty())
            return;
        if (wxNOT_FOUND == Model_Infotable::instance().findArrayItem("TRANSACTIONS_FILTER", new_name))
            nameOK = true;
        else {
            wxString msgStr = wxString() << _t("A setting with this name already exists") << "\n"
                << "\n"
                << _t("Please specify a new name for the setting") << "\n";
            wxMessageBox(msgStr, _t("Name in use"), wxICON_ERROR);
        }
    }

    int i = Model_Infotable::instance().findArrayItem("TRANSACTIONS_FILTER", selected);
    if (i != wxNOT_FOUND) {
        Model_Infotable::instance().eraseArrayItem("TRANSACTIONS_FILTER", i);

        // Change the name
        Value::MemberIterator v_name = j_doc.FindMember("LABEL");
        v_name->value.SetString(new_name.utf8_str(), j_doc.GetAllocator());
        // Serialize the new entry
        StringBuffer buffer;
        buffer.Clear();
        PrettyWriter<StringBuffer> writer(buffer);
        writer.SetFormatOptions(kFormatSingleLineArray);
        j_doc.Accept(writer);
        data = wxString::FromUTF8(buffer.GetString());
        Model_Infotable::instance().prependArrayItem("TRANSACTIONS_FILTER", data, -1);

        DoRecreateNavTreeControl();
        setNavTreeSection(_t("Transaction Report"));
    }
}
//--------------------------------------------------------------------------

void mmGUIFrame::OnPopupEditFilter(wxCommandEvent& /*event*/)
{
    if (!m_db)
        return;
    if (Model_Account::instance().all().empty())
        return;

    wxString data = selectedItemData_->getString();

    const auto filter_settings = Model_Infotable::instance().getArrayString("TRANSACTIONS_FILTER");

    wxSharedPtr<mmFilterTransactionsDialog> dlg(new mmFilterTransactionsDialog(this, -1, true, data));
    bool is_ok = (dlg->ShowModal() == wxID_OK);
    if (filter_settings != Model_Infotable::instance().getArrayString("TRANSACTIONS_FILTER")) {
        DoRecreateNavTreeControl();
        setNavTreeSection(_t("Transaction Report"));
    }

    if (is_ok) {
        mmReportTransactions* rs = new mmReportTransactions(dlg);
        createReportsPage(rs, true);
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnPopupDeleteAccount(wxCommandEvent& /*event*/)
{
    if (!selectedItemData_)
        return;

    int64 id = selectedItemData_->getId();
    Model_Account::Data* account = Model_Account::instance().get(id);
    if (!account)
        return;

    wxString warning_msg = _t("Do you want to delete the account?");
    if (account->ACCOUNTTYPE == Model_Account::TYPE_NAME_INVESTMENT ||
        account->ACCOUNTTYPE == Model_Account::TYPE_NAME_SHARES
    ) {
        warning_msg += "\n\n" + _t("This will also delete any associated Shares.");
    }
    wxMessageDialog msgDlg(
        this, warning_msg, _t("Confirm Account Deletion"),
        wxYES_NO | wxNO_DEFAULT | wxICON_ERROR
    );
    if (msgDlg.ShowModal() == wxID_YES) {
        Model_Account::instance().remove(account->ACCOUNTID);
        mmAttachmentManage::DeleteAllAttachments(
            Model_Attachment::REFTYPE_NAME_BANKACCOUNT, account->ACCOUNTID
        );
        DoRecreateNavTreeControl(true);
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnItemMenu(wxTreeEvent& event)
{
    wxTreeItemId selectedItem = event.GetItem();
    m_nav_tree_ctrl->SelectItem(selectedItem);
    if (menuBar_->FindItem(MENU_ORGCATEGS)->IsEnabled())
        showTreePopupMenu(selectedItem, ScreenToClient(wxGetMousePosition()));
    else
        wxMessageBox(
            _t("MMEX has been opened without an active database."),
            _t("MMEX: Menu Popup Error"),
            wxOK | wxICON_EXCLAMATION
        );
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnItemRightClick(wxTreeEvent& event)
{
    wxTreeItemId selectedItem = event.GetItem();
    m_nav_tree_ctrl->SelectItem(selectedItem);

    OnSelChanged(event);
}
//----------------------------------------------------------------------------

void mmGUIFrame::AppendImportMenu(wxMenu& menu)
{
    wxMenu* importFrom(new wxMenu);
    menu.AppendSubMenu(importFrom, _t("&Import from"));
    importFrom->Append(MENU_TREEPOPUP_ACCOUNT_IMPORTUNIVCSV, _tu("&CSV Files…"));
    importFrom->Append(MENU_TREEPOPUP_ACCOUNT_IMPORTXML, _tu("&XML Files…"), _t("Import from XML file (Excel format)"));
    importFrom->AppendSeparator();
    importFrom->Append(MENU_TREEPOPUP_ACCOUNT_IMPORTQIF, _tu("&QIF Files…"));

    wxMenu* exportTo(new wxMenu);
    menu.AppendSubMenu(exportTo, _t("&Export as"));
    exportTo->Append(MENU_TREEPOPUP_ACCOUNT_EXPORT2CSV, _tu("&CSV File…"));
    exportTo->Append(MENU_TREEPOPUP_ACCOUNT_EXPORT2XML, _tu("&XML File…"));
    exportTo->AppendSeparator();
    exportTo->Append(MENU_TREEPOPUP_ACCOUNT_EXPORT2MMEX, _tu("&MMEX CSV File…"));
    exportTo->Append(MENU_TREEPOPUP_ACCOUNT_EXPORT2JSON, _tu("&JSON File…"));
    exportTo->Append(MENU_TREEPOPUP_ACCOUNT_EXPORT2QIF, _tu("&QIF File…"));
}

void mmGUIFrame::showTreePopupMenu(const wxTreeItemId& id, const wxPoint& pt)
{
    mmTreeItemData* iData = dynamic_cast<mmTreeItemData*>(m_nav_tree_ctrl->GetItemData(id));
    if (!iData)
        return;

    selectedItemData_ = iData;
    wxCommandEvent e;
    wxMenu menu;
    switch (iData->getType()) {
    case mmTreeItemData::HOME_PAGE:
        return OnThemeManager(e);
    case mmTreeItemData::HELP_BUDGET:
    case mmTreeItemData::BUDGET:
        return OnBudgetSetupDialog(e);
    case mmTreeItemData::FILTER:
        return OnTransactionReport(e);
    case mmTreeItemData::FILTER_REPORT: {
        const wxString data = iData->getString();
        wxLogDebug("MENU FILTER: %s", data);
        menu.Append(MENU_TREEPOPUP_FILTER_EDIT, _tu("&Customize Report…"));
        menu.Append(MENU_TREEPOPUP_FILTER_RENAME, _tu("&Rename Report…"));
        menu.Append(MENU_TREEPOPUP_FILTER_DELETE, _tu("&Delete Report…"));
        PopupMenu(&menu, pt);
        break;
    }
    case  mmTreeItemData::GRM:
    case  mmTreeItemData::HELP_PAGE_GRM:
        return OnGeneralReportManager(e);
    case mmTreeItemData::HELP_REPORT:
        return mmDoHideReportsDialog();
    case mmTreeItemData::INVESTMENT: {
        int64 acct_id = iData->getId();
        Model_Account::Data* account = Model_Account::instance().get(acct_id);
        if (account) {
            menu.Append(
                MENU_TREEPOPUP_EDIT,
                _tu("&Edit Account…")
            );
            menu.Append(
                MENU_TREEPOPUP_DELETE,
                _tu("&Delete Account…")
            );
            menu.AppendSeparator();
            menu.Append(MENU_TREEPOPUP_LAUNCHWEBSITE, _t("&Launch Account Website"));
            menu.Append(MENU_TREEPOPUP_ACCOUNTATTACHMENTS, _tu("&Attachment Manager…"));
            menu.Enable(MENU_TREEPOPUP_LAUNCHWEBSITE, !account->WEBSITE.IsEmpty());
            PopupMenu(&menu, pt);
        }
        break;
    }
    case mmTreeItemData::CHECKING: {
        int64 acct_id = iData->getId();
        if (acct_id >= 1) { // isAccount
            Model_Account::Data* account = Model_Account::instance().get(acct_id);
            if (!account)
                break;
            menu.Append(
                MENU_TREEPOPUP_EDIT,
                _tu("&Edit Account…")
            );
            menu.Append(
                MENU_TREEPOPUP_REALLOCATE,
                _tu("&Change Account Type…")
            );
            menu.AppendSeparator();
            menu.Append(
                MENU_TREEPOPUP_DELETE,
                _tu("&Delete Account…")
            );
            menu.AppendSeparator();
            menu.Append(MENU_TREEPOPUP_LAUNCHWEBSITE, _t("&Launch Account Website"));
            menu.Append(
                MENU_TREEPOPUP_ACCOUNTATTACHMENTS,
                _tu("&Attachment Manager…")
            );
            menu.Enable(MENU_TREEPOPUP_LAUNCHWEBSITE, !account->WEBSITE.IsEmpty());
            menu.Enable(MENU_TREEPOPUP_REALLOCATE, account->ACCOUNTTYPE != Model_Account::TYPE_NAME_SHARES && account->ACCOUNTTYPE != Model_Account::TYPE_NAME_INVESTMENT && account->ACCOUNTTYPE != Model_Account::TYPE_NAME_ASSET);
            menu.AppendSeparator();
            AppendImportMenu(menu);
            PopupMenu(&menu, pt);
        }
        else if (acct_id == -1 || acct_id <= -3) { // isAllTrans, isGroup
            menu.Append(
                MENU_TREEPOPUP_ACCOUNT_NEW,
                _tu("&New Account…")
            );
            menu.Append(
                MENU_TREEPOPUP_ACCOUNT_EDIT,
                _tu("&Edit Account…")
            );
            menu.Append(MENU_TREEPOPUP_ACCOUNT_LIST, _t("Account &List"));
            menu.AppendSeparator();
            menu.Append(
                MENU_TREEPOPUP_ACCOUNT_DELETE,
                _tu("&Delete Account…")
            );
            menu.AppendSeparator();

            AppendImportMenu(menu);

            menu.AppendSeparator();
            wxMenu* viewAccounts(new wxMenu);
            viewAccounts->AppendRadioItem(
                MENU_TREEPOPUP_ACCOUNT_VIEWALL,
                _t("&All")
            )->Check(m_temp_view == VIEW_ACCOUNTS_ALL_STR);
            viewAccounts->AppendRadioItem(
                MENU_TREEPOPUP_ACCOUNT_VIEWFAVORITE,
                _t("&Favorites")
            )->Check(m_temp_view == VIEW_ACCOUNTS_FAVORITES_STR);
            viewAccounts->AppendRadioItem(
                MENU_TREEPOPUP_ACCOUNT_VIEWOPEN,
                _t("&Open")
            )->Check(m_temp_view == VIEW_ACCOUNTS_OPEN_STR);
            viewAccounts->AppendRadioItem(
                MENU_TREEPOPUP_ACCOUNT_VIEWCLOSED,
                _t("&Closed")
            )->Check(m_temp_view == VIEW_ACCOUNTS_CLOSED_STR);
            menu.AppendSubMenu(viewAccounts, _t("Accounts &Visible"));

            PopupMenu(&menu, pt);
        }
        break;
    }
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnViewAccountsTemporaryChange(wxCommandEvent& e)
{
    int evt_id = e.GetId();
    //Get current settings for view accounts
    const wxString vAccts = Model_Setting::instance().getViewAccounts();
    if (m_temp_view.empty())
        m_temp_view = vAccts;

    //Set view ALL & Refresh Navigation Panel
    switch (evt_id) {
    case MENU_TREEPOPUP_ACCOUNT_VIEWALL:
        m_temp_view = VIEW_ACCOUNTS_ALL_STR;
        break;
    case MENU_TREEPOPUP_ACCOUNT_VIEWFAVORITE:
        m_temp_view = VIEW_ACCOUNTS_FAVORITES_STR;
        break;
    case MENU_TREEPOPUP_ACCOUNT_VIEWOPEN:
        m_temp_view = VIEW_ACCOUNTS_OPEN_STR;
        break;
    case MENU_TREEPOPUP_ACCOUNT_VIEWCLOSED:
        m_temp_view = VIEW_ACCOUNTS_CLOSED_STR;
        break;
    }
    Model_Setting::instance().setViewAccounts(m_temp_view);
    RefreshNavigationTree();

    //Restore settings
    Model_Setting::instance().setViewAccounts(vAccts);
}

//----------------------------------------------------------------------------

void mmGUIFrame::createMenu()
{
    wxMenu* menu_file = new wxMenu;

    wxMenuItem* menuItemNew = new wxMenuItem(menu_file, MENU_NEW, _tu("&New Database…"), _t("New Database"));
    wxMenuItem* menuItemOpen = new wxMenuItem(menu_file, MENU_OPEN, _tu("&Open Database…") + "\tCtrl-O", _t("Open Database"));
    wxMenuItem* menuItemSaveAs = new wxMenuItem(menu_file, MENU_SAVE_AS, _tu("Save Database &As…"), _t("Save Database As"));
    menu_file->Append(menuItemNew);
    menu_file->Append(menuItemOpen);
    menu_file->Append(menuItemSaveAs);
    menu_file->AppendSeparator();

    m_menuRecentFiles = new wxMenu;
    menu_file->Append(MENU_RECENT_FILES, _t("&Recent Files"), m_menuRecentFiles);
    wxMenuItem* menuClearRecentFiles = new wxMenuItem(menu_file, MENU_RECENT_FILES_CLEAR, _t("&Clear Recent Files"));
    menu_file->Append(menuClearRecentFiles);
    menu_file->AppendSeparator();

    wxMenu* importMenu = new wxMenu;
    menu_file->Append(MENU_IMPORT, _t("&Import from"), importMenu);
    importMenu->Append(MENU_IMPORT_UNIVCSV, _tu("&CSV File…"), _t("Import from CSV file"));
    importMenu->Append(MENU_IMPORT_XML, _tu("&XML File…"), _t("Import from XML file (Excel format)"));
    importMenu->AppendSeparator();
    importMenu->Append(MENU_IMPORT_QIF, _tu("&QIF File…"), _t("Import from QIF file"));
    importMenu->AppendSeparator();
    importMenu->Append(MENU_IMPORT_WEBAPP, _tu("&WebApp…"), _t("Import from the WebApp"));

    wxMenu* exportMenu = new wxMenu;
    menu_file->Append(MENU_EXPORT, _t("&Export as"), exportMenu);
    exportMenu->Append(MENU_EXPORT_CSV, _tu("&CSV File…"), _t("Export as CSV file"));
    exportMenu->Append(MENU_EXPORT_XML, _tu("&XML File…"), _t("Export as XML file"));
    exportMenu->AppendSeparator();
    exportMenu->Append(MENU_EXPORT_MMEX, _tu("&MMEX CSV File…"), _t("Export as fixed CSV file"));
    exportMenu->Append(MENU_EXPORT_JSON, _tu("&JSON File…"), _t("Export as JSON file"));
    exportMenu->Append(MENU_EXPORT_QIF, _tu("&QIF File…"), _t("Export as QIF file"));
    exportMenu->AppendSeparator();
    exportMenu->Append(MENU_EXPORT_HTML, _tu("&HTML File…"), _t("Export as HTML file"));

    menu_file->AppendSeparator();

    wxMenuItem* menuItemPrint = new wxMenuItem(menu_file, wxID_PRINT,
        _tu("&Print…"), _t("Print current view"));
    menu_file->Append(menuItemPrint);

    menu_file->AppendSeparator();

    wxMenuItem* menuItemQuit = new wxMenuItem(menu_file, wxID_EXIT,
        _t("E&xit") + "\tCtrl-Q", _t("Quit this program"));
    menu_file->Append(menuItemQuit);

    // Create the required menu items
    wxMenu* menuView = new wxMenu;
    wxMenuItem* menuItemToolbar = new wxMenuItem(
        menuView,
        MENU_VIEW_TOOLBAR,
        _t("&Toolbar"),
        _t("Show/Hide the toolbar"),
        wxITEM_CHECK
    );
    wxMenuItem* menuItemLinks = new wxMenuItem(
        menuView,
        MENU_VIEW_LINKS,
        _t("&Navigator") + "\tF5",
        _t("Show/Hide Navigator"),
        wxITEM_CHECK
    );
    wxMenuItem* menuItemHideShareAccounts = new wxMenuItem(
        menuView,
        MENU_VIEW_HIDE_SHARE_ACCOUNTS,
        _t("&Display Share Accounts"),
        _t("Show/Hide Share Accounts in Navigator"),
        wxITEM_CHECK
    );
    wxMenuItem* menuItemHideDeletedTransactions = new wxMenuItem(
        menuView,
        MENU_VIEW_HIDE_DELETED_TRANSACTIONS,
        _t("&Display Deleted Transactions"),
        _t("Show/Hide Deleted Transactions in Navigator"),
        wxITEM_CHECK
    );
    wxMenuItem* menuItemBudgetFinancialYears = new wxMenuItem(
        menuView,
        MENU_VIEW_BUDGET_FINANCIAL_YEARS,
        _t("Budgets: As Financial &Years"),
        _t("Display Budgets in Financial Year Format"),
        wxITEM_CHECK
    );
    wxMenuItem* menuItemBudgetTransferTotal = new wxMenuItem(
        menuView,
        MENU_VIEW_BUDGET_TRANSFER_TOTAL,
        _t("Budgets: &Include Transfers in Totals"),
        _t("Include the transfer transactions in the Budget Totals"),
        wxITEM_CHECK
    );
    wxMenuItem* menuItemBudgetCategorySummary = new wxMenuItem(
        menuView,
        MENU_VIEW_BUDGET_CATEGORY_SUMMARY,
        _t("Budget Category Report: with &Summaries"),
        _t("Include the category summaries in the Budget Category Summary"),
        wxITEM_CHECK
    );
    wxMenuItem* menuItemIgnoreFutureTransactions = new wxMenuItem(
        menuView,
        MENU_VIEW_IGNORE_FUTURE_TRANSACTIONS,
        _t("Ignore F&uture Transactions"),
        _t("Ignore Future transactions"),
        wxITEM_CHECK
    );
    wxMenuItem* menuItemShowToolTips = new wxMenuItem(
        menuView,
        MENU_VIEW_SHOW_TOOLTIPS,
        _t("Show T&ooltips"),
        _t("Show Tooltips"),
        wxITEM_CHECK
    );
    wxMenuItem* menuItemShowMoneyTips = new wxMenuItem(
        menuView,
        MENU_VIEW_SHOW_MONEYTIPS,
        _t("Show &Money Tips"),
        _t("Show Money Tips"),
        wxITEM_CHECK
    );

    //Add the menu items to the menu bar
    menuView->Append(menuItemToolbar);
    menuView->Append(menuItemLinks);
    menuView->Append(menuItemHideShareAccounts);
    menuView->Append(menuItemHideDeletedTransactions);
    menuView->Append(menuItemBudgetFinancialYears);
    menuView->Append(menuItemBudgetTransferTotal);
    menuView->AppendSeparator();
    menuView->Append(menuItemBudgetCategorySummary);
    menuView->AppendSeparator();
    menuView->Append(menuItemIgnoreFutureTransactions);
    menuView->AppendSeparator();
    menuView->Append(menuItemShowToolTips);
    menuView->Append(menuItemShowMoneyTips);
    menuView->AppendSeparator();

#if (wxMAJOR_VERSION >= 3 && wxMINOR_VERSION >= 0)
    wxMenuItem* menuItemToggleFullscreen = new wxMenuItem(
        menuView,
        MENU_VIEW_TOGGLE_FULLSCREEN,
        _t("&Full Screen") + "\tF11",
        _t("Toggle full screen"),
        wxITEM_CHECK
    );
    menuView->Append(menuItemToggleFullscreen);
#endif

    wxMenuItem* menuItemResetView = new wxMenuItem(
        menuView,
        MENU_VIEW_RESET,
        _t("&Reset View"),
        _t("Reset view and dock tools")
    );
    menuView->Append(menuItemResetView);   

    menuView->AppendSeparator();
    wxMenuItem* menuItemLanguage = new wxMenuItem(
        menuView,
        MENU_LANG,
        _t("User Interface &Language"),
        _t("Change user interface language")
    );
    wxMenu* menuLang = new wxMenu;

    wxArrayString lang_files = wxTranslations::Get()->GetAvailableTranslations("mmex");
    std::map<wxString, std::pair<int, wxString>> langs;
    menuLang->AppendRadioItem(MENU_LANG + 1 + wxLANGUAGE_DEFAULT, _t("System default"))
        ->Check(m_app->getGUILanguage() == wxLANGUAGE_DEFAULT);
    for (auto & file : lang_files) {
        const wxLanguageInfo* info = wxLocale::FindLanguageInfo(file);
        if (info)
            langs[wxGetTranslation(info->Description)] = std::make_pair(info->Language, info->CanonicalName);
    }
    langs[wxGetTranslation(wxLocale::GetLanguageName(wxLANGUAGE_ENGLISH_US))] = std::make_pair(wxLANGUAGE_ENGLISH_US, "en_US");
    for (auto const& lang : langs) {
        menuLang->AppendRadioItem(MENU_LANG + 1 + lang.second.first, lang.first, lang.second.second)
            ->Check(lang.second.first == m_app->getGUILanguage());
    }
    menuItemLanguage->SetSubMenu(menuLang);
    menuView->Append(menuItemLanguage);

    wxMenu* menuAccounts = new wxMenu;

    wxMenuItem* menuItemAcctList = new wxMenuItem(
        menuAccounts,
        MENU_HOMEPAGE,
        _t("Dash&board"),
        _t("Open Dashboard")
    );

    wxMenuItem* menuItemNewAcct = new wxMenuItem(
        menuAccounts,
        MENU_NEWACCT,
        _tu("&New Account…"),
        _t("New Account")
    );

    wxMenuItem* menuItemAcctEdit = new wxMenuItem(
        menuAccounts,
        MENU_ACCTEDIT,
        _tu("&Edit Account…"),
        _t("Edit Account")
    );

    wxMenuItem* menuItemReallocateAcct = new wxMenuItem(
        menuAccounts,
        MENU_ACCOUNT_REALLOCATE,
        _tu("&Change Account Type…"),
        _t("Change the account type of an account")
    );

    wxMenuItem* menuItemAcctDelete = new wxMenuItem(
        menuAccounts,
        MENU_ACCTDELETE,
        _tu("&Delete Account…"),
        _t("Delete Account from database")
    );

    menuAccounts->Append(menuItemAcctList);
    menuAccounts->AppendSeparator();
    menuAccounts->Append(menuItemNewAcct);
    menuAccounts->Append(menuItemAcctEdit);
    menuAccounts->Append(menuItemReallocateAcct);
    menuAccounts->Append(menuItemAcctDelete);

    // Tools Menu
    wxMenu* menuTools = new wxMenu;

    wxMenuItem* menuItemRates = new wxMenuItem(menuTools
        , MENU_RATES, _t("Download &Rates"), _t("Download Currency and Stock Rates"));
    menuTools->Append(menuItemRates);

    menuTools->AppendSeparator();

    wxMenuItem* menuItemPayee = new wxMenuItem(menuTools
        , MENU_ORGPAYEE, _tu("&Payee Manager…"), _t("Payee Manager"));
    menuTools->Append(menuItemPayee);

    wxMenuItem* menuItemCateg = new wxMenuItem(menuTools
        , MENU_ORGCATEGS, _tu("&Category Manager…"), _t("Category Manager"));
    menuTools->Append(menuItemCateg);

    wxMenuItem* menuItemTags = new wxMenuItem(menuTools
        , MENU_ORGTAGS, _tu("&Tag Manager…"), _t("Tag Manager"));
    menuTools->Append(menuItemTags);

    wxMenuItem* menuItemCurrency = new wxMenuItem(menuTools, MENU_CURRENCY
        , _tu("Curre&ncy Manager…"), _t("Currency Manager"));
    menuTools->Append(menuItemCurrency);

    wxMenuItem* menuItemCategoryRelocation = new wxMenuItem(menuTools
        , MENU_CATEGORY_RELOCATION, _tu("&Categories…")
        , _t("Merge categories"));
    wxMenuItem* menuItemPayeeRelocation = new wxMenuItem(menuTools
        , MENU_PAYEE_RELOCATION, _tu("&Payees…")
        , _t("Merge payees"));
    wxMenuItem* menuItemTagRelocation = new wxMenuItem(menuTools
        , MENU_TAG_RELOCATION, _tu("&Tags…")
        , _t("Merge tags"));
    wxMenuItem* menuItemRelocation = new wxMenuItem(menuTools
        , MENU_RELOCATION, _t("&Merge")
        , _t("Merge categories, payees, and tags"));
    wxMenu* menuRelocation = new wxMenu;
    menuRelocation->Append(menuItemPayeeRelocation);
    menuRelocation->Append(menuItemCategoryRelocation);
    menuRelocation->Append(menuItemTagRelocation);
    menuItemRelocation->SetSubMenu(menuRelocation);
    menuTools->Append(menuItemRelocation);

    menuTools->AppendSeparator();

    wxMenuItem* menuItemBudgeting = new wxMenuItem(menuTools, MENU_BUDGETSETUPDIALOG
        , _tu("&Budget Planner…"), _t("Budget Planner"));
    menuTools->Append(menuItemBudgeting);

    wxMenuItem* menuItemBillsDeposits = new wxMenuItem(menuTools, MENU_BILLSDEPOSITS
        , _t("&Scheduled Transactions"), _t("Bills and Deposits"));
    menuTools->Append(menuItemBillsDeposits);

    wxMenuItem* menuItemAssets = new wxMenuItem(menuTools, MENU_ASSETS
        , _t("&Assets"), _t("Assets"));
    menuTools->Append(menuItemAssets);

    menuTools->AppendSeparator();

    wxMenuItem* menuItemThemes = new wxMenuItem(menuTools, MENU_THEME_MANAGER
        , _tu("T&heme Manager…"), _t("Theme Manager"));
    menuTools->Append(menuItemThemes);

    menuTools->AppendSeparator();

    wxMenuItem* menuItemTransactions = new wxMenuItem(menuTools, MENU_TRANSACTIONREPORT
        , _tu("Tra&nsaction Report…"), _t("Transaction Report"));
    menuTools->Append(menuItemTransactions);

    menuTools->AppendSeparator();

    wxMenuItem* menuItemGRM = new wxMenuItem(menuTools, wxID_VIEW_LIST
        , _tu("&General Report Manager…"), _t("General Report Manager"));
    menuTools->Append(menuItemGRM);

    wxMenuItem* menuItemCF = new wxMenuItem(menuTools, wxID_BROWSE
        , _tu("C&ustom Field Manager…"), _t("Custom Field Manager"));
    menuTools->Append(menuItemCF);

    menuTools->AppendSeparator();

    wxMenuItem* menuItemWA = new wxMenuItem(menuTools, MENU_REFRESH_WEBAPP
        , _tu("Refresh &WebApp…"), _t("Refresh WebApp"));
    menuTools->Append(menuItemWA);
    menuTools->AppendSeparator();

    wxMenuItem* menuItemOptions = new wxMenuItem(menuTools, wxID_PREFERENCES
        , _tu("&Settings…") + "\tAlt-F12", _t("Settings"));
    menuTools->Append(menuItemOptions);

    menuTools->AppendSeparator();

    wxMenu* menuDatabase = new wxMenu;
    wxMenuItem* menuItemConvertDB = new wxMenuItem(menuTools, MENU_CONVERT_ENC_DB
        , _tu("&Decrypt Encrypted Database…")
        , _t("Convert encrypted database to unencrypted database"));
    wxMenuItem* menuItemChangeEncryptPassword = new wxMenuItem(menuTools, MENU_CHANGE_ENCRYPT_PASSWORD
        , _tu("Change Encrypted &Password…")
        , _t("Change the password of an encrypted database"));
    wxMenuItem* menuItemVacuumDB = new wxMenuItem(menuTools, MENU_DB_VACUUM
        , _tu("&Optimize Database…")
        , _t("Optimize database for space and performance"));
    wxMenuItem* menuItemCheckDB = new wxMenuItem(menuTools, MENU_DB_DEBUG
        , _tu("Database Check and De&bug…")
        , _t("Generate database report or fix errors"));
    menuDatabase->Append(menuItemConvertDB);
    menuDatabase->Append(menuItemChangeEncryptPassword);
    menuDatabase->Append(menuItemVacuumDB);
    menuDatabase->Append(menuItemCheckDB);
    menuTools->AppendSubMenu(menuDatabase, _t("&Database")
        , _t("Database management"));
    menuItemChangeEncryptPassword->Enable(false);

    // Help Menu
    wxMenu* menuHelp = new wxMenu;

    wxMenuItem* menuItemHelp = new wxMenuItem(menuTools, wxID_HELP,
        _t("&Help") + "\tF1", _t("Show Help"));
    menuHelp->Append(menuItemHelp);

    //Community Submenu
    wxMenuItem* menuItemWebsite = new wxMenuItem(menuHelp, MENU_WEBSITE
        , _t("&Website")
        , _t("Visit MMEX website for the latest news and updates"));
    wxMenuItem* menuItemFacebook = new wxMenuItem(menuHelp, MENU_FACEBOOK
        , _t("Face&book"), _t("Visit MMEX Facebook page"));
    wxMenuItem* menuItemCrowdin = new wxMenuItem(menuHelp, MENU_CROWDIN
        , _t("&Crowdin"), _t("Help translate MMEX on Crowdin"));
    wxMenuItem* menuItemTwitter = new wxMenuItem(menuHelp, MENU_TWITTER
        , _t("&X"), _t("Follow MMEX on X"));
    wxMenuItem* menuItemYouTube = new wxMenuItem(menuHelp, MENU_YOUTUBE
        , _t("&YouTube"), _t("Watch MMEX videos on YouTube"));
    wxMenuItem* menuItemSlack = new wxMenuItem(menuHelp, MENU_SLACK
        , _t("&Slack"), _t("Communicate with the MMEX team online"));
    wxMenuItem* menuItemGitHub = new wxMenuItem(menuHelp, MENU_GITHUB
        , _t("&GitHub"), _t("Access open source code repository and track reported bug statuses"));
    wxMenuItem* menuItemWiki = new wxMenuItem(menuHelp, MENU_WIKI
        , _t("Wi&ki"), _t("Read and update MMEX wiki"));
    wxMenuItem* menuItemReportIssues = new wxMenuItem(menuHelp, MENU_REPORTISSUES
        , _t("&Forum")
        , _t("Visit MMEX forum to read and post comments and for support"));
    wxMenuItem* menuItemAppleAppStore = new wxMenuItem(menuHelp, MENU_APPLE_APPSTORE
        , _t("MMEX for &iOS")
        , _t("Install MMEX on iPads and iPhones"));
    wxMenuItem* menuItemGooglePlay = new wxMenuItem(menuHelp, MENU_GOOGLEPLAY
        , _t("MMEX for &Android")
        , _t("Install MMEX for Android based smartphones and tablets"));
    wxMenuItem* menuItemNotify = new wxMenuItem(menuHelp, MENU_ANNOUNCEMENTMAILING
        , _t("&Newsletter")
        , _t("Visit the MMEX blog to view announcements"));
    wxMenuItem* menuItemRSS = new wxMenuItem(menuHelp, MENU_RSS
        , _t("&RSS Feed"), _t("Connect RSS web feed to news aggregator"));
    wxMenuItem* menuItemDonate = new wxMenuItem(menuHelp, MENU_DONATE
        , _t("Make a &Donation")
        , _t("Donate to the MMEX team to support infrastructure and development"));
    wxMenuItem* menuItemBuyCoffee = new wxMenuItem(menuHelp, MENU_BUY_COFFEE
        , _t("&Buy us a Coffee")
        , _t("Buy a key developer a coffee"));

    wxMenuItem* menuItemCommunity = new wxMenuItem(menuHelp, MENU_COMMUNITY
        , _t("&Community")
        , _t("Stay in touch with MMEX community"));

    wxMenu* menuCommunity = new wxMenu;
    menuCommunity->Append(menuItemWebsite);
    menuCommunity->Append(menuItemGitHub);
    menuCommunity->Append(menuItemCrowdin);
    menuCommunity->Append(menuItemWiki);
    menuCommunity->Append(menuItemNotify);
    menuCommunity->Append(menuItemReportIssues);
    menuCommunity->AppendSeparator();
    menuCommunity->Append(menuItemAppleAppStore);
    menuCommunity->Append(menuItemGooglePlay);
    menuCommunity->AppendSeparator();
    menuCommunity->Append(menuItemRSS);
    menuCommunity->Append(menuItemYouTube);
    menuCommunity->Append(menuItemFacebook);
    menuCommunity->Append(menuItemSlack);
    menuCommunity->Append(menuItemTwitter);
    menuCommunity->AppendSeparator();
    menuCommunity->Append(menuItemDonate);
    menuCommunity->Append(menuItemBuyCoffee);
    menuItemCommunity->SetSubMenu(menuCommunity);
    menuHelp->Append(menuItemCommunity);

    wxMenuItem* menuItemReportBug = new wxMenuItem(menuTools, MENU_REPORT_BUG
        , _t("Report a &Bug")
        , _t("Report an error in application to the developers"));
    menuHelp->Append(menuItemReportBug);

    wxMenuItem* menuItemDiagnostics = new wxMenuItem(menuTools, MENU_DIAGNOSTICS
        , _t("View &Diagnostics")
        , _t("Help provide information to the developers"));
    menuHelp->Append(menuItemDiagnostics);

    wxMenuItem* menuItemAppStart = new wxMenuItem(menuTools, MENU_SHOW_APPSTART
        , _t("&Show App Start Dialog"), _t("App Start Dialog"));
    menuHelp->Append(menuItemAppStart);

    wxMenuItem* menuItemCheck = new wxMenuItem(menuTools, MENU_CHECKUPDATE
        , _t("Check for &Updates"), _t("Check For Updates"));
    menuHelp->Append(menuItemCheck);

    wxMenuItem* menuItemAbout = new wxMenuItem(menuTools, wxID_ABOUT
        , _t("&About"), _t("About dialog"));
    menuHelp->Append(menuItemAbout);

    menuBar_ = new wxMenuBar;
    menuBar_->Append(menu_file, _t("&File"));
    menuBar_->Append(menuAccounts, _t("&Accounts"));
    menuBar_->Append(menuTools, _t("&Tools"));
    menuBar_->Append(menuView, _t("&View"));
    menuBar_->Append(menuHelp, _t("&Help"));
    SetMenuBar(menuBar_);

    menuBar_->Check(MENU_VIEW_HIDE_SHARE_ACCOUNTS, !Option::instance().getHideShareAccounts());
    menuBar_->Check(MENU_VIEW_HIDE_DELETED_TRANSACTIONS, !Option::instance().getHideDeletedTransactions());
    menuBar_->Check(MENU_VIEW_BUDGET_FINANCIAL_YEARS, Option::instance().getBudgetFinancialYears());
    menuBar_->Check(MENU_VIEW_BUDGET_TRANSFER_TOTAL, Option::instance().getBudgetIncludeTransfers());
    menuBar_->Check(MENU_VIEW_BUDGET_CATEGORY_SUMMARY, Option::instance().getBudgetSummaryWithoutCategories());
    menuBar_->Check(MENU_VIEW_IGNORE_FUTURE_TRANSACTIONS, Option::instance().getIgnoreFutureTransactions());
    menuBar_->Check(MENU_VIEW_SHOW_TOOLTIPS, Option::instance().getShowToolTips());
    menuBar_->Check(MENU_VIEW_SHOW_MONEYTIPS, Option::instance().getShowMoneyTips());
}
//----------------------------------------------------------------------------

void mmGUIFrame::createToolBar()
{
    const int toolbar_icon_size = Option::instance().getToolbarIconSize();
    const long style = wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_HORIZONTAL | wxAUI_TB_PLAIN_BACKGROUND;

    toolBar_ = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, style);
    toolBar_->SetToolBorderPadding(1);
    mmThemeMetaColour(toolBar_, meta::COLOR_LISTPANEL);
    //toolBar_->SetBackgroundColour(wxColor(*wxRED));
    //toolBar_->SetToolBitmapSize(wxSize(toolbar_icon_size, toolbar_icon_size));  // adjust tool size to match the icon size being used

    toolBar_->AddTool(MENU_NEW, _t("New"), mmBitmapBundle(png::NEW_DB, toolbar_icon_size), _t("New Database"));
    toolBar_->AddTool(MENU_OPEN, _t("Open"), mmBitmapBundle(png::OPEN, toolbar_icon_size), _t("Open Database"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(MENU_NEWACCT, _t("New Account"), mmBitmapBundle(png::NEW_ACC, toolbar_icon_size), _t("New Account"));
    toolBar_->AddTool(MENU_HOMEPAGE, _t("Dashboard"), mmBitmapBundle(png::HOME, toolbar_icon_size), _t("Open Dashboard"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(wxID_NEW, _t("New"), mmBitmapBundle(png::NEW_TRX, toolbar_icon_size), _t("New Transaction"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(MENU_ORGPAYEE, _t("Payee Manager"), mmBitmapBundle(png::PAYEE, toolbar_icon_size), _t("Payee Manager"));
    toolBar_->AddTool(MENU_ORGCATEGS, _t("Category Manager"), mmBitmapBundle(png::CATEGORY, toolbar_icon_size), _t("Category Manager"));
    toolBar_->AddTool(MENU_ORGTAGS, _t("Tag Manager"), mmBitmapBundle(png::TAG, toolbar_icon_size), _t("Tag Manager"));
    toolBar_->AddTool(MENU_CURRENCY, _t("Currency Manager"), mmBitmapBundle(png::CURR, toolbar_icon_size), _t("Currency Manager"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(MENU_TRANSACTIONREPORT, _t("Transaction Report"), mmBitmapBundle(png::FILTER, toolbar_icon_size), _t("Transaction Report"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(wxID_VIEW_LIST, _t("General Report Manager"), mmBitmapBundle(png::GRM, toolbar_icon_size), _t("General Report Manager"));

    toolBar_->AddTool(MENU_RATES, _t("Download Rates"), mmBitmapBundle(png::CURRATES, toolbar_icon_size), _t("Download currency and stock rates"));

   
    toolBar_->AddSeparator();
    toolBar_->AddTool(wxID_PRINT, _t("&Print"), mmBitmapBundle(png::PRINT, toolbar_icon_size), _t("Print"));
    toolBar_->AddSeparator();

    toolBar_->AddStretchSpacer();
    toolBar_->AddSeparator();
    toolBar_->AddTool(MENU_VIEW_TOGGLE_FULLSCREEN, _t("Full Screen") + "\tF11", mmBitmapBundle(png::FULLSCREEN, toolbar_icon_size), _t("Toggle full screen"));
    toolBar_->AddTool(wxID_PREFERENCES, _t("&Settings"), mmBitmapBundle(png::OPTIONS, toolbar_icon_size), _t("Settings"));
    toolBar_->AddSeparator();


    wxString news_array;
    for (const auto& entry : websiteNewsArray_)
    {
        news_array += entry.Title + "\n";
    }
    if (news_array.empty())
    {
        news_array = _t("News");
    }
    const auto news_ico = (websiteNewsArray_.size() > 0) ? mmBitmapBundle(png::NEW_NEWS, toolbar_icon_size) : mmBitmapBundle(png::NEWS, toolbar_icon_size);

    toolBar_->AddTool(MENU_ANNOUNCEMENTMAILING, _t("News"), news_ico, news_array);
    toolBar_->AddTool(wxID_ABOUT, _t("&About"), mmBitmapBundle(png::ABOUT, toolbar_icon_size), _t("About"));
    toolBar_->AddTool(wxID_HELP, _t("&Help") + "\tF1", mmBitmapBundle(png::HELP, toolbar_icon_size), _t("Help"));

    // after adding the buttons to the toolbar, must call Realize() to reflect changes
    toolBar_->Realize();
}
//----------------------------------------------------------------------------

void mmGUIFrame::InitializeModelTables()
{
    m_all_models.push_back(&Model_Infotable::instance(m_db.get()));
    m_all_models.push_back(&Model_Asset::instance(m_db.get()));
    m_all_models.push_back(&Model_Stock::instance(m_db.get()));
    m_all_models.push_back(&Model_StockHistory::instance(m_db.get()));
    m_all_models.push_back(&Model_Account::instance(m_db.get()));
    m_all_models.push_back(&Model_Payee::instance(m_db.get()));
    m_all_models.push_back(&Model_Checking::instance(m_db.get()));
    m_all_models.push_back(&Model_Currency::instance(m_db.get()));
    m_all_models.push_back(&Model_CurrencyHistory::instance(m_db.get()));
    m_all_models.push_back(&Model_Budgetyear::instance(m_db.get()));
    m_all_models.push_back(&Model_Category::instance(m_db.get()));
    m_all_models.push_back(&Model_Billsdeposits::instance(m_db.get()));
    m_all_models.push_back(&Model_Splittransaction::instance(m_db.get()));
    m_all_models.push_back(&Model_Budgetsplittransaction::instance(m_db.get()));
    m_all_models.push_back(&Model_Budget::instance(m_db.get()));
    m_all_models.push_back(&Model_Report::instance(m_db.get()));
    m_all_models.push_back(&Model_Attachment::instance(m_db.get()));
    m_all_models.push_back(&Model_CustomFieldData::instance(m_db.get()));
    m_all_models.push_back(&Model_CustomField::instance(m_db.get()));
    m_all_models.push_back(&Model_Tag::instance(m_db.get()));
    m_all_models.push_back(&Model_Taglink::instance(m_db.get()));
    m_all_models.push_back(&Model_Translink::instance(m_db.get()));
    m_all_models.push_back(&Model_Shareinfo::instance(m_db.get()));
}

bool mmGUIFrame::createDataStore(const wxString& fileName, const wxString& pwd, bool openingNew)
{
    if (m_db) {
        ShutdownDatabase();
        // Backup the database according to user requirements
        if (Option::instance().getDatabaseUpdated() &&
            Model_Setting::instance().getBool("BACKUPDB_UPDATE", false)
        ) {
            dbUpgrade::BackupDB(
                m_filename,
                dbUpgrade::BACKUPTYPE::CLOSE,
                Model_Setting::instance().getInt("MAX_BACKUP_FILES", 4)
            );
            Option::instance().setDatabaseUpdated(false);
        }
    }

    wxFileName checkExt(fileName);
    wxString password;
    bool passwordCheckPassed = true;
    if (checkExt.GetExt().Lower() == "emb" && wxFileName::FileExists(fileName)) {
        wxString password_message = wxString::Format(
            _t("Please enter password for Database\n\n%s"),
            fileName
        );
        password = !pwd.empty() ? pwd :
            wxGetPasswordFromUser(password_message, _t("MMEX: Encrypted Database")
        );
        if (password.IsEmpty())
            passwordCheckPassed = false;
    }

    const wxString dialogErrorMessageHeading = _t("Opening MMEX Database – Error");

    // Existing Database
    if (!openingNew
        && !fileName.IsEmpty()
        && wxFileName::FileExists(fileName)
        && passwordCheckPassed
    ) {
        /* Do a backup before opening */
        if (Model_Setting::instance().getBool("BACKUPDB", false)) {
            dbUpgrade::BackupDB(
                fileName,
                dbUpgrade::BACKUPTYPE::START,
                Model_Setting::instance().getInt("MAX_BACKUP_FILES", 4)
            );
        }

        m_db = mmDBWrapper::Open(fileName, password);
        // if the database pointer has been reset, the password is possibly incorrect
        if (!m_db)
            return false;

        m_commit_callback_hook = new CommitCallbackHook();
        m_db->SetCommitHook(m_commit_callback_hook.get());
        m_update_callback_hook = new UpdateCallbackHook();
        m_db->SetUpdateHook(m_update_callback_hook.get());

        //Check if DB upgrade needed
        if (dbUpgrade::isUpgradeDBrequired(m_db.get())) {
            // close & reopen database in debug mode for upgrade (bypassing SQLITE_CorruptRdOnly flag)
            ShutdownDatabase();
            m_db = mmDBWrapper::Open(fileName, password, true);
            //DB backup is handled inside UpgradeDB
            if (!dbUpgrade::UpgradeDB(m_db.get(), fileName)) {
                int response = wxMessageBox(_t("Have MMEX support provided a debug/patch file?"), _t("MMEX upgrade"), wxYES_NO);
                if (response == wxYES) {
                    // upgrade failure turns CorruptRdOnly flag back on, so reopen again in debug mode
                    ShutdownDatabase();
                    m_db = mmDBWrapper::Open(fileName, password, true);
                    dbUpgrade::SqlFileDebug(m_db.get());
                }
                ShutdownDatabase();
                return false;
            }
        }

        InitializeModelTables();

        wxString UID = Model_Infotable::instance().getString("UID", wxEmptyString);
        if (UID.IsEmpty()) {
            UID = Model_Setting::instance().getString("UUID", wxEmptyString);
            Model_Infotable::instance().setString("UID", UID);
        }
        Model_Setting::instance().setString("UID", UID);

        // ** OBSOLETE **
        // Mantained only for really old compatibility reason and replaced by dbupgrade.cpp
        if (!Model_Infotable::instance().checkDBVersion()) {
            const wxString note = wxString::Format("%s - %s", mmex::getProgramName(), _t("No File opened"));
            this->SetTitle(note);
            wxMessageBox(_t("Sorry. The Database version is too old or Database password is incorrect")
                , dialogErrorMessageHeading
                , wxOK | wxICON_EXCLAMATION);

            ShutdownDatabase();
            return false;
        }

        m_password = password;

        // WE CAN EVENTUALLY DELETE THIS CODE
        // Get Hidden Categories id from stored INFO string and move to Category/Subcategory tables
        wxString sSettings = Model_Infotable::instance().getString("HIDDEN_CATEGS_ID", "");
        if (!sSettings.empty()) {
            wxStringTokenizer token(sSettings, ";");
            Model_Category::instance().Savepoint();
            while (token.HasMoreTokens()) {
                wxString catData = token.GetNextToken();
                wxLongLong_t catID = 0;
                wxLongLong_t subCatID = 0;
                if (2 == sscanf(catData.mb_str(),"*%lld:%lld*", &catID, &subCatID)) {
                    if (subCatID == -1) {
                        Model_Category::Data* cat = Model_Category::instance().get(catID);
                        if (cat && cat->CATEGID != -1) {
                            cat->ACTIVE = 0;
                            Model_Category::instance().save(cat);
                        }
                    }
                    else {
                        Model_Category::Data* subcat = Model_Category::instance().get(subCatID);
                        if (subcat && subcat->CATEGID != -1) {
                            subcat->ACTIVE = 0;
                            Model_Category::instance().save(subcat);
                        }
                    }
                }
            }
            Model_Category::instance().ReleaseSavepoint();
            Model_Infotable::instance().setString("HIDDEN_CATEGS_ID", "");
        }
    }
    else if (openingNew) { // New Database
        // Remove file so we can replace it instead of opening it
        if (wxFileName::FileExists(fileName))
            wxRemoveFile(fileName);

        m_db = mmDBWrapper::Open(fileName, password);
        m_commit_callback_hook = new CommitCallbackHook();
        m_db->SetCommitHook(m_commit_callback_hook.get());
        m_update_callback_hook = new UpdateCallbackHook();
        m_db->SetUpdateHook(m_update_callback_hook.get());

        m_password = password;
        dbUpgrade::InitializeVersion(m_db.get());
        InitializeModelTables();

        mmNewDatabaseWizard* wizard = new mmNewDatabaseWizard(this);
        wizard->CenterOnParent();
        if (!wizard->RunIt(true)) {
            ShutdownDatabase();
            wxRemoveFile(fileName);
            return false;
        }

        wxButton* next = static_cast<wxButton*>(wizard->FindWindow(wxID_FORWARD)); //FIXME: 
        if (next) next->SetLabel(_tu("&Next →"));

        SetDataBaseParameters(fileName);
        /* Jump to new account creation screen */
        wxCommandEvent evt;
        OnNewAccount(evt);
        return true;
    }
    else { // open of existing database failed
        const wxString note = wxString::Format(
            "%s - %s",
            mmex::getProgramName(),
            _t("No File opened")
        );
        this->SetTitle(note);

        wxString msgStr = _t("Unable to find previously opened database.\n");
        if (!passwordCheckPassed)
            msgStr = _t("Password not entered for encrypted database.\n");

        msgStr << fileName;
        wxMessageBox(msgStr, dialogErrorMessageHeading, wxOK | wxICON_ERROR);
        menuEnableItems(false);
        return false;
    }

    SetDataBaseParameters(fileName);

    return true;
}
//----------------------------------------------------------------------------

void mmGUIFrame::SetDataBaseParameters(const wxString& fileName)
{
    wxFileName fname(fileName);
    wxString title = wxString::Format("%s - %s", fname.GetFullName(), mmex::getProgramName());
    if (mmex::isPortableMode())
        title = wxString::Format("%s [%s]", title, _t("portable mode"));

    SetTitle(title);

    if (m_db) {
        m_filename = fileName;
        /* Set InfoTable Options into memory */
        Option::instance().load();
    }
    else {
        m_filename.Clear();
        m_password.Clear();
    }
}
//----------------------------------------------------------------------------

bool mmGUIFrame::openFile(const wxString& fileName, bool openingNew, const wxString &password)
{ 
    menuBar_->FindItem(MENU_CHANGE_ENCRYPT_PASSWORD)->Enable(false);
    if (createDataStore(fileName, password, openingNew)) {
        m_recentFiles->AddFileToHistory(fileName);
        menuEnableItems(true);
        menuPrintingEnable(false);

        if (m_db->IsEncrypted()) {
            menuBar_->FindItem(MENU_CHANGE_ENCRYPT_PASSWORD)->Enable(true);
        }

        if (!m_app->GetSilentParam()) {
            db_lockInPlace = Model_Infotable::instance().getBool("ISUSED", false);
            if (db_lockInPlace) {
                int response = wxMessageBox(_t(
                    "Attempting to open a database that has been marked as opened by another instance of MMEX.\n"
                    "To avoid data loss or conflict, it's strongly recommended that you close all other applications that may be using the database.\n\n"
                    "If nothing else is running, it's possible that the database was left open as a result of a crash during previous usage of MMEX.") +
                    "\n\n" +
                    _t("Do you want to open the database?")
                    , _t("MMEX Instance Check"), wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
                if (response == wxNO)
                    return false;
            }
        }

        Model_Infotable::instance().setBool("ISUSED", true);
        db_lockInPlace = false;
        autoRepeatTransactionsTimer_.Start(REPEAT_TRANS_DELAY_TIME, wxTIMER_ONE_SHOT);
    }
    else
        return false;

    return true;
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnNew(wxCommandEvent& /*event*/)
{
    autoRepeatTransactionsTimer_.Stop();
    wxFileDialog dlg(this,
        _t("Choose database file to create"),
        wxEmptyString,
        wxEmptyString,
        _t("MMEX Database")+" (*.mmb)|*.mmb",
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT
    );

    if (dlg.ShowModal() != wxID_OK)
        return;

    wxString fileName = dlg.GetPath();

    if (!fileName.EndsWith(".mmb"))
        fileName += ".mmb";

    SetDatabaseFile(fileName, true);
    Model_Setting::instance().setString("LASTFILENAME", fileName);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnOpen(wxCommandEvent& /*event*/)
{
    autoRepeatTransactionsTimer_.Stop();
    wxString fileName = wxFileSelector(
        _t("Choose database file to open"),
        wxEmptyString, wxEmptyString, wxEmptyString,
        _t("MMEX Database") + " (*.mmb)|*.mmb|" + _t("Encrypted MMEX Database") + " (*.emb)|*.emb",
        wxFD_FILE_MUST_EXIST | wxFD_OPEN,
        this
    );

    if (!fileName.empty()) {
        SetDatabaseFile(fileName);
        saveSettings();
        if (m_db) {
            autocleanDeletedTransactions();
            if (!Model_Stock::instance().all().empty() &&
                Model_Setting::instance().getBool("REFRESH_STOCK_QUOTES_ON_OPEN", false)
            ) {
                wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_RATES);
                this->GetEventHandler()->AddPendingEvent(evt);
            }
        }
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnConvertEncryptedDB(wxCommandEvent& /*event*/)
{
    wxString encFileName = wxFileSelector(
        _t("Choose Encrypted database file to open"),
        wxEmptyString, wxEmptyString, wxEmptyString,
        _t("Encrypted MMEX Database") + " (*.emb)|*.emb",
        wxFD_FILE_MUST_EXIST,
        this
    );

    if (encFileName.empty())
        return;

    wxString password = wxGetPasswordFromUser(
        _t("Enter password for database"),
        _t("MMEX: Encrypted Database")
    );
    if (password.empty())
        return;

    wxFileDialog dlg(
        this,
        _t("Choose database file to Save As"),
        wxEmptyString,
        wxEmptyString,
        _t("MMEX Database") + " (*.mmb)|*.mmb",
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT
    );

    if (dlg.ShowModal() != wxID_OK)
        return;

    wxString fileName = dlg.GetPath();

    if (!dlg.GetPath().EndsWith(".mmb"))
        fileName += ".mmb";

    wxCopyFile(encFileName, fileName);

    wxSQLite3Database db;
    wxSQLite3CipherSQLCipher cipher;
    cipher.InitializeVersionDefault(4);
    cipher.SetLegacy(true);

    db.Open(fileName, cipher, password);
    db.ReKey(cipher, wxEmptyString);
    db.Close();

    mmErrorDialogs::MessageError(this, _t("Converted database!"), _t("MMEX message"));
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnChangeEncryptPassword(wxCommandEvent& /*event*/)
{
    wxString password_change_heading = _t("MMEX: Encryption Password Change");
    wxString password_message = wxString::Format(
        _t("New password for database:") + "\n\n%s",
        m_filename
    );

    wxPasswordEntryDialog dlg(this, password_message, password_change_heading);
    if (dlg.ShowModal() == wxID_OK) {
        wxString new_password = dlg.GetValue();
        if (new_password.IsEmpty()) {
            wxMessageBox(
                _t("New password must not be empty."),
                password_change_heading,
                wxOK | wxICON_WARNING
            );
        }
        else {
            wxPasswordEntryDialog confirm_dlg(
                this,
                _t("Please confirm new password"),
                password_change_heading
            );
            if (confirm_dlg.ShowModal() == wxID_OK) {
                wxString confirm_password = confirm_dlg.GetValue();
                if (!confirm_password.IsEmpty() && (new_password == confirm_password)) {
                    wxSQLite3CipherSQLCipher cipher;
                    cipher.InitializeVersionDefault(4);
                    cipher.SetLegacy(true);

                    m_db->ReKey(cipher, confirm_password);
                    wxMessageBox(_t("Password change completed"), password_change_heading);
                }
                else {
                    wxMessageBox(_t("Confirm password failed."), password_change_heading);
                }
            }
        }
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnVacuumDB(wxCommandEvent& /*event*/)
{
    wxMessageDialog msgDlg(
        this,
        wxString::Format("%s\n\n%s", _t("Backup database before optimization."), _t("Do you want to continue?")),
        _t("DB Optimization"),
        wxYES_NO | wxNO_DEFAULT | wxICON_WARNING
    );
    if (msgDlg.ShowModal() == wxID_YES) {
        const wxString SizeBefore = wxFileName(m_filename).GetHumanReadableSize();
        m_db->Vacuum();
        const wxString SizeAfter = wxFileName(m_filename).GetHumanReadableSize();
        wxMessageBox(wxString::Format(
            _t("Database Optimization Completed!\n\n"
                "Size before: %1$s\n"
                "Size after: %2$s\n"
            ),
            SizeBefore, SizeAfter),
            _t("DB Optimization")
        );
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnDebugDB(wxCommandEvent& /*event*/)
{
    wxASSERT(m_db);
    wxString resultMessage;
    wxSQLite3Statement stmt = m_db->PrepareStatement("PRAGMA integrity_check;");
    if (stmt.IsReadOnly()) {
        try {
            wxSQLite3ResultSet rs = stmt.ExecuteQuery();
            int columnCount = rs.GetColumnCount();
            while (rs.NextRow()) {
                wxString strRow = "";
                for (int i = 0; i < columnCount; ++i)
                    strRow << rs.GetAsString(i);
                resultMessage << strRow + wxTextFile::GetEOL();
            }
        }
        catch (const wxSQLite3Exception& e) {
            wxMessageBox(
                _t("Query error, please contact MMEX support!") + "\n\n" + e.GetMessage(),
                _t("MMEX debug error"),
                wxOK | wxICON_ERROR
            );
            return;
        }
    }

    if (!resultMessage.IsEmpty()) {
        wxTextEntryDialog checkDlg(this, _t("Result of database integrity check:"), _t("Database Check"), resultMessage.Trim(), wxOK | wxTE_MULTILINE);
        checkDlg.SetIcon(mmex::getProgramIcon());
        wxTextCtrl* textCtrl = dynamic_cast<wxTextCtrl*>(checkDlg.FindWindow(3000));
        if (textCtrl) {
            textCtrl->SetEditable(false);
            textCtrl->SetInsertionPointEnd();
        }

        wxButton* ok = static_cast<wxButton*>(checkDlg.FindWindow(wxID_OK));
        if (ok)
            ok->SetLabel(wxGetTranslation(g_OkLabel));

        checkDlg.Centre();
        checkDlg.ShowModal();
    }

    wxMessageDialog msgDlg(
        this,
        wxString::Format("%s\n\n%s",
            _t("Please use this function only if requested by MMEX support and an .mmdbg debug file has been supplied."),
            _t("Do you want to continue?")
        ),
        _t("Database Debug"),
        wxYES_NO | wxNO_DEFAULT | wxICON_WARNING
    );

    msgDlg.SetYesNoLabels(_t("&Yes"), _t("&No"));

    if (msgDlg.ShowModal() == wxID_YES) {
        dbUpgrade::SqlFileDebug(m_db.get());
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnSaveAs(wxCommandEvent& /*event*/)
{
    wxASSERT(m_db);

    if (m_filename.empty()) {
        wxASSERT(false);
        return;
    }

    wxFileDialog dlg(this,
        _t("Save database file as"),
        wxEmptyString,
        wxEmptyString,
        _t("MMEX Database")+" (*.mmb)|*.mmb|"+_t("Encrypted MMEX Database")+" (*.emb)|*.emb",
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT
    );

    if (dlg.ShowModal() != wxID_OK) return;

    // Ensure database is in a steady state first
    createHomePage();

    bool encrypt = dlg.GetFilterIndex() != 0; // emb -> Encrypted mMB
    wxFileName newFileName(dlg.GetPath());
    wxString ext = encrypt ? "emb" : "mmb";
    if (newFileName.GetExt().Lower() != ext) newFileName.SetExt(ext);

    wxFileName oldFileName(m_filename); // opened db's file

    if (newFileName == oldFileName) { // on case-sensitive FS uses case-sensitive comparison
        wxMessageDialog dlgMsg(
            this,
            _t("Unable to copy file to itself"),
            _t("Save database file as"),
            wxOK | wxICON_WARNING
        );
        dlgMsg.ShowModal();
        return;
    }

    // prepare to copy
    wxString new_password;
    bool rekey = encrypt ^ m_db->IsEncrypted();

    if (encrypt) {
        if (rekey) {
            new_password = wxGetPasswordFromUser(_t("Enter password for new database"), _t("MMEX: Encrypted Database"));
            if (new_password.empty())
                return;
        }
        else {
            new_password = m_password;
        }
    }

    // copying db
    ShutdownDatabase(); // database must be closed before copying its file

    if (!wxCopyFile(oldFileName.GetFullPath(), newFileName.GetFullPath(), true))  // true -> overwrite if file exists
        return;

    if (rekey) { // encrypt or reset encryption
        wxSQLite3Database dbx;

        wxSQLite3CipherSQLCipher cipher;
        cipher.InitializeVersionDefault(4);
        cipher.SetLegacy(true);

        dbx.Open(newFileName.GetFullPath(), cipher, m_password);
        dbx.ReKey(cipher, new_password); // empty password resets encryption
        dbx.Close();
    }

    m_password.clear();
    if (openFile(newFileName.GetFullPath(), false, new_password)) {
        DoRecreateNavTreeControl(true);
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnExportToCSV(wxCommandEvent& /*event*/)
{
    mmUnivCSVDialog(this, mmUnivCSVDialog::DIALOG_TYPE_EXPORT_CSV, gotoAccountID_).ShowModal();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnExportToXML(wxCommandEvent& /*event*/)
{
    mmUnivCSVDialog(this, mmUnivCSVDialog::DIALOG_TYPE_EXPORT_XML, gotoAccountID_).ShowModal();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnExportToQIF(wxCommandEvent& /*event*/)
{
    mmQIFExportDialog dlg(this, mmQIFExportDialog::QIF, gotoAccountID_);
    dlg.ShowModal();
}
void mmGUIFrame::OnExportToJSON(wxCommandEvent& /*event*/)
{
    mmQIFExportDialog dlg(this, mmQIFExportDialog::JSON, gotoAccountID_);
    dlg.ShowModal();
}
void mmGUIFrame::OnExportToMMEX(wxCommandEvent& /*event*/)
{
    mmQIFExportDialog dlg(this, mmQIFExportDialog::CSV, gotoAccountID_);
    dlg.ShowModal();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnImportQIF(wxCommandEvent& /*event*/)
{

    mmQIFImportDialog dlg(this, gotoAccountID_);
    dlg.ShowModal();
    int64 account_id = dlg.get_last_imported_acc();
    RefreshNavigationTree();
    if (account_id > 0) {
        setGotoAccountID(account_id);
        Model_Account::Data* account = Model_Account::instance().get(account_id);
        setNavTreeAccount(account->ACCOUNTNAME);
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_GOTOACCOUNT);
        this->GetEventHandler()->AddPendingEvent(evt);
    }
    else {
        refreshPanelData();
    }

}
//----------------------------------------------------------------------------

void mmGUIFrame::OnImportUniversalCSV(wxCommandEvent& /*event*/)
{
    if (Model_Account::instance().all().empty()) {
        wxMessageBox(_t("No account available to import"), _t("Universal CSV Import"), wxOK | wxICON_WARNING);
        return;
    }

    mmUnivCSVDialog univCSVDialog(this, mmUnivCSVDialog::DIALOG_TYPE_IMPORT_CSV, gotoAccountID_);
    univCSVDialog.ShowModal();
    if (univCSVDialog.isImportCompletedSuccessfully()) {
        Model_Account::Data* account = Model_Account::instance().get(univCSVDialog.ImportedAccountID());
        if (account) {
            createCheckingPage(account->ACCOUNTID);
            setNavTreeAccount(account->ACCOUNTNAME);
        }
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnImportXML(wxCommandEvent& /*event*/)
{
    if (Model_Account::instance().all().empty()) {
        wxMessageBox(_t("No account available to import"), _t("Universal CSV Import"), wxOK | wxICON_WARNING);
        return;
    }

    mmUnivCSVDialog univCSVDialog(this, mmUnivCSVDialog::DIALOG_TYPE_IMPORT_XML, gotoAccountID_);
    univCSVDialog.ShowModal();
    if (univCSVDialog.isImportCompletedSuccessfully()) {
        Model_Account::Data* account = Model_Account::instance().get(univCSVDialog.ImportedAccountID());
        if (account) {
            createCheckingPage(account->ACCOUNTID);
            setNavTreeAccount(account->ACCOUNTNAME);
        }
    }
}

//----------------------------------------------------------------------------

void mmGUIFrame::OnImportWebApp(wxCommandEvent& /*event*/)
{
    mmWebAppDialog dlg(this, false);
    if (dlg.ShowModal() == wxID_HELP) {
        helpFileIndex_ = mmex::HTML_WEBAPP;
        createHelpPage(helpFileIndex_);
        setNavTreeSection(_t("Help"));
    }
    if (dlg.getRefreshRequested())
        refreshPanelData();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close(true);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnNewAccount(wxCommandEvent& /*event*/)
{
    mmAddAccountWizard* wizard = new mmAddAccountWizard(this);
    wizard->CenterOnParent();
    wizard->RunIt();

    if (wizard->acctID_ != -1) {
        Model_Account::Data* account = Model_Account::instance().get(wizard->acctID_);
        mmNewAcctDialog dlg(account, this);
        dlg.ShowModal();
        if (account->ACCOUNTTYPE == Model_Account::TYPE_NAME_ASSET) {
            wxMessageBox(_t(
                "Asset Accounts hold Asset transactions\n\n"
                "Asset transactions are created within the Assets View\n"
                "after the selection of the Asset within that view.\n\n"
                "Asset Accounts can also hold normal transactions to regular accounts."
            ), _t("Asset Account Creation"));
        }

        if (account->ACCOUNTTYPE == Model_Account::TYPE_NAME_SHARES) {
            wxMessageBox(_tu(
                "Share Accounts hold Share transactions\n\n"
                "Share transactions are created within the Stock Portfolio View\n"
                "after the selection of the Company Stock within the associated view.\n\n"
                "These accounts only become visible after associating a Stock to the Share Account\n"
                "Or by using the Menu View → 'Display Share Accounts'\n"
                "Share Accounts can also hold normal transactions to regular account."
            ), _t("Share Account Creation"));
        }

        RefreshNavigationTree();
    }

    createHomePage();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnAccountList(wxCommandEvent& /*event*/)
{
    createHomePage();
}
//----------------------------------------------------------------------------

void mmGUIFrame::refreshPanelData(wxCommandEvent& /*event*/)
{
    refreshPanelData();
}
void mmGUIFrame::refreshPanelData()
{
    int id = panelCurrent_ ? panelCurrent_->GetId() : mmID_HOMEPAGE;
    wxLogDebug("Panel ID: %d", id);

    switch (id) {
    case mmID_HOMEPAGE:
        createHomePage();
        break;
    case mmID_CHECKING:
        wxDynamicCast(panelCurrent_, mmCheckingPanel)->refreshList();
        break;
    case mmID_STOCKS:
        wxDynamicCast(panelCurrent_, mmStocksPanel)->RefreshList();
        break;
    case mmID_ASSETS:
        wxDynamicCast(panelCurrent_, mmAssetsPanel)->RefreshList();
        break;
    case mmID_BILLS:
        wxDynamicCast(panelCurrent_, mmBillsDepositsPanel)->RefreshList();
        break;
    case mmID_BUDGET:
        wxDynamicCast(panelCurrent_, mmBudgetingPanel)->RefreshList();
        break;
    case mmID_REPORTS:
        if (activeReport_) {
            mmReportsPanel* reportsPanel = wxDynamicCast(panelCurrent_, mmReportsPanel);
            if (reportsPanel) createReportsPage(reportsPanel->getPrintableBase(), false);
        }
        break;
    default:
        createHelpPage();
        break;
    }
}

void mmGUIFrame::SetNavTreeSelection(wxTreeItemId id) {
    if (id.IsOk()) {
        wxTreeEvent evt(wxEVT_TREE_SEL_CHANGED, m_nav_tree_ctrl, id);
        m_nav_tree_ctrl->GetEventHandler()->AddPendingEvent(evt);
    }
}

void mmGUIFrame::OnOrgCategories(wxCommandEvent& /*event*/)
{
    mmCategDialog dlg(this, false, -1);
    dlg.ShowModal();
    if (dlg.getRefreshRequested()) {
        activeReport_ = false;
        refreshPanelData();
        RefreshNavigationTree();
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnOrgPayees(wxCommandEvent& /*event*/)
{
    mmPayeeDialog dlg(this, false);
    dlg.ShowModal();
    if (dlg.getRefreshRequested()) {
        activeReport_ = false;
        refreshPanelData();
        RefreshNavigationTree();
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnOrgTags(wxCommandEvent& /*event*/)
{
    mmTagDialog dlg(this);
    dlg.ShowModal();
    if (dlg.getRefreshRequested()) {
        activeReport_ = false;
        refreshPanelData();
        RefreshNavigationTree();
    }
    
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnNewTransaction(wxCommandEvent& event)
{
    if (!m_db)
        return;

    if (Model_Account::instance().all_checking_account_names().empty()) return;
    mmTransDialog dlg(this, gotoAccountID_, {0, false});

    int i = dlg.ShowModal();
    if (i == wxID_CANCEL)
        return;

    gotoAccountID_ = dlg.GetAccountID();
    gotoTransID_ = { dlg.GetTransactionID(), 0 };
    Model_Account::Data * account = Model_Account::instance().get(gotoAccountID_);
    if (account) {
        createCheckingPage(gotoAccountID_);
        setNavTreeAccount(account->ACCOUNTNAME);
    }

    if (i == wxID_NEW)
        OnNewTransaction(event);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnTransactionReport(wxCommandEvent& WXUNUSED(event))
{
    if (!m_db) return;
    if (Model_Account::instance().all().empty()) return;

    const auto filter_settings = Model_Infotable::instance().getArrayString("TRANSACTIONS_FILTER");

    wxSharedPtr<mmFilterTransactionsDialog> dlg(new mmFilterTransactionsDialog(this, -1, true));
    bool is_ok = (dlg->ShowModal() == wxID_OK);
    if (filter_settings != Model_Infotable::instance().getArrayString("TRANSACTIONS_FILTER")) {
        DoRecreateNavTreeControl();
    }
    if (is_ok) {
        mmReportTransactions* rs = new mmReportTransactions(dlg);
        setNavTreeSection(_t("Transaction Report"));
        createReportsPage(rs, true);
    }
}

void mmGUIFrame::OnBudgetSetupDialog(wxCommandEvent& /*event*/)
{
    if (!m_db)
        return;

    const auto a = Model_Budgetyear::instance().all(Model_Budgetyear::COL_BUDGETYEARNAME).to_json();
    mmBudgetYearDialog(this).ShowModal();
    const auto b = Model_Budgetyear::instance().all(Model_Budgetyear::COL_BUDGETYEARNAME).to_json();
    if (a != b)
        DoRecreateNavTreeControl(true);
    setNavTreeSection(_t("Budget Planner"));
}

void mmGUIFrame::OnGeneralReportManager(wxCommandEvent& /*event*/)
{
    if (!m_db)
        return;

    mmGeneralReportManager dlg(this, m_db.get());
    dlg.ShowModal();
    RefreshNavigationTree();
}

void mmGUIFrame::OnOptions(wxCommandEvent& /*event*/)
{
    if (!m_db.get())
        return;

    mmOptionsDialog systemOptions(this, this->m_app);
    if (systemOptions.ShowModal() == wxID_OK) {
        //set the View Menu Option items the same as the options saved.
        menuBar_->FindItem(MENU_VIEW_BUDGET_FINANCIAL_YEARS)->Check(Option::instance().getBudgetFinancialYears());
        menuBar_->FindItem(MENU_VIEW_BUDGET_TRANSFER_TOTAL)->Check(Option::instance().getBudgetIncludeTransfers());
        menuBar_->FindItem(MENU_VIEW_BUDGET_CATEGORY_SUMMARY)->Check(Option::instance().getBudgetSummaryWithoutCategories());
        menuBar_->FindItem(MENU_VIEW_IGNORE_FUTURE_TRANSACTIONS)->Check(Option::instance().getIgnoreFutureTransactions());
        menuBar_->FindItem(MENU_VIEW_SHOW_TOOLTIPS)->Check(Option::instance().getShowToolTips());
        menuBar_->FindItem(MENU_VIEW_SHOW_MONEYTIPS)->Check(Option::instance().getShowMoneyTips());
        menuBar_->Refresh();
        menuBar_->Update();
        refreshPanelData();
        RefreshNavigationTree();

        // Reset columns of the checking panel in case the time columns was added/removed
        int id = panelCurrent_->GetId();
        if (id == mmID_CHECKING)
            wxDynamicCast(panelCurrent_, mmCheckingPanel)->resetColumnView();

        const wxString& sysMsg = _t("Settings have been updated.") + "\n\n"
            + _t("Some settings take effect only after an application restart.");
        wxMessageBox(sysMsg, _t("Settings"), wxOK | wxICON_INFORMATION);
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnCustomFieldsManager(wxCommandEvent& WXUNUSED(event))
{
    if (!m_db)
        return;

    mmCustomFieldListDialog dlg(this);
    dlg.ShowModal();
    createHomePage();
}

void mmGUIFrame::OnThemeManager(wxCommandEvent& /*event*/)
{
    mmThemesDialog dlg(this);
    dlg.ShowModal();
}

bool mmGUIFrame::OnRefreshWebApp(bool is_silent)
{
    if (mmWebApp::MMEX_WebApp_UpdateAccount()
        && mmWebApp::MMEX_WebApp_UpdateCategory()
        && mmWebApp::MMEX_WebApp_UpdatePayee()) {
        if (!is_silent) {
            wxMessageBox(_t("Accounts, Payees, and Categories Updated"), _t("Refresh WebApp"), wxOK | wxICON_INFORMATION);
        }
        return true;
    }
    else {
        if (!is_silent) {
            wxMessageBox(
                _t("Issue encountered updating WebApp, check Web server and WebApp settings"),
                _t("Refresh WebApp"),
                wxOK | wxICON_ERROR
            );
        }
    }
    return false;
}

void mmGUIFrame::OnRefreshWebApp(wxCommandEvent&)
{
    OnRefreshWebApp(false);
}

//----------------------------------------------------------------------------

void mmGUIFrame::OnHelp(wxCommandEvent& /*event*/)
{
    createHelpPage();
    setNavTreeSection(_t("Help"));
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnCheckUpdate(wxCommandEvent& /*event*/)
{
    mmUpdate::checkUpdates(this, false);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnSimpleURLOpen(wxCommandEvent& event)
{
    wxString url;
    switch (event.GetId()) {
    case MENU_FACEBOOK: url = mmex::weblink::Facebook; break;
    case MENU_TWITTER: url = mmex::weblink::Twitter; break;
    case MENU_WEBSITE: url = mmex::weblink::WebSite; break;
    case MENU_WIKI: url = mmex::weblink::Wiki; break;
    case MENU_DONATE: url = mmex::weblink::Donate; break;
    case MENU_CROWDIN: url = mmex::weblink::Crowdin; break;
    case MENU_REPORTISSUES: url = mmex::weblink::Forum; break;
    case MENU_APPLE_APPSTORE: url = mmex::weblink::AppleAppStore; break;
    case MENU_GOOGLEPLAY: url = mmex::weblink::GooglePlay; break;
    case MENU_BUY_COFFEE: url = mmex::weblink::SquareCashGuan; break;
    case MENU_RSS: url = mmex::weblink::NewsRSS; break;
    case MENU_YOUTUBE: url = mmex::weblink::YouTube; break;
    case MENU_GITHUB: url = mmex::weblink::GitHub; break;
    case MENU_SLACK: url = mmex::weblink::Slack; break;
    }
    if (!url.IsEmpty()) wxLaunchDefaultBrowser(url);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnBeNotified(wxCommandEvent& /*event*/)
{
    Model_Setting::instance().setString(INIDB_NEWS_LAST_READ_DATE, wxDate::Today().FormatISODate());
    wxLaunchDefaultBrowser(mmex::weblink::News);

    int toolbar_icon_size = Option::instance().getToolbarIconSize();
    //toolBar_->SetToolBitmapSize(wxSize(toolbar_icon_size, toolbar_icon_size));
    toolBar_->SetToolBitmap(MENU_ANNOUNCEMENTMAILING, mmBitmapBundle(png::NEWS, toolbar_icon_size));

    const auto b = toolBar_->FindTool(MENU_ANNOUNCEMENTMAILING);
    if (b) b->SetShortHelp(_t("News"));
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnReportBug(wxCommandEvent& WXUNUSED(event))
{
    mmPrintableBase* br = new mmBugReport();
    setNavTreeSection(_t("Reports"));
    createReportsPage(br, true);
}

void mmGUIFrame::OnDiagnostics(wxCommandEvent& /*event*/)
{
    mmDiagnosticsDialog dlg(this, this->IsMaximized());
    dlg.ShowModal();
}

void mmGUIFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    mmAboutDialog(this, 0).ShowModal();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnPrintPage(wxCommandEvent& WXUNUSED(event))
{
    panelCurrent_->PrintPage();
}

//----------------------------------------------------------------------------

void mmGUIFrame::showBeginAppDialog(bool fromScratch)
{
    mmAppStartDialog dlg(this, m_app);
    if (fromScratch) {
        dlg.SetCloseButtonToExit();
    }

    int end_mod = dlg.ShowModal();
    switch (end_mod) {
    case wxID_FILE1: {
        wxFileName fname(Model_Setting::instance().getLastDbPath());
        if (fname.IsOk()) {
            SetDatabaseFile(fname.GetFullPath());
        }
        break;
    }
    case wxID_OPEN: {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_OPEN);
        AddPendingEvent(evt);
        break;
    }
    case wxID_NEW: {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_NEW);
        AddPendingEvent(evt);
        break;
    }
    case wxID_SETUP: {
        auto language = Option::instance().getLanguageID(true);
        const auto langName = language == wxLANGUAGE_DEFAULT ? _t("System default") : wxLocale::GetLanguageName(language);
        break;
    }
    case wxID_EXIT:
        Close();
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnShowAppStartDialog(wxCommandEvent& WXUNUSED(event))
{
    showBeginAppDialog();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnExportToHtml(wxCommandEvent& WXUNUSED(event))
{
    wxString fileName = wxFileSelector(
        _t("Choose HTML file to Export"),
        wxEmptyString, wxEmptyString, wxEmptyString,
        "*.html",
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT
    );

    if (!fileName.empty()) {
        wxString htmlText = panelCurrent_->BuildPage();
        correctEmptyFileExt("html", fileName);
        wxFileOutputStream output(fileName);
        wxTextOutputStream text(output);
        text << htmlText;
        output.Close();
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnBillsDeposits(wxCommandEvent& WXUNUSED(event))
{
    createBillsDeposits();
}
//----------------------------------------------------------------------------

void mmGUIFrame::createHomePage()
{
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String("Home Page");

    const auto time = wxDateTime::UNow();

    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);
    int id = panelCurrent_ ? panelCurrent_->GetId() : -1;

    /* Update home page details only if it is being displayed */
    if (id == mmID_HOMEPAGE) {
        mmHomePagePanel* homePage = wxDynamicCast(panelCurrent_, mmHomePagePanel);
        homePage->createHtml();
    }
    else {
        DoWindowsFreezeThaw(homePanel_);
        wxSizer *sizer = cleanupHomePanel();
        panelCurrent_ = new mmHomePagePanel(homePanel_,
            this, mmID_HOMEPAGE,
            wxDefaultPosition, wxDefaultSize,
            wxNO_BORDER | wxTAB_TRAVERSAL
        );
        sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
        homePanel_->Layout();
        DoWindowsFreezeThaw(homePanel_);
    }

    wxTreeItemId root = m_nav_tree_ctrl->GetRootItem();
    if (root.IsOk() && m_nav_tree_ctrl->HasChildren(root)) {
        wxTreeItemIdValue cookie;
        wxTreeItemId firstChild = m_nav_tree_ctrl->GetFirstChild(root, cookie);
        if (firstChild.IsOk())
            m_nav_tree_ctrl->SelectItem(firstChild);
        m_nav_tree_ctrl->SetEvtHandlerEnabled(true);
    }

    json_writer.Key("seconds");
    json_writer.Double((wxDateTime::UNow() - time).GetMilliseconds().ToDouble() / 1000);
    json_writer.EndObject();

    Model_Usage::instance().AppendToUsage(wxString::FromUTF8(json_buffer.GetString()));
}
//----------------------------------------------------------------------------

void mmGUIFrame::createReportsPage(mmPrintableBase* rs, bool cleanup)
{
    if (!rs) return;
    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);

    DoWindowsFreezeThaw(homePanel_);
    wxSizer *sizer = cleanupHomePanel();
    panelCurrent_ = new mmReportsPanel(
        rs,
        cleanup, homePanel_, this, mmID_REPORTS,
        wxDefaultPosition, wxDefaultSize,
        wxNO_BORDER | wxTAB_TRAVERSAL
    );

    sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
    homePanel_->Layout();
    DoWindowsFreezeThaw(homePanel_);

    menuPrintingEnable(true);
    m_nav_tree_ctrl->SetEvtHandlerEnabled(true);
}
//----------------------------------------------------------------------------

void mmGUIFrame::createHelpPage(int index)
{
    helpFileIndex_ = index;
    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);
    DoWindowsFreezeThaw(homePanel_);
    wxSizer *sizer = cleanupHomePanel();
    panelCurrent_ = new mmHelpPanel(homePanel_, this, wxID_HELP);
    sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
    homePanel_->Layout();
    DoWindowsFreezeThaw(homePanel_);
    menuPrintingEnable(true);
    m_nav_tree_ctrl->SetEvtHandlerEnabled(true);
}
//----------------------------------------------------------------------------

void mmGUIFrame::createBillsDeposits()
{
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String("Bills & Deposits Panel");

    const auto time = wxDateTime::UNow();

    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);
    if (panelCurrent_->GetId() == mmID_BILLS) {
        mmBillsDepositsPanel* billsDepositsPanel = wxDynamicCast(panelCurrent_, mmBillsDepositsPanel);
        billsDepositsPanel->RefreshList();
    }
    else {
        DoWindowsFreezeThaw(homePanel_);
        wxSizer *sizer = cleanupHomePanel();
        panelCurrent_ = new mmBillsDepositsPanel(homePanel_, mmID_BILLS);

        sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);

        homePanel_->Layout();
        DoWindowsFreezeThaw(homePanel_);
    }
    menuPrintingEnable(true);
    m_nav_tree_ctrl->SetEvtHandlerEnabled(true);

    json_writer.Key("seconds");
    json_writer.Double((wxDateTime::UNow() - time).GetMilliseconds().ToDouble() / 1000);
    json_writer.EndObject();

    Model_Usage::instance().AppendToUsage(wxString::FromUTF8(json_buffer.GetString()));
    m_nav_tree_ctrl->SetFocus();
    setNavTreeSection(_t("Scheduled Transactions"));
}
//----------------------------------------------------------------------------

void mmGUIFrame::createBudgetingPage(int64 budgetYearID)
{
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String("Budget Panel");

    const auto time = wxDateTime::UNow();

    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);
    if (panelCurrent_->GetId() == mmID_BUDGET) {
        mmBudgetingPanel* budgetingPage = wxDynamicCast(panelCurrent_, mmBudgetingPanel);
        budgetingPage->DisplayBudgetingDetails(budgetYearID);
    }
    else {
        DoWindowsFreezeThaw(homePanel_);
        wxSizer *sizer = cleanupHomePanel();
        panelCurrent_ = new mmBudgetingPanel(budgetYearID, homePanel_, this, mmID_BUDGET);
        sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
        homePanel_->Layout();
        DoWindowsFreezeThaw(homePanel_);
    }

    json_writer.Key("seconds");
    json_writer.Double((wxDateTime::UNow() - time).GetMilliseconds().ToDouble() / 1000);
    json_writer.EndObject();

    Model_Usage::instance().AppendToUsage(wxString::FromUTF8(json_buffer.GetString()));

    menuPrintingEnable(true);
    m_nav_tree_ctrl->SetEvtHandlerEnabled(true);
}
//----------------------------------------------------------------------------

void mmGUIFrame::createCheckingPage(int64 checking_id, const std::vector<int64> &group_ids) {
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String(
        (checking_id == -1) ? "All Transactions" :
        (checking_id == -2) ? "Deleted Transactions" :
        (checking_id <= -3) ? "Group Transactions" :
        "Checking Panel"
    );

    const auto time = wxDateTime::UNow();

    // Check if the credit balance needs to be displayed or not
    // If this differs from before then we need to rebuild
    bool newCreditDisplayed = false;
    if (checking_id >= 1) {
        Model_Account::Data* account = Model_Account::instance().get(checking_id);
        newCreditDisplayed = (account->CREDITLIMIT != 0);
    }

    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);
    bool done = false;
    if (panelCurrent_->GetId() == mmID_CHECKING) {
        mmCheckingPanel* cp = wxDynamicCast(panelCurrent_, mmCheckingPanel);
        if ((checking_id == -1 && cp->isAllTrans()) ||
            (checking_id == -2 && cp->isDeletedTrans()) ||
            (checking_id >= 1 && cp->isAccount() && newCreditDisplayed == creditDisplayed_)
        ) {
            cp->refreshList();
            if (cp->isAccount())
                cp->loadAccount(checking_id);
            done = true;
        }
    }
    if (!done) {
        DoWindowsFreezeThaw(homePanel_);
        if (checking_id >= 1)
            creditDisplayed_ = newCreditDisplayed;
        wxSizer *sizer = cleanupHomePanel();
        panelCurrent_ = new mmCheckingPanel(this, homePanel_, checking_id, group_ids);
        sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
        homePanel_->Layout();
        DoWindowsFreezeThaw(homePanel_);
    }

    json_writer.Key("seconds");
    json_writer.Double((wxDateTime::UNow() - time).GetMilliseconds().ToDouble() / 1000);
    json_writer.EndObject();

    Model_Usage::instance().AppendToUsage(wxString::FromUTF8(json_buffer.GetString()));

    menuPrintingEnable(true);
    if (checking_id >= 1 && gotoTransID_.first > 0) {
        mmCheckingPanel* cp = wxDynamicCast(panelCurrent_, mmCheckingPanel);
        cp->setSelectedTransaction(gotoTransID_);
        gotoTransID_ = { -1, 0 };
    }
    m_nav_tree_ctrl->SetEvtHandlerEnabled(true);
    m_nav_tree_ctrl->SetFocus();
}

void mmGUIFrame::createStocksAccountPage(int64 accountID)
{
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String("Stock Panel");

    const auto time = wxDateTime::UNow();

    if (panelCurrent_->GetId() == mmID_STOCKS) {
        mmStocksPanel* sp = wxDynamicCast(panelCurrent_, mmStocksPanel);
        sp->DisplayAccountDetails(accountID);
    }
    else {
        //updateNavTreeControl();
        DoWindowsFreezeThaw(homePanel_);
        wxSizer *sizer = cleanupHomePanel();
        panelCurrent_ = new mmStocksPanel(accountID, this, homePanel_);
        sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
        homePanel_->Layout();
        DoWindowsFreezeThaw(homePanel_);
    }

    json_writer.Key("seconds");
    json_writer.Double((wxDateTime::UNow() - time).GetMilliseconds().ToDouble() / 1000);
    json_writer.EndObject();

    Model_Usage::instance().AppendToUsage(wxString::FromUTF8(json_buffer.GetString()));
    menuPrintingEnable(true);
}

//----------------------------------------------------------------------------

void mmGUIFrame::OnGotoAccount(wxCommandEvent& WXUNUSED(event))
{
    bool proper_type = false;
    Model_Account::Data *acc = Model_Account::instance().get(gotoAccountID_);
    if (acc)
        proper_type = Model_Account::type_id(acc) != Model_Account::TYPE_ID_INVESTMENT;
    if (proper_type)
        createCheckingPage(gotoAccountID_);
    m_nav_tree_ctrl->Refresh();
}

void mmGUIFrame::OnGotoStocksAccount(wxCommandEvent& WXUNUSED(event))
{
    bool proper_type = false;
    Model_Account::Data *acc = Model_Account::instance().get(gotoAccountID_);
    if (acc)
        proper_type = Model_Account::type_id(acc) == Model_Account::TYPE_ID_INVESTMENT;
    if (proper_type)
        createStocksAccountPage(gotoAccountID_);
    m_nav_tree_ctrl->Refresh();
}

void mmGUIFrame::OnAssets(wxCommandEvent& /*event*/)
{
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String("Asset Panel");

    const auto time = wxDateTime::UNow();

    if (panelCurrent_->GetId() == mmID_ASSETS)
        refreshPanelData();
    else {
        DoWindowsFreezeThaw(homePanel_);
        wxSizer* sizer = cleanupHomePanel();
        panelCurrent_ = new mmAssetsPanel(this, homePanel_, mmID_ASSETS);
        sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
        homePanel_->Layout();
        DoWindowsFreezeThaw(homePanel_);
        menuPrintingEnable(true);
        setNavTreeSection(_t("Assets"));
    }

    json_writer.Key("seconds");
    json_writer.Double((wxDateTime::UNow() - time).GetMilliseconds().ToDouble() / 1000);
    json_writer.EndObject();

    Model_Usage::instance().AppendToUsage(wxString::FromUTF8(json_buffer.GetString()));
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnCurrency(wxCommandEvent& /*event*/)
{
    mmMainCurrencyDialog(this, false, false).ShowModal();
    refreshPanelData();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnRates(wxCommandEvent& WXUNUSED(event))
{
    wxBusyInfo info
#if (wxMAJOR_VERSION == 3 && wxMINOR_VERSION >= 1)
    (
        wxBusyInfoFlags()
        .Parent(this)
        .Title(_t("Downloading stock prices from Yahoo"))
        .Text(_tu("Please wait…"))
        .Foreground(*wxWHITE)
        .Background(wxColour(0, 102, 51))
        .Transparency(4 * wxALPHA_OPAQUE / 5)
    );
#else
    (_t("Downloading stock prices from Yahoo"), this);
#endif
    wxString msg;
    getOnlineCurrencyRates(msg);
    wxLogDebug("%s", msg);

    Model_Stock::Data_Set stock_list = Model_Stock::instance().all();
    if (!stock_list.empty()) {

        std::map<wxString, double> symbols;
        for (const auto& stock : stock_list) {
            const wxString symbol = stock.SYMBOL.Upper();
            if (symbol.IsEmpty()) continue;
            symbols[symbol] = stock.CURRENTPRICE;
        }

        std::map<wxString, double> stocks_data;
        if (get_yahoo_prices(symbols, stocks_data, "", msg, yahoo_price_type::SHARES)) {
            Model_StockHistory::instance().Savepoint();
            for (auto& s : stock_list) {
                std::map<wxString, double>::const_iterator it = stocks_data.find(s.SYMBOL.Upper());
                if (it == stocks_data.end())
                    continue;

                double dPrice = it->second;

                if (dPrice != 0) {
                    msg += wxString::Format("%s\t: %0.6f -> %0.6f\n", s.SYMBOL, s.CURRENTPRICE, dPrice);
                    s.CURRENTPRICE = dPrice;
                    if (s.STOCKNAME.empty()) s.STOCKNAME = s.SYMBOL;
                    Model_Stock::instance().save(&s);
                    Model_StockHistory::instance().addUpdate(s.SYMBOL
                        , wxDate::Now(), dPrice, Model_StockHistory::ONLINE);
                }
            }
            Model_StockHistory::instance().ReleaseSavepoint();
            wxString strLastUpdate;
            strLastUpdate.Printf(_t("%1$s on %2$s"), wxDateTime::Now().FormatTime()
                , mmGetDateTimeForDisplay(wxDateTime::Now().FormatISODate()));
            Model_Infotable::instance().setString("STOCKS_LAST_REFRESH_DATETIME", strLastUpdate);
        }

        wxLogDebug("%s", msg);
    }

    refreshPanelData();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnEditAccount(wxCommandEvent& /*event*/)
{
    const auto &accounts = Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME);
    if (accounts.empty()) {
        wxMessageBox(_t("No account available to edit!"), _t("Accounts"), wxOK | wxICON_WARNING);
        return;
    }

    mmSingleChoiceDialog scd(this, _t("Choose Account to Edit"), _t("Accounts"), accounts);
    if (scd.ShowModal() == wxID_OK) {
        Model_Account::Data* account = Model_Account::instance().get(scd.GetStringSelection());
        mmNewAcctDialog dlg(account, this);
        if (dlg.ShowModal() == wxID_OK)
            RefreshNavigationTree();
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnDeleteAccount(wxCommandEvent& /*event*/)
{
    const auto &accounts = Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME);
    if (accounts.empty()) {
        wxMessageBox(_t("No account available to delete!"), _t("Accounts"), wxOK | wxICON_WARNING);
        return;
    }

    mmSingleChoiceDialog scd(this, _t("Choose Account to Delete"), _t("Accounts"), accounts);
    if (scd.ShowModal() == wxID_OK) {
        Model_Account::Data* account = Model_Account::instance().get(scd.GetStringSelection());
        wxString deletingAccountName = wxString::Format(
            _t("Do you you want to delete\n%1$s account: %2$s?"),
            wxGetTranslation(account->ACCOUNTTYPE),
            account->ACCOUNTNAME
        );
        wxMessageDialog msgDlg(this, deletingAccountName, _t("Confirm Account Deletion"),
            wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION);
        if (msgDlg.ShowModal() == wxID_YES) {
            Model_Account::instance().remove(account->id());
            mmAttachmentManage::DeleteAllAttachments(Model_Attachment::REFTYPE_NAME_BANKACCOUNT, account->id());
        }
    }
    DoRecreateNavTreeControl(true);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnReallocateAccount(wxCommandEvent& WXUNUSED(event))
{
    mmSingleChoiceDialog account_choice(
        this,
        _t("Select account"), _t("Change Account Type"),
        Model_Account::instance().all_checking_account_names()
    );

    if (account_choice.ShowModal() == wxID_OK) {
        Model_Account::Data* account = Model_Account::instance().get(account_choice.GetStringSelection());
        if (account)
            ReallocateAccount(account->ACCOUNTID);
    }
}

void mmGUIFrame::ReallocateAccount(int64 accountID)
{
    Model_Account::Data* account = Model_Account::instance().get(accountID);
    int accountTypeId = Model_Account::type_id(account);
    wxArrayString types;
    for (int i = 0; i < Model_Account::TYPE_ID_size; ++i) {
        if (i != Model_Account::TYPE_ID_INVESTMENT && i != accountTypeId)
            types.Add(Model_Account::type_name(i));
    }
    wxArrayString types_loc;
    for (const auto &entry : types)
        types_loc.Add(wxGetTranslation(entry));

    mmSingleChoiceDialog type_choice(
        this,
        wxString::Format(_t("Select new account type for %s"), account->ACCOUNTNAME),
        _t("Change Account Type"),
        types_loc
    );

    if (type_choice.ShowModal() == wxID_OK) {
        int sel = type_choice.GetSelection();
        account->ACCOUNTTYPE = types[sel];
        Model_Account::instance().save(account);
        DoRecreateNavTreeControl(true);
    }
}

void mmGUIFrame::OnViewToolbar(wxCommandEvent &event)
{
    m_mgr.GetPane("toolbar").Show(event.IsChecked());
    m_mgr.Update();
    Model_Setting::instance().setBool("SHOWTOOLBAR", event.IsChecked());
}

void mmGUIFrame::OnViewLinks(wxCommandEvent &event)
{
    m_mgr.GetPane("Navigation").Show(event.IsChecked());
    m_mgr.Update();
}

void mmGUIFrame::OnViewToolbarUpdateUI(wxUpdateUIEvent &event)
{
    event.Check(m_mgr.GetPane("toolbar").IsShown());
}

void mmGUIFrame::OnViewLinksUpdateUI(wxUpdateUIEvent &event)
{
    event.Check(m_mgr.GetPane("Navigation").IsShown());
}

void mmGUIFrame::OnHideShareAccounts(wxCommandEvent &WXUNUSED(event))
{
    Option::instance().setHideShareAccounts(!Option::instance().getHideShareAccounts());
    RefreshNavigationTree();
}

void mmGUIFrame::OnHideDeletedTransactions(wxCommandEvent& WXUNUSED(event))
{
    Option::instance().setHideDeletedTransactions(!Option::instance().getHideDeletedTransactions());
    RefreshNavigationTree();
}

void mmGUIFrame::RefreshNavigationTree()
{
    // Save currently selected item data
    mmTreeItemData* iData = nullptr;
    wxString sectionName;
    wxTreeItemId selection = m_nav_tree_ctrl->GetSelection();
    if (selection.IsOk() && selectedItemData_) {
        iData = new mmTreeItemData(*selectedItemData_);
        // also save current section
        wxTreeItemId parentID = m_nav_tree_ctrl->GetItemParent(selection);
        if (parentID.IsOk() && parentID != m_nav_tree_ctrl->GetRootItem())
            sectionName = m_nav_tree_ctrl->GetItemText(parentID);
    }
    DoRecreateNavTreeControl();
    // Find and reselect the previously selected item
    if (iData) {
        // search for the item first under the selected section
        wxTreeItemId navTreeID = sectionName.empty() ? m_nav_tree_ctrl->GetRootItem() :
            getNavTreeChild(m_nav_tree_ctrl->GetRootItem(), sectionName);
        if (navTreeID.IsOk())
            navTreeID = findItemByData(navTreeID, *iData);
        // if we didn't find it search all nodes from root
        if (!navTreeID.IsOk())
            navTreeID = findItemByData(m_nav_tree_ctrl->GetRootItem(), *iData);
        if (navTreeID.IsOk()) {
            m_nav_tree_ctrl->EnsureVisible(navTreeID);
            m_nav_tree_ctrl->SelectItem(navTreeID);
        }
        delete(iData);
    }
}

wxTreeItemId mmGUIFrame::findItemByData(wxTreeItemId itemId, mmTreeItemData& searchData)
{
    // Check if the current item's data matches the search data
    if (itemId.IsOk())
        return wxTreeItemId();
    if (m_nav_tree_ctrl->GetItemData(itemId)) {
        if (*dynamic_cast<mmTreeItemData*>(m_nav_tree_ctrl->GetItemData(itemId)) == searchData)
            return itemId;
    }

    wxTreeItemIdValue searchCookie;
    wxTreeItemId childId = m_nav_tree_ctrl->GetFirstChild(itemId, searchCookie);
    while (childId.IsOk()) {
        // Recursively search for the item in the child subtree
        wxTreeItemId foundId = findItemByData(childId, searchData);
        if (foundId.IsOk()) {
            // Item found in the child subtree, return it
            return foundId;
        }

        // Not found yet, so move to the next child
        childId = m_nav_tree_ctrl->GetNextChild(itemId, searchCookie);
    }

    // Item not found
    return wxTreeItemId();
}

void mmGUIFrame::OnViewBudgetFinancialYears(wxCommandEvent& WXUNUSED(event))
{
    Option::instance().setBudgetFinancialYears(!Option::instance().getBudgetFinancialYears());
    refreshPanelData();
}

void mmGUIFrame::OnViewBudgetTransferTotal(wxCommandEvent& WXUNUSED(event))
{
    Option::instance().setBudgetIncludeTransfers(!Option::instance().getBudgetIncludeTransfers());
    refreshPanelData();
}

void mmGUIFrame::OnViewBudgetCategorySummary(wxCommandEvent& WXUNUSED(event))
{
    Option::instance().setBudgetSummaryWithoutCategories(!Option::instance().getBudgetSummaryWithoutCategories());
    refreshPanelData();
}

void mmGUIFrame::OnViewIgnoreFutureTransactions(wxCommandEvent& WXUNUSED(event))
{
    Option::instance().setIgnoreFutureTransactions(!Option::instance().getIgnoreFutureTransactions());
    RefreshNavigationTree();
}

void mmGUIFrame::OnViewShowToolTips(wxCommandEvent& WXUNUSED(event))
{
    Option::instance().setShowToolTips(!Option::instance().getShowToolTips());
    RefreshNavigationTree();
}

void mmGUIFrame::OnViewShowMoneyTips(wxCommandEvent& WXUNUSED(event))
{
    Option::instance().setShowMoneyTips(!Option::instance().getShowMoneyTips());
    RefreshNavigationTree();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnCategoryRelocation(wxCommandEvent& /*event*/)
{
    relocateCategoryDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK) {
        wxString msgStr;
        msgStr << _t("Merge categories completed") << "\n\n"
            << wxString::Format(_t("Records have been updated in the database: %i"),
                dlg.updatedCategoriesCount());
        wxMessageBox(msgStr, _t("Merge categories result"));
        refreshPanelData();
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnPayeeRelocation(wxCommandEvent& /*event*/)
{
    relocatePayeeDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK) {
        wxString msgStr;
        msgStr << _t("Merge payees completed") << "\n\n"
            << wxString::Format(_t("Records have been updated in the database: %i"),
                dlg.updatedPayeesCount())
            << "\n\n";
        wxMessageBox(msgStr, _t("Merge payees result"));
        refreshPanelData();
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnTagRelocation(wxCommandEvent& /*event*/)
{
    relocateTagDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK) {
        wxString msgStr;
        msgStr << _t("Merge tags completed") << "\n\n"
            << wxString::Format(_t("Records have been updated in the database: %i"),
                dlg.updatedTagsCount())
            << "\n\n";
        wxMessageBox(msgStr, _t("Merge tags result"));
        refreshPanelData();
    }
}
//----------------------------------------------------------------------------

wxSizer* mmGUIFrame::cleanupHomePanel(bool new_sizer)
{
    wxASSERT(homePanel_);

    homePanel_->DestroyChildren();
    homePanel_->SetSizer(new_sizer ? new wxBoxSizer(wxHORIZONTAL) : nullptr);

    return homePanel_->GetSizer();
}
//----------------------------------------------------------------------------

void mmGUIFrame::autocleanDeletedTransactions() {
    wxDateSpan days = wxDateSpan::Days(Model_Setting::instance().getInt("DELETED_TRANS_RETAIN_DAYS", 30));
    wxDateTime earliestDate = wxDateTime().Now().ToUTC().Subtract(days);
    Model_Checking::Data_Set deletedTransactions = Model_Checking::instance().find(Model_Checking::DELETEDTIME(earliestDate.FormatISOCombined(), LESS_OR_EQUAL), Model_Checking::DELETEDTIME(wxEmptyString, NOT_EQUAL));
    if (!deletedTransactions.empty()) {
        Model_Checking::instance().Savepoint();
        Model_Attachment::instance().Savepoint();
        Model_Splittransaction::instance().Savepoint();
        Model_CustomFieldData::instance().Savepoint();
        for (const auto& transaction : deletedTransactions) {
            // removing the checking transaction also removes split, translink, and share entries
            Model_Checking::instance().remove(transaction.TRANSID);

            // remove also any attachments for the transaction
            const wxString& RefType = Model_Attachment::REFTYPE_NAME_TRANSACTION;
            mmAttachmentManage::DeleteAllAttachments(RefType, transaction.TRANSID);

            // remove also any custom fields for the transaction
            Model_CustomFieldData::DeleteAllData(RefType, transaction.TRANSID);
        }
        Model_CustomFieldData::instance().ReleaseSavepoint();
        Model_Splittransaction::instance().ReleaseSavepoint();
        Model_Attachment::instance().ReleaseSavepoint();
        Model_Checking::instance().ReleaseSavepoint();
    }
}

void mmGUIFrame::SetDatabaseFile(const wxString& dbFileName, bool newDatabase)
{
    autoRepeatTransactionsTimer_.Stop();

    if (openFile(dbFileName, newDatabase)) {
        DoRecreateNavTreeControl(true);
        mmLoadColorsFromDatabase();
    }
    else {
        resetNavTreeControl();
        cleanupHomePanel();
        showBeginAppDialog(true);
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnRecentFiles(wxCommandEvent& event)
{
    int fileNum = event.GetId() - m_recentFiles->GetBaseId();
    if (fileNum == 0)
        return;
    const wxString file_name = m_recentFiles->GetHistoryFile(fileNum);
    wxFileName file(file_name);
    if (file.FileExists()) {
        SetDatabaseFile(file_name);
        saveSettings();
    }
    else {
        wxMessageBox(
            wxString::Format(_t("File %s not found."), file_name),
            _t("Error"),
            wxOK | wxICON_ERROR
        );
        m_recentFiles->RemoveFileFromHistory(fileNum);
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnClearRecentFiles(wxCommandEvent& /*event*/)
{
    m_recentFiles->ClearHistory();
    m_recentFiles->AddFileToHistory(m_filename);
}

void mmGUIFrame::setGotoAccountID(int64 account_id, Fused_Transaction::IdRepeat fused_id)
{
    gotoAccountID_ = account_id;
    gotoTransID_ = fused_id;
}

void mmGUIFrame::OnToggleFullScreen(wxCommandEvent& WXUNUSED(event))
{
#if (wxMAJOR_VERSION >= 3 && wxMINOR_VERSION >= 0)
    this->ShowFullScreen(!IsFullScreen());
#endif
}

void mmGUIFrame::OnResetView(wxCommandEvent& WXUNUSED(event))
{
    Model_Setting::instance().setBool("SHOWTOOLBAR", true);
    m_mgr.GetPane("toolbar").Show(true).Dock().Top().Position(0);
    m_mgr.GetPane("Navigation").Show(true).Dock().Left();
    m_mgr.Update();
}

void mmGUIFrame::OnClose(wxCloseEvent&)
{
    Destroy();
}

void mmGUIFrame::OnChangeGUILanguage(wxCommandEvent& event)
{
    wxLanguage lang = static_cast<wxLanguage>(event.GetId() - MENU_LANG - 1);
    if (lang != m_app->getGUILanguage() && m_app->setGUILanguage(lang))
        mmErrorDialogs::MessageWarning(
            this,
            _t("The language for this application has been changed. "
                "The change will take effect the next time the application is started."),
            _t("Language change")
        );
}

void mmGUIFrame::OnKeyDown(wxTreeEvent& event)
{
    if (selectedItemData_) {
        auto data = selectedItemData_->getString();

        int key_code = event.GetKeyCode();

        if (key_code == WXK_RETURN || key_code == WXK_NUMPAD_ENTER) {
            if (data == "item@Transaction Report")
                OnTransactionReport(event);
        }
    }
    event.Skip();
}

void mmGUIFrame::DoUpdateBudgetNavigation(wxTreeItemId& parent_item)
{
    const auto all_budgets = Model_Budgetyear::instance().all(Model_Budgetyear::COL_BUDGETYEARNAME);
    if (!all_budgets.empty()) {
        std::map <wxString, int64> years;

        wxRegEx pattern_year(R"(^([0-9]{4})$)");
        wxRegEx pattern_month(R"(^([0-9]{4})-([0-9]{2})$)");

        for (const auto& e : all_budgets) {
            const wxString& name = e.BUDGETYEARNAME;
            if (pattern_year.Matches(name)) {
                years[name] = e.BUDGETYEARID;
            }
            else {
                if (pattern_month.Matches(name)) {
                    wxString root_year = pattern_month.GetMatch(name, 1);
                    if (years.find(root_year) == years.end())
                        years[root_year] = e.BUDGETYEARID;
                }
            }
        }

        for (const auto& entry : years) {
            wxTreeItemId year_budget;
            for (const auto& e : all_budgets) {
                if (entry.second == e.BUDGETYEARID) {
                    year_budget = m_nav_tree_ctrl->AppendItem(parent_item, e.BUDGETYEARNAME, img::CALENDAR_PNG, img::CALENDAR_PNG);
                    m_nav_tree_ctrl->SetItemData(year_budget, new mmTreeItemData(mmTreeItemData::BUDGET, e.BUDGETYEARID));
                }
                else if (pattern_month.Matches(e.BUDGETYEARNAME) &&
                    pattern_month.GetMatch(e.BUDGETYEARNAME, 1) == entry.first
                ) {
                    wxTreeItemId month_budget = m_nav_tree_ctrl->AppendItem(
                        year_budget,
                        e.BUDGETYEARNAME,
                        img::CALENDAR_PNG, img::CALENDAR_PNG
                    );
                    m_nav_tree_ctrl->SetItemData(
                        month_budget,
                        new mmTreeItemData(mmTreeItemData::BUDGET, e.BUDGETYEARID)
                    );
                }
            }
        }
    }
}
