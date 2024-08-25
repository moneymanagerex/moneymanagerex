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
#include "columnorder.h"
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
EVT_MENU(MENU_COLUMN_ORDER, mmGUIFrame::OnColumnOrderManager)
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

mmGUIFrame::mmGUIFrame(mmGUIApp* app, const wxString& title
    , const wxPoint& pos
    , const wxSize& size)
    : wxFrame(nullptr, wxID_ANY, title, pos, size)
    , m_app(app)
    , autoRepeatTransactionsTimer_(this, AUTO_REPEAT_TRANSACTIONS_TIMER_ID)
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
    if (!dbpath.IsOk())
    {
        from_scratch = Model_Setting::instance().GetBoolSetting("SHOWBEGINAPP", true);
        if (from_scratch)
            dbpath = wxGetEmptyString();
        else
            dbpath = Model_Setting::instance().getLastDbPath();
    }

    //Read news, if checking enabled
    if (Option::instance().CheckNewsOnStartup())
        getNewsRSS(websiteNewsArray_);

    /* Create the Controls for the frame */
    mmFontSize(this);
    LoadTheme();
    createMenu();    
    createControls();
    CreateToolBar();

#if wxUSE_STATUSBAR
    CreateStatusBar();
#endif // wxUSE_STATUSBAR
    m_recentFiles = new mmFileHistory(); // TODO Max files
    m_recentFiles->SetMenuPathStyle(wxFH_PATH_SHOW_ALWAYS);
    m_recentFiles->UseMenu(m_menuRecentFiles);
    m_recentFiles->Load();

    // Load perspective
    const wxString auiPerspective = Model_Setting::instance()
        .GetStringSetting("AUIPERSPECTIVE", wxEmptyString);
    m_mgr.LoadPerspective(auiPerspective);

    // add the toolbars to the manager
    m_mgr.AddPane(toolBar_, wxAuiPaneInfo().
        Name("toolbar").ToolbarPane().Top()
        .LeftDockable(false).RightDockable(false)
        .Show(Model_Setting::instance().GetBoolSetting("SHOWTOOLBAR", true)));

    // change look and feel of wxAuiManager
    m_mgr.GetArtProvider()->SetMetric(16, 0);
    m_mgr.GetArtProvider()->SetMetric(3, 1);

    // "commit" all changes made to wxAuiManager
    m_mgr.GetPane("Navigation").Caption(_("Navigator"));
    m_mgr.GetPane("toolbar").Caption(_("Toolbar"));
    m_mgr.Update();

    // Show license agreement at first open
    if (Model_Setting::instance().GetStringSetting(INIDB_SEND_USAGE_STATS, "") == "")
    {
        mmAboutDialog(this, 4).ShowModal();
    }

    //Check for new version at startup
    if (Model_Setting::instance().GetBoolSetting("UPDATECHECK", true))
        mmUpdate::checkUpdates(this, true);

    //Show appstart
    if (from_scratch || !dbpath.IsOk())
    {
        menuEnableItems(false);
        showBeginAppDialog(dbpath.GetFullName().IsEmpty());
    }
    else
    {
        if (openFile(dbpath.GetFullPath(), false))
        {
            DoRecreateNavTreeControl(true);
            //setHomePageActive(false);
            mmLoadColorsFromDatabase();
        }
        else
        {
            resetNavTreeControl();
            cleanupHomePanel();
            showBeginAppDialog(true);
        }
    }

    const wxAcceleratorEntry entries[] =
    {
        wxAcceleratorEntry(wxACCEL_NORMAL, WXK_F9, wxID_NEW),
        wxAcceleratorEntry(wxACCEL_NORMAL, WXK_F5, wxID_REFRESH),
    };

    wxAcceleratorTable tab(sizeof(entries) / sizeof(*entries), entries);
    SetAcceleratorTable(tab);

    if (m_db) {
        // Clean up deleted transactions
        autocleanDeletedTransactions();

        // Refresh stock quotes
        if (!Model_Stock::instance().all().empty() && Model_Setting::instance().GetBoolSetting("REFRESH_STOCK_QUOTES_ON_OPEN", false))
        {
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
    for (const auto & model : this->m_all_models)
    {
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
    if (Option::instance().DatabaseUpdated() && Model_Setting::instance().GetBoolSetting("BACKUPDB_UPDATE", false))
    {
        dbUpgrade::BackupDB(m_filename, dbUpgrade::BACKUPTYPE::CLOSE, Model_Setting::instance().GetIntSetting("MAX_BACKUP_FILES", 4));
    }
}

void mmGUIFrame::ShutdownDatabase()
{
    if (m_db)
    {
        if (!Model_Infotable::instance().cache_.empty()) //Cache empty on InfoTable means instance never initialized
        {
            if (!db_lockInPlace)
                Model_Infotable::instance().Set("ISUSED", false);
        }
        m_db->SetCommitHook(nullptr);
        m_db->Close();
        m_db.reset();
        for (auto& model : m_all_models)
            model->destroyCache();
    }
}

void mmGUIFrame::resetNavTreeControl()
{
    wxTreeItemId root = m_nav_tree_ctrl->GetRootItem();
    cleanupNavTreeControl(root);
    m_nav_tree_ctrl->DeleteAllItems();
}

void mmGUIFrame::cleanupNavTreeControl(wxTreeItemId& item)
{
    while (item.IsOk())
    {
        if (m_nav_tree_ctrl->ItemHasChildren(item))
        {
            wxTreeItemIdValue cookie;
            wxTreeItemId childitem = m_nav_tree_ctrl->GetFirstChild(item, cookie);
            cleanupNavTreeControl(childitem);
        }
        mmTreeItemData* iData = dynamic_cast<mmTreeItemData*>(m_nav_tree_ctrl->GetItemData(item));
        m_nav_tree_ctrl->SetItemData(item, nullptr);
        if (iData)
            delete iData;
        item = m_nav_tree_ctrl->GetNextSibling(item);
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
// returns a wxTreeItemID for the accountName in the navtree section.
wxTreeItemId mmGUIFrame::getTreeItemfor(const wxTreeItemId& itemID, const wxString& accountName) const
{
    wxTreeItemIdValue treeDummyValue;
    wxTreeItemId navTreeID = m_nav_tree_ctrl->GetFirstChild(itemID, treeDummyValue);

    while (navTreeID.IsOk())
    {
        const auto text = m_nav_tree_ctrl->GetItemText(navTreeID);
        if (accountName == text)
            break;
        else
            navTreeID = m_nav_tree_ctrl->GetNextChild(itemID, treeDummyValue);
    }
    return navTreeID;
}

//----------------------------------------------------------------------------
bool mmGUIFrame::setAccountInSection(const wxString& sectionName, const wxString& accountName)
{
    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);
    bool accountFound = false;
    wxTreeItemId rootItem = getTreeItemfor(m_nav_tree_ctrl->GetRootItem()
        , wxGetTranslation(sectionName));
    if (rootItem.IsOk() && m_nav_tree_ctrl->ItemHasChildren(rootItem))
    {
        m_nav_tree_ctrl->ExpandAllChildren(rootItem);
        wxTreeItemId accountItem = getTreeItemfor(rootItem, accountName);
        if (accountItem.IsOk())
        {
            // Set the NavTreeCtrl and prevent any event code being executed for now.
            m_nav_tree_ctrl->SelectItem(accountItem);
            //processPendingEvents();
            accountFound = true;
        }
    }
    m_nav_tree_ctrl->SetEvtHandlerEnabled(true);
    return accountFound;
}

//----------------------------------------------------------------------------
bool mmGUIFrame::setNavTreeSection(const wxString &sectionName)
{
    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);
    bool accountNotFound = true;
    wxTreeItemId rootItem = getTreeItemfor(m_nav_tree_ctrl->GetRootItem(), sectionName);
    if (rootItem.IsOk())
    {
        // Set the NavTreeCtrl and prevent any event code being executed for now.
        m_nav_tree_ctrl->SelectItem(rootItem);
        //processPendingEvents();
        accountNotFound = false;
    }
    m_nav_tree_ctrl->SetEvtHandlerEnabled(true);

    return !accountNotFound;
}

//----------------------------------------------------------------------------
void mmGUIFrame::setAccountNavTreeSection(const wxString& accountName)
{
    if (setAccountInSection(wxTRANSLATE("Favorites"), accountName)) return;
    if (setAccountInSection(wxTRANSLATE("Bank Accounts"), accountName)) return;
    if (setAccountInSection(wxTRANSLATE("Credit Card Accounts"), accountName)) return;
    if (setAccountInSection(wxTRANSLATE("Term Accounts"), accountName)) return;
    if (setAccountInSection(wxTRANSLATE("Stock Portfolios"), accountName)) return;
    if (setAccountInSection(wxTRANSLATE("Cash Accounts"), accountName)) return;
    if (setAccountInSection(wxTRANSLATE("Loan Accounts"), accountName)) return;
    if (setAccountInSection(wxTRANSLATE("Assets"), accountName)) return;
}

//----------------------------------------------------------------------------
void mmGUIFrame::OnAutoRepeatTransactionsTimer(wxTimerEvent& /*event*/)
{
    //WebApp check
    if (mmWebApp::WebApp_CheckEnabled())
    {
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
    for (const auto& q1 : bills.all())
    {
        bills.decode_fields(q1);
        if (bills.autoExecuteManual() && bills.requireExecution())
        {
            if (bills.allowExecution() && bills.AllowTransaction(q1))
            {
                continueExecution = true;
                mmBDDialog repeatTransactionsDlg(this, q1.BDID, false, true);
                repeatTransactionsDlg.SetDialogHeader(_("Auto Repeat Transactions"));
                if (repeatTransactionsDlg.ShowModal() == wxID_OK)
                {
                    refreshPanelData();
                }
                else // stop repeat executions from occuring
                    continueExecution = false;
            }
        }

        if (bills.autoExecuteSilent() && bills.requireExecution())
        {
            if (bills.allowExecution() && bills.AllowTransaction(q1))
            {
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
                int transID = Model_Checking::instance().save(tran);

                Model_Splittransaction::Cache checking_splits;
                std::vector<wxArrayInt> splitTags;
                for (const auto &item : Model_Billsdeposits::splittransaction(q1))
                {
                    Model_Splittransaction::Data *split = Model_Splittransaction::instance().create();
                    split->TRANSID = transID;
                    split->CATEGID = item.CATEGID;
                    split->SPLITTRANSAMOUNT = item.SPLITTRANSAMOUNT;
                    split->NOTES = item.NOTES;
                    checking_splits.push_back(split);
                    wxArrayInt tags;
                    for (const auto& tag :
                         Model_Taglink::instance().find(Model_Taglink::REFTYPE(Model_Attachment::reftype_desc(Model_Attachment::BILLSDEPOSITSPLIT)),
                                                        Model_Taglink::REFID(item.SPLITTRANSID)))
                        tags.Add(tag.TAGID);
                    splitTags.push_back(tags);
                }
                Model_Splittransaction::instance().save(checking_splits);

                // Save split tags
                const wxString& splitRefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTIONSPLIT);

                for (size_t i = 0; i < checking_splits.size(); i++)
                {
                    Model_Taglink::Data_Set splitTaglinks;
                    for (const auto& tagId : splitTags.at(i))
                    {
                        Model_Taglink::Data* t = Model_Taglink::instance().create();
                        t->REFTYPE = splitRefType;
                        t->REFID = checking_splits[i]->SPLITTRANSID;
                        t->TAGID = tagId;
                        splitTaglinks.push_back(*t);
                    }
                    Model_Taglink::instance().update(splitTaglinks, splitRefType, checking_splits.at(i)->SPLITTRANSID);
                }

                // Copy the custom fields to the newly created transaction
                const auto& customDataSet = Model_CustomFieldData::instance().find(Model_CustomFieldData::REFID(-q1.BDID));
                Model_CustomFieldData::instance().Savepoint();
                for (const auto& entry : customDataSet)
                {
                    Model_CustomFieldData::Data* fieldData = Model_CustomFieldData::instance().create();
                    fieldData->FIELDID = entry.FIELDID;
                    fieldData->REFID = transID;
                    fieldData->CONTENT = entry.CONTENT;
                    Model_CustomFieldData::instance().save(fieldData);
                }
                Model_CustomFieldData::instance().ReleaseSavepoint();
                
                // Save base transaction tags
                Model_Taglink::Data_Set taglinks;
                const wxString& txnRefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
                for (const auto& tag : Model_Taglink::instance().find(Model_Taglink::REFTYPE(Model_Attachment::reftype_desc(Model_Attachment::BILLSDEPOSIT)),
                                                                      Model_Taglink::REFID(q1.BDID)))
                {
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
    {
        autoRepeatTransactionsTimer_.Start(5, wxTIMER_ONE_SHOT);
    }
    else
        refreshPanelData();
}
//----------------------------------------------------------------------------

/*
        Save our settings to ini db.
        */
void mmGUIFrame::saveSettings()
{
    Model_Setting::instance().Savepoint();
    if (!m_filename.IsEmpty())
    {
        wxFileName fname(m_filename);
        Model_Setting::instance().Set("LASTFILENAME", fname.GetFullPath());
    }
    /* Aui Settings */
    Model_Setting::instance().Set("AUIPERSPECTIVE", m_mgr.SavePerspective());

    // prevent values being saved while window is in an iconised state.
    if (this->IsIconized()) this->Restore();

    int value_x = 0, value_y = 0;
    this->GetPosition(&value_x, &value_y);
    Model_Setting::instance().Set("ORIGINX", value_x);
    Model_Setting::instance().Set("ORIGINY", value_y);

    int value_w = 0, value_h = 0;
    this->GetSize(&value_w, &value_h);
    Model_Setting::instance().Set("SIZEW", value_w);
    Model_Setting::instance().Set("SIZEH", value_h);
    Model_Setting::instance().Set("ISMAXIMIZED", this->IsMaximized());
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
    m_nav_tree_ctrl = new wxTreeCtrl(this, ID_NAVTREECTRL, wxDefaultPosition, wxDefaultSize,
        wxTR_SINGLE | wxTR_HAS_BUTTONS | wxTR_NO_LINES | wxTR_TWIST_BUTTONS);

    m_nav_tree_ctrl->SetMinSize(wxSize(100, 100));
    mmThemeMetaColour(m_nav_tree_ctrl, meta::COLOR_NAVPANEL);
    mmThemeMetaColour(m_nav_tree_ctrl, meta::COLOR_NAVPANEL_FONT, true);

    const auto navIconSize = Option::instance().getNavigationIconSize();
    m_nav_tree_ctrl->SetImages(navtree_images_list(navIconSize));

    m_nav_tree_ctrl->Connect(ID_NAVTREECTRL, wxEVT_TREE_SEL_CHANGED, wxTreeEventHandler(mmGUIFrame::OnSelChanged), nullptr, this);
    m_nav_tree_ctrl->Connect(ID_NAVTREECTRL, wxEVT_TREE_ITEM_RIGHT_CLICK, wxTreeEventHandler(mmGUIFrame::OnSelChanged), nullptr, this);
    
    homePanel_ = new wxPanel(this, wxID_ANY,
        wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxTR_SINGLE | wxNO_BORDER);

    m_mgr.AddPane(m_nav_tree_ctrl, wxAuiPaneInfo()
        .Name("Navigation")
        .BestSize(wxSize(200, 100)).MinSize(wxSize(100, 100))
        .Left());

    m_mgr.AddPane(homePanel_, wxAuiPaneInfo()
        .Name("Home").Caption("Home")
        .CenterPane().PaneBorder(false));
}
//----------------------------------------------------------------------------

void mmGUIFrame::DoRecreateNavTreeControl(bool home_page)
{
    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);
    if (home_page) {
        createHomePage();
    }

    DoWindowsFreezeThaw(m_nav_tree_ctrl);
    resetNavTreeControl();

    wxTreeItemId  root = m_nav_tree_ctrl->AddRoot(_("Dashboard"), img::HOUSE_PNG, img::HOUSE_PNG);
    m_nav_tree_ctrl->SetItemData(root, new mmTreeItemData(mmTreeItemData::HOME_PAGE, "Dashboard"));
    m_nav_tree_ctrl->SetItemBold(root, true);

    wxTreeItemId alltransactions = m_nav_tree_ctrl->AppendItem(root, _("All Transactions"), img::ALLTRANSACTIONS_PNG, img::ALLTRANSACTIONS_PNG);
    m_nav_tree_ctrl->SetItemData(alltransactions, new mmTreeItemData(mmTreeItemData::ALL_TRANSACTIONS, "All Transactions"));
    m_nav_tree_ctrl->SetItemBold(alltransactions, true);

    wxTreeItemId favorites = m_nav_tree_ctrl->AppendItem(root, _("Favorites"), img::FAVOURITE_PNG, img::FAVOURITE_PNG);
    m_nav_tree_ctrl->SetItemData(favorites, new mmTreeItemData(mmTreeItemData::MENU_FAVORITES, "Favorites"));
    m_nav_tree_ctrl->SetItemBold(favorites, true);

    wxTreeItemId accounts = m_nav_tree_ctrl->AppendItem(root, _("Bank Accounts"), img::SAVINGS_ACC_NORMAL_PNG, img::SAVINGS_ACC_NORMAL_PNG);
    m_nav_tree_ctrl->SetItemData(accounts, new mmTreeItemData(mmTreeItemData::MENU_ACCOUNT, "Bank Accounts"));
    m_nav_tree_ctrl->SetItemBold(accounts, true);

    wxTreeItemId cardAccounts = m_nav_tree_ctrl->AppendItem(root, _("Credit Card Accounts"), img::CARD_ACC_NORMAL_PNG, img::CARD_ACC_NORMAL_PNG);
    m_nav_tree_ctrl->SetItemData(cardAccounts, new mmTreeItemData(mmTreeItemData::MENU_ACCOUNT, "Credit Card Accounts"));
    m_nav_tree_ctrl->SetItemBold(cardAccounts, true);

    wxTreeItemId cashAccounts = m_nav_tree_ctrl->AppendItem(root, _("Cash Accounts"), img::CASH_ACC_NORMAL_PNG, img::CASH_ACC_NORMAL_PNG);
    m_nav_tree_ctrl->SetItemData(cashAccounts, new mmTreeItemData(mmTreeItemData::MENU_ACCOUNT, "Cash Accounts"));
    m_nav_tree_ctrl->SetItemBold(cashAccounts, true);

    wxTreeItemId loanAccounts = m_nav_tree_ctrl->AppendItem(root, _("Loan Accounts"), img::LOAN_ACC_NORMAL_PNG, img::LOAN_ACC_NORMAL_PNG);
    m_nav_tree_ctrl->SetItemData(loanAccounts, new mmTreeItemData(mmTreeItemData::MENU_ACCOUNT, "Loan Accounts"));
    m_nav_tree_ctrl->SetItemBold(loanAccounts, true);

    wxTreeItemId termAccounts = m_nav_tree_ctrl->AppendItem(root, _("Term Accounts"), img::TERMACCOUNT_NORMAL_PNG, img::TERMACCOUNT_NORMAL_PNG);
    m_nav_tree_ctrl->SetItemData(termAccounts, new mmTreeItemData(mmTreeItemData::MENU_ACCOUNT, "Term Accounts"));
    m_nav_tree_ctrl->SetItemBold(termAccounts, true);

    wxTreeItemId stocks = m_nav_tree_ctrl->AppendItem(root, _("Stock Portfolios"), img::STOCK_ACC_NORMAL_PNG, img::STOCK_ACC_NORMAL_PNG);
    m_nav_tree_ctrl->SetItemData(stocks, new mmTreeItemData(mmTreeItemData::HELP_PAGE_STOCKS, "Stock Portfolios"));
    m_nav_tree_ctrl->SetItemBold(stocks, true);

    wxTreeItemId shareAccounts = m_nav_tree_ctrl->AppendItem(root, _("Share Accounts"), img::STOCK_ACC_NORMAL_PNG, img::STOCK_ACC_NORMAL_PNG);
    m_nav_tree_ctrl->SetItemData(shareAccounts, new mmTreeItemData(mmTreeItemData::MENU_ACCOUNT, "Share Accounts"));
    m_nav_tree_ctrl->SetItemBold(shareAccounts, true);

    wxTreeItemId assets = m_nav_tree_ctrl->AppendItem(root, _("Assets"), img::ASSET_NORMAL_PNG, img::ASSET_NORMAL_PNG);
    m_nav_tree_ctrl->SetItemData(assets, new mmTreeItemData(mmTreeItemData::ASSETS, "Assets"));
    m_nav_tree_ctrl->SetItemBold(assets, true);

    wxTreeItemId bills = m_nav_tree_ctrl->AppendItem(root, _("Scheduled Transactions"), img::SCHEDULE_PNG, img::SCHEDULE_PNG);
    m_nav_tree_ctrl->SetItemData(bills, new mmTreeItemData(mmTreeItemData::BILLS, "Scheduled Transactions"));
    m_nav_tree_ctrl->SetItemBold(bills, true);

    wxTreeItemId trash = m_nav_tree_ctrl->AppendItem(root, _("Deleted Transactions"), img::TRASH_PNG, img::TRASH_PNG);
    m_nav_tree_ctrl->SetItemData(trash, new mmTreeItemData(mmTreeItemData::TRASH, "Deleted Transactions"));
    m_nav_tree_ctrl->SetItemBold(trash, true);

    wxTreeItemId budgeting = m_nav_tree_ctrl->AppendItem(root, _("Budget Planner"), img::CALENDAR_PNG, img::CALENDAR_PNG);
    m_nav_tree_ctrl->SetItemData(budgeting, new mmTreeItemData(mmTreeItemData::HELP_BUDGET, "Budget Setup"));
    m_nav_tree_ctrl->SetItemBold(budgeting, true);
    this->DoUpdateBudgetNavigation(budgeting);

    wxTreeItemId transactionFilter = m_nav_tree_ctrl->AppendItem(root, _("Transaction Report"), img::FILTER_PNG, img::FILTER_PNG);
    m_nav_tree_ctrl->SetItemBold(transactionFilter, true);
    m_nav_tree_ctrl->SetItemData(transactionFilter, new mmTreeItemData(mmTreeItemData::FILTER, "Transaction Report"));
    this->DoUpdateFilterNavigation(transactionFilter);

    wxTreeItemId reports = m_nav_tree_ctrl->AppendItem(root, _("Reports"), img::PIECHART_PNG, img::PIECHART_PNG);
    m_nav_tree_ctrl->SetItemBold(reports, true);
    m_nav_tree_ctrl->SetItemData(reports, new mmTreeItemData(mmTreeItemData::HELP_REPORT, "Reports"));
    this->DoUpdateReportNavigation(reports);

    wxTreeItemId grm = m_nav_tree_ctrl->AppendItem(root, _("General Report Manager"), img::CUSTOMSQL_GRP_PNG, img::CUSTOMSQL_GRP_PNG);
    m_nav_tree_ctrl->SetItemBold(grm, true);
    m_nav_tree_ctrl->SetItemData(grm, new mmTreeItemData(mmTreeItemData::HELP_PAGE_GRM, "General Report Manager"));
    this->DoUpdateGRMNavigation(grm);

    ///////////////////////////////////////////////////////////////////

    wxTreeItemId help = m_nav_tree_ctrl->AppendItem(root, _("Help"), img::HELP_PNG, img::HELP_PNG);
    m_nav_tree_ctrl->SetItemData(help, new mmTreeItemData(mmTreeItemData::HELP_PAGE_MAIN, "Help"));
    m_nav_tree_ctrl->SetItemBold(help, true);

    bool hideShareAccounts = Option::instance().HideShareAccounts();

    if (m_db)
    {
        /* Start Populating the dynamic data */
        m_temp_view = Model_Setting::instance().GetViewAccounts();
        wxASSERT(m_temp_view == VIEW_ACCOUNTS_ALL_STR || m_temp_view == VIEW_ACCOUNTS_FAVORITES_STR
            || m_temp_view == VIEW_ACCOUNTS_OPEN_STR || m_temp_view == VIEW_ACCOUNTS_CLOSED_STR);

        for (const auto& account : Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME))
        {
            if ((m_temp_view == VIEW_ACCOUNTS_OPEN_STR) && (Model_Account::status(account) != Model_Account::OPEN))
                continue;
            else if (m_temp_view == VIEW_ACCOUNTS_CLOSED_STR && (Model_Account::status(account) == Model_Account::OPEN))
                continue;
            else if (m_temp_view == VIEW_ACCOUNTS_FAVORITES_STR && !Model_Account::FAVORITEACCT(account))
                continue;

            int selectedImage = Option::instance().AccountImageId(account.ACCOUNTID, false);

            wxTreeItemId tacct;
            Model_Account::TYPE account_type = Model_Account::type(account);
            if (Model_Account::FAVORITEACCT(account) && (Model_Account::status(account) == Model_Account::OPEN))
            {
                if (Model_Account::type(account) != Model_Account::INVESTMENT &&
                    (account_type != Model_Account::SHARES || !hideShareAccounts))
                {
                    tacct = m_nav_tree_ctrl->AppendItem(favorites, account.ACCOUNTNAME, selectedImage, selectedImage);
                    m_nav_tree_ctrl->SetItemData(tacct, new mmTreeItemData(mmTreeItemData::ACCOUNT, account.ACCOUNTID));
                }
            }

            switch (account_type)
            {
            case Model_Account::INVESTMENT:
            {
                tacct = m_nav_tree_ctrl->AppendItem(stocks, account.ACCOUNTNAME, selectedImage, selectedImage);
                m_nav_tree_ctrl->SetItemData(tacct, new mmTreeItemData(mmTreeItemData::STOCK, account.ACCOUNTID));
                // find all the accounts associated with this stock portfolio
                Model_Stock::Data_Set stock_account_list = Model_Stock::instance().find(Model_Stock::HELDAT(account.ACCOUNTID));
                std::sort(stock_account_list.begin(), stock_account_list.end(), SorterBySTOCKNAME());
                // Put the names of the Stock_entry names as children of the stock account.
                for (const auto& stock_entry : stock_account_list)
                {
                    if (Model_Translink::HasShares(stock_entry.STOCKID))
                    {
                        wxTreeItemId se = m_nav_tree_ctrl->AppendItem(tacct, stock_entry.STOCKNAME, selectedImage, selectedImage);
                        int account_id = stock_entry.STOCKID;
                        if (Model_Translink::ShareAccountId(account_id))
                        {
                            m_nav_tree_ctrl->SetItemData(se, new mmTreeItemData(mmTreeItemData::ACCOUNT, account_id));
                        }
                    }
                }
                break;
            }
            case Model_Account::CHECKING:
                tacct = m_nav_tree_ctrl->AppendItem(accounts, account.ACCOUNTNAME, selectedImage, selectedImage);
                m_nav_tree_ctrl->SetItemData(tacct, new mmTreeItemData(mmTreeItemData::ACCOUNT, account.ACCOUNTID));
                break;
            case Model_Account::SHARES:
                if (!hideShareAccounts)
                {
                    tacct = m_nav_tree_ctrl->AppendItem(shareAccounts, account.ACCOUNTNAME, selectedImage, selectedImage);
                    m_nav_tree_ctrl->SetItemData(tacct, new mmTreeItemData(mmTreeItemData::ACCOUNT, account.ACCOUNTID));
                }
                break;
            case Model_Account::ASSET:
                tacct = m_nav_tree_ctrl->AppendItem(assets, account.ACCOUNTNAME, selectedImage, selectedImage);
                m_nav_tree_ctrl->SetItemData(tacct, new mmTreeItemData(mmTreeItemData::ACCOUNT, account.ACCOUNTID));
                break;
            case Model_Account::TERM:
                tacct = m_nav_tree_ctrl->AppendItem(termAccounts, account.ACCOUNTNAME, selectedImage, selectedImage);
                m_nav_tree_ctrl->SetItemData(tacct, new mmTreeItemData(mmTreeItemData::ACCOUNT, account.ACCOUNTID));
                break;
            case Model_Account::CREDIT_CARD:
                tacct = m_nav_tree_ctrl->AppendItem(cardAccounts, account.ACCOUNTNAME, selectedImage, selectedImage);
                m_nav_tree_ctrl->SetItemData(tacct, new mmTreeItemData(mmTreeItemData::ACCOUNT, account.ACCOUNTID));
                break;
            case Model_Account::CASH:
                tacct = m_nav_tree_ctrl->AppendItem(cashAccounts, account.ACCOUNTNAME, selectedImage, selectedImage);
                m_nav_tree_ctrl->SetItemData(tacct, new mmTreeItemData(mmTreeItemData::ACCOUNT, account.ACCOUNTID));
                break;
            case Model_Account::LOAN:
                tacct = m_nav_tree_ctrl->AppendItem(loanAccounts, account.ACCOUNTNAME, selectedImage, selectedImage);
                m_nav_tree_ctrl->SetItemData(tacct, new mmTreeItemData(mmTreeItemData::ACCOUNT, account.ACCOUNTID));
                break;
            default:
                wxASSERT(0);
                break;
            }

        }

        loadNavigationTreeItemsStatusFromJson();

        if (!m_nav_tree_ctrl->ItemHasChildren(favorites)) {
            m_nav_tree_ctrl->Delete(favorites);
        }
        if (!m_nav_tree_ctrl->ItemHasChildren(accounts)) {
            m_nav_tree_ctrl->Delete(accounts);
        }
        if (!m_nav_tree_ctrl->ItemHasChildren(cardAccounts)) {
            m_nav_tree_ctrl->Delete(cardAccounts);
        }
        if (!m_nav_tree_ctrl->ItemHasChildren(termAccounts)) {
            m_nav_tree_ctrl->Delete(termAccounts);
        }
        if (!m_nav_tree_ctrl->ItemHasChildren(stocks)) {
            m_nav_tree_ctrl->Delete(stocks);
        }
        if (!m_nav_tree_ctrl->ItemHasChildren(cashAccounts)) {
            m_nav_tree_ctrl->Delete(cashAccounts);
        }
        if (!m_nav_tree_ctrl->ItemHasChildren(loanAccounts)) {
            m_nav_tree_ctrl->Delete(loanAccounts);
        }
        if (!m_nav_tree_ctrl->ItemHasChildren(shareAccounts) || hideShareAccounts)
        {
            m_nav_tree_ctrl->Delete(shareAccounts);
        }
        if (Model_Checking::instance().find(Model_Checking::DELETEDTIME(wxEmptyString, NOT_EQUAL)).empty() || Option::instance().HideDeletedTransactions())
        {
            m_nav_tree_ctrl->Delete(trash);
            if (panelCurrent_ && panelCurrent_->GetId() == mmID_DELETEDTRANSACTIONS) {
                wxCommandEvent event(wxEVT_MENU, MENU_HOMEPAGE);
                GetEventHandler()->AddPendingEvent(event);
            }
        }
    }
    m_nav_tree_ctrl->EnsureVisible(root);
    if (home_page) m_nav_tree_ctrl->SelectItem(m_nav_tree_ctrl->GetRootItem());
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
    m_nav_tree_ctrl->Expand(root);

    const wxString& str = Model_Infotable::instance().GetStringInfo("NAV_TREE_STATUS", "");
    Document json_doc;
    if (json_doc.Parse(str.utf8_str()).HasParseError()) {
        json_doc.Parse("{}");
    }

    std::stack<wxTreeItemId> items;
    if (m_nav_tree_ctrl->GetRootItem().IsOk()) {
        items.push(m_nav_tree_ctrl->GetRootItem());
    }

    while (!items.empty())
    {
        wxTreeItemId next = items.top();
        items.pop();

        wxTreeItemIdValue cookie;
        wxTreeItemId nextChild = m_nav_tree_ctrl->GetFirstChild(next, cookie);
        while (nextChild.IsOk())
        {
            if (m_nav_tree_ctrl->HasChildren(nextChild)) items.push(nextChild);
            nextChild = m_nav_tree_ctrl->GetNextSibling(nextChild);
        }

        mmTreeItemData* iData =
            dynamic_cast<mmTreeItemData*>(m_nav_tree_ctrl->GetItemData(next));
        if (iData)
        {
            const wxString nav_key = iData->getString();
            if (json_doc.HasMember(nav_key.utf8_str()))
            {
                Value json_key(nav_key.utf8_str(), json_doc.GetAllocator());
                if (json_doc[json_key].IsBool() && json_doc[json_key].GetBool())
                {
                    m_nav_tree_ctrl->Expand(next);
                }
            }
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
//----------------------------------------------------------------------------

void mmGUIFrame::OnTreeItemCollapsing(wxTreeEvent& event)
{
    mmTreeItemData* iData =
        dynamic_cast<mmTreeItemData*>(m_nav_tree_ctrl->GetItemData(event.GetItem()));

    // disallow collapsing of HOME item
    if (mmTreeItemData::HOME_PAGE == iData->getType())
        event.Veto();
}

//----------------------------------------------------------------------------

void mmGUIFrame::OnTreeItemCollapsed(wxTreeEvent& event)
{
    mmTreeItemData* iData =
        dynamic_cast<mmTreeItemData*>(m_nav_tree_ctrl->GetItemData(event.GetItem()));
    if (!iData) return;
    navTreeStateToJson();
}

void mmGUIFrame::OnDropFiles(wxDropFilesEvent& event)
{
    int id = panelCurrent_->GetId();
    if (!(id == mmID_CHECKING || id == mmID_ALLTRANSACTIONS)) return;
    
    if (event.GetNumberOfFiles() > 0) {
        wxString* dropped = event.GetFiles();
        wxASSERT(dropped);

        wxString name;
        wxArrayString files;

        for (int i = 0; i < event.GetNumberOfFiles(); i++) {
            name = dropped[i];
            if (wxFileExists(name))
                files.push_back(name);
            else if (wxDirExists(name))
                wxDir::GetAllFiles(name, &files);
        }

        for (size_t i = 0; i < files.size(); i++) {
            wxString ext = wxFileName(files[i]).GetExt().MakeLower();
            if (ext == "csv" || ext == "xml")
            {
                mmUnivCSVDialog(this, (ext == "csv" ? mmUnivCSVDialog::DIALOG_TYPE_IMPORT_CSV : mmUnivCSVDialog::DIALOG_TYPE_IMPORT_XML), gotoAccountID_, files[i]).ShowModal();
            }
            else if (ext == "qif")
            {
                mmQIFImportDialog(this, gotoAccountID_, files[i]).ShowModal();
            }
        }
        refreshPanelData();
    }
}

void mmGUIFrame::navTreeStateToJson()
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();

    std::stack<wxTreeItemId> items;
    if (m_nav_tree_ctrl->GetRootItem().IsOk())
        items.push(m_nav_tree_ctrl->GetRootItem());

    while (!items.empty())
    {
        wxTreeItemId next = items.top();
        items.pop();

        wxTreeItemIdValue cookie;
        wxTreeItemId nextChild = m_nav_tree_ctrl->GetFirstChild(next, cookie);
        while (nextChild.IsOk())
        {
            if (m_nav_tree_ctrl->HasChildren(nextChild)) items.push(nextChild);
            nextChild = m_nav_tree_ctrl->GetNextSibling(nextChild);
        }

        mmTreeItemData* iData = dynamic_cast<mmTreeItemData*>(m_nav_tree_ctrl->GetItemData(next));
        if (iData && !iData->getString().empty() && m_nav_tree_ctrl->IsExpanded(next))
        {
            json_writer.Key(iData->getString().utf8_str());
            json_writer.Bool(m_nav_tree_ctrl->IsExpanded(next));
        }
    };
    json_writer.EndObject();

    const wxString nav_tree_status = wxString::FromUTF8(json_buffer.GetString());
    wxLogDebug("=========== navTreeStateToJson =============================");
    wxLogDebug(nav_tree_status);
    Model_Infotable::instance().Set("NAV_TREE_STATUS", nav_tree_status);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnSelChanged(wxTreeEvent& event)
{
    if (!m_db) return;

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
    switch (iData->getType())
    {
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
    case mmTreeItemData::FILTER:
        return OnTransactionReport(e);
    case mmTreeItemData::ASSETS:
        return OnAssets(e);
    case mmTreeItemData::BILLS:
        return OnBillsDeposits(e);
    case  mmTreeItemData::ALL_TRANSACTIONS:
    {
        DragAcceptFiles(true);
        return createAllTransactionsPage();
    }
    case  mmTreeItemData::TRASH:
        return createDeletedTransactionsPage();
    case mmTreeItemData::BUDGET:
        return createBudgetingPage(iData->getData());
    case mmTreeItemData::REPORT:
        activeReport_ = true;
        return createReportsPage(iData->get_report(), false);
    case mmTreeItemData::GRM:
        activeReport_ = true;
        return createReportsPage(iData->get_report(), false);
    case mmTreeItemData::FILTER_REPORT:
    {
        activeReport_ = true;
        wxSharedPtr<mmFilterTransactionsDialog> dlg(new mmFilterTransactionsDialog(this, iData->getString()));
        mmReportTransactions* rs = new mmReportTransactions(dlg);
        return createReportsPage(rs, true);
    }
    case mmTreeItemData::ACCOUNT:
    {
        DragAcceptFiles(true);
        Model_Account::Data* account = Model_Account::instance().get(iData->getData());
        gotoAccountID_ = account->ACCOUNTID;
        return createCheckingAccountPage(gotoAccountID_);
    }
    case mmTreeItemData::STOCK:
    {
        Model_Account::Data* account = Model_Account::instance().get(iData->getData());
        gotoAccountID_ = account->ACCOUNTID;
        return createStocksAccountPage(gotoAccountID_);
    }
    }
    wxLogDebug("");

}
//----------------------------------------------------------------------------

void mmGUIFrame::OnLaunchAccountWebsite(wxCommandEvent& /*event*/)
{
    if (selectedItemData_)
    {
        int data = selectedItemData_->getData();
        Model_Account::Data* account = Model_Account::instance().get(data);
        if (account)
        {
            wxString website = account->WEBSITE;
            if (!website.IsEmpty()) wxLaunchDefaultBrowser(website);
            return;
        }
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnAccountAttachments(wxCommandEvent& /*event*/)
{
    if (selectedItemData_)
    {
        int RefId = selectedItemData_->getData();
        wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::BANKACCOUNT);

        mmAttachmentDialog dlg(this, RefType, RefId);
        dlg.ShowModal();
    }
}

//----------------------------------------------------------------------------
void mmGUIFrame::OnPopupEditAccount(wxCommandEvent& /*event*/)
{
    if (selectedItemData_)
    {
        int data = selectedItemData_->getData();
        Model_Account::Data* account = Model_Account::instance().get(data);
        if (account)
        {
            mmNewAcctDialog dlg(account, this);
            if (dlg.ShowModal() == wxID_OK) {
                RefreshNavigationTree();
            }
        }
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnPopupReallocateAccount(wxCommandEvent& WXUNUSED(event))
{
    if (selectedItemData_)
    {
        int account_id = selectedItemData_->getData();
        ReallocateAccount(account_id);
    }
}

//----------------------------------------------------------------------------

void mmGUIFrame::OnPopupDeleteFilter(wxCommandEvent& /*event*/)
{
    if (!m_db) return;

    wxString data = selectedItemData_->getString();
    Document j_doc;
    if (j_doc.Parse(data.utf8_str()).HasParseError())
        j_doc.Parse("{}");
    Value& j_label = GetValueByPointerWithDefault(j_doc, "/LABEL", "");
    wxString selected = j_label.IsString() ? wxString::FromUTF8(j_label.GetString()) : "";

    if (wxMessageBox(
        _("The selected item will be deleted") + "\n\n" +
        _("Do you wish to continue?")
        , _("Settings item deletion"), wxYES_NO | wxICON_WARNING) == wxNO)
        return;

    int sel_json = Model_Infotable::instance().FindLabelInJSON("TRANSACTIONS_FILTER", selected);
    if (sel_json != wxNOT_FOUND)
    {
        Model_Infotable::instance().Erase("TRANSACTIONS_FILTER", sel_json);
        RefreshNavigationTree();
    }
}
//--------------------------------------------------------------------------

void mmGUIFrame::OnPopupRenameFilter(wxCommandEvent& /*event*/)
{
    if (!m_db) return;
    wxString data = selectedItemData_->getString();
    Document j_doc;
    if (j_doc.Parse(data.utf8_str()).HasParseError())
        j_doc.Parse("{}");
    Value& j_label = GetValueByPointerWithDefault(j_doc, "/LABEL", "");
    wxString selected = j_label.IsString() ? wxString::FromUTF8(j_label.GetString()) : "";

    wxString new_name;
    bool nameOK = false;
    while (!nameOK)
    {
        new_name = wxGetTextFromUser(_("Setting Name"), _("Please Enter"), selected);
        if (new_name.empty())
            return;
        if (wxNOT_FOUND == Model_Infotable::instance().FindLabelInJSON("TRANSACTIONS_FILTER", new_name))
            nameOK = true;
        else
        {
            wxString msgStr = wxString() << _("A setting with this name already exists") << "\n"
                << "\n"
                << _("Please specify a new name for the setting") << "\n";

            wxMessageBox(msgStr, _("Name in use"), wxICON_ERROR);
        }
    }

    int sel_json = Model_Infotable::instance().FindLabelInJSON("TRANSACTIONS_FILTER", selected);
    if (sel_json != wxNOT_FOUND)
    {
        Model_Infotable::instance().Erase("TRANSACTIONS_FILTER", sel_json);

        // Change the name
        Value::MemberIterator v_name = j_doc.FindMember("LABEL");
        v_name->value.SetString(new_name.mb_str(), j_doc.GetAllocator());
        // Serialize the new entry
        StringBuffer buffer;
        buffer.Clear();
        PrettyWriter<StringBuffer> writer(buffer);
        writer.SetFormatOptions(kFormatSingleLineArray);
        j_doc.Accept(writer);
        data = wxString::FromUTF8(buffer.GetString());
        Model_Infotable::instance().Prepend("TRANSACTIONS_FILTER", data, -1);

        DoRecreateNavTreeControl();
        setNavTreeSection(_("Transaction Report"));
    }
}
//--------------------------------------------------------------------------

void mmGUIFrame::OnPopupEditFilter(wxCommandEvent& /*event*/)
{
    if (!m_db) return;
    if (Model_Account::instance().all().empty()) return;
    wxString data = selectedItemData_->getString();

    const auto filter_settings = Model_Infotable::instance().GetArrayStringSetting("TRANSACTIONS_FILTER");

    wxSharedPtr<mmFilterTransactionsDialog> dlg(new mmFilterTransactionsDialog(this, -1, true, data));
    bool is_ok = (dlg->ShowModal() == wxID_OK);
    if (filter_settings != Model_Infotable::instance().GetArrayStringSetting("TRANSACTIONS_FILTER")) {
        DoRecreateNavTreeControl();
        setNavTreeSection(_("Transaction Report"));
    }

    if (is_ok) {
        mmReportTransactions* rs = new mmReportTransactions(dlg);
        createReportsPage(rs, true);
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnPopupDeleteAccount(wxCommandEvent& /*event*/)
{
    if (selectedItemData_)
    {
        int data = selectedItemData_->getData();
        Model_Account::Data* account = Model_Account::instance().get(data);
        if (account)
        {
            wxString warning_msg = _("Do you really want to delete the account?");
            if (account->ACCOUNTTYPE == Model_Account::all_type()[Model_Account::INVESTMENT] || account->ACCOUNTTYPE == Model_Account::all_type()[Model_Account::SHARES])
            {
                warning_msg += "\n\nThis will also delete any associated Shares.";
            }
            wxMessageDialog msgDlg(this
                , warning_msg
                , _("Confirm Account Deletion")
                , wxYES_NO | wxNO_DEFAULT | wxICON_ERROR);
            if (msgDlg.ShowModal() == wxID_YES)
            {
                Model_Account::instance().remove(account->ACCOUNTID);
                mmAttachmentManage::DeleteAllAttachments(Model_Attachment::reftype_desc(Model_Attachment::BANKACCOUNT), account->ACCOUNTID);
                DoRecreateNavTreeControl(true);
            }
        }
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
        wxMessageBox(_("MMEX has been opened without an active database.")
            , _("MMEX: Menu Popup Error"), wxOK | wxICON_EXCLAMATION);
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
    menu.AppendSubMenu(importFrom, _("&Import from"));
    importFrom->Append(MENU_TREEPOPUP_ACCOUNT_IMPORTUNIVCSV, _("&CSV Files..."));
    importFrom->Append(MENU_TREEPOPUP_ACCOUNT_IMPORTXML, _("&XML Files..."), _("Import from XML file (Excel format)"));
    importFrom->AppendSeparator();
    importFrom->Append(MENU_TREEPOPUP_ACCOUNT_IMPORTQIF, _("&QIF Files..."));

    wxMenu* exportTo(new wxMenu);
    menu.AppendSubMenu(exportTo, _("&Export as"));
    exportTo->Append(MENU_TREEPOPUP_ACCOUNT_EXPORT2CSV, _("&CSV File..."));
    exportTo->Append(MENU_TREEPOPUP_ACCOUNT_EXPORT2XML, _("&XML File..."));
    exportTo->AppendSeparator();
    exportTo->Append(MENU_TREEPOPUP_ACCOUNT_EXPORT2MMEX, _("&MMEX CSV File..."));
    exportTo->Append(MENU_TREEPOPUP_ACCOUNT_EXPORT2JSON, _("&JSON File..."));
    exportTo->Append(MENU_TREEPOPUP_ACCOUNT_EXPORT2QIF, _("&QIF File..."));
}

void mmGUIFrame::showTreePopupMenu(const wxTreeItemId& id, const wxPoint& pt)
{
    mmTreeItemData* iData = dynamic_cast<mmTreeItemData*>(m_nav_tree_ctrl->GetItemData(id));
    if (iData)
        selectedItemData_ = iData;
    else
        return;
    wxCommandEvent e;
    wxMenu menu;
    switch (iData->getType())
    {
    case mmTreeItemData::HOME_PAGE:
        return OnThemeManager(e);
    case mmTreeItemData::HELP_BUDGET:
    case mmTreeItemData::BUDGET:
        return OnBudgetSetupDialog(e);
    case mmTreeItemData::FILTER:
        return OnTransactionReport(e);
    case mmTreeItemData::FILTER_REPORT:
    {
        const wxString data = iData->getString();
        wxLogDebug("MENU FILTER: %s", data);
        menu.Append(MENU_TREEPOPUP_FILTER_EDIT, _("&Customize Report..."));
        menu.Append(MENU_TREEPOPUP_FILTER_RENAME, _("&Rename Report..."));
        menu.Append(MENU_TREEPOPUP_FILTER_DELETE, _("&Delete Report..."));
        PopupMenu(&menu, pt);
        break;
    }
    case  mmTreeItemData::GRM:
    case  mmTreeItemData::HELP_PAGE_GRM:
        return OnGeneralReportManager(e);
    case mmTreeItemData::HELP_REPORT:
        return mmDoHideReportsDialog();
    case mmTreeItemData::STOCK:
    {
        int data = iData->getData();

        Model_Account::Data* account = Model_Account::instance().get(data);
        if (account)
        {
            menu.Append(MENU_TREEPOPUP_EDIT, _("&Edit Account..."));
            menu.Append(MENU_TREEPOPUP_DELETE, _("&Delete Account..."));
            menu.AppendSeparator();
            menu.Append(MENU_TREEPOPUP_LAUNCHWEBSITE, _("&Launch Account Website"));
            menu.Append(MENU_TREEPOPUP_ACCOUNTATTACHMENTS, _("&Attachment Manager..."));
            menu.Enable(MENU_TREEPOPUP_LAUNCHWEBSITE, !account->WEBSITE.IsEmpty());

            PopupMenu(&menu, pt);
        }
        break;
    }
    case mmTreeItemData::ACCOUNT:
    {
        int data = iData->getData();

        Model_Account::Data* account = Model_Account::instance().get(data);
        if (account)
        {
            menu.Append(MENU_TREEPOPUP_EDIT, _("&Edit Account..."));
            menu.Append(MENU_TREEPOPUP_REALLOCATE, _("&Change Account Type..."));
            menu.AppendSeparator();
            menu.Append(MENU_TREEPOPUP_DELETE, _("&Delete Account..."));
            menu.AppendSeparator();
            menu.Append(MENU_TREEPOPUP_LAUNCHWEBSITE, _("&Launch Account Website"));
            menu.Append(MENU_TREEPOPUP_ACCOUNTATTACHMENTS, _("&Attachment Manager..."));
            menu.Enable(MENU_TREEPOPUP_LAUNCHWEBSITE, !account->WEBSITE.IsEmpty());
            menu.Enable(MENU_TREEPOPUP_REALLOCATE, account->ACCOUNTTYPE != Model_Account::all_type()[Model_Account::SHARES]);
            menu.AppendSeparator();
            AppendImportMenu(menu);
            PopupMenu(&menu, pt);
        }
        break;
    }
    case mmTreeItemData::MENU_ACCOUNT:
    case mmTreeItemData::MENU_FAVORITES:
    case mmTreeItemData::ALL_TRANSACTIONS:
    {
        menu.Append(MENU_TREEPOPUP_ACCOUNT_NEW, _("&New Account..."));
        menu.Append(MENU_TREEPOPUP_ACCOUNT_EDIT, _("&Edit Account..."));
        menu.Append(MENU_TREEPOPUP_ACCOUNT_LIST, _("Account &List"));
        menu.AppendSeparator();
        menu.Append(MENU_TREEPOPUP_ACCOUNT_DELETE, _("&Delete Account..."));
        menu.AppendSeparator();

        AppendImportMenu(menu);

        menu.AppendSeparator();
        wxMenu* viewAccounts(new wxMenu);
        viewAccounts->AppendRadioItem(MENU_TREEPOPUP_ACCOUNT_VIEWALL, _("&All"))->Check(m_temp_view == VIEW_ACCOUNTS_ALL_STR);
        viewAccounts->AppendRadioItem(MENU_TREEPOPUP_ACCOUNT_VIEWFAVORITE, _("&Favorites"))->Check(m_temp_view == VIEW_ACCOUNTS_FAVORITES_STR);
        viewAccounts->AppendRadioItem(MENU_TREEPOPUP_ACCOUNT_VIEWOPEN, _("&Open"))->Check(m_temp_view == VIEW_ACCOUNTS_OPEN_STR);
        viewAccounts->AppendRadioItem(MENU_TREEPOPUP_ACCOUNT_VIEWCLOSED, _("&Closed"))->Check(m_temp_view == VIEW_ACCOUNTS_CLOSED_STR);
        menu.AppendSubMenu(viewAccounts, _("Accounts &Visible"));

        PopupMenu(&menu, pt);
        break;
    }
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnViewAccountsTemporaryChange(wxCommandEvent& e)
{
    int evt_id = e.GetId();
    //Get current settings for view accounts
    const wxString vAccts = Model_Setting::instance().GetViewAccounts();
    if (m_temp_view.empty())
        m_temp_view = vAccts;

    //Set view ALL & Refresh Navigation Panel
    switch (evt_id)
    {
    case MENU_TREEPOPUP_ACCOUNT_VIEWALL: m_temp_view = VIEW_ACCOUNTS_ALL_STR; break;
    case MENU_TREEPOPUP_ACCOUNT_VIEWFAVORITE: m_temp_view = VIEW_ACCOUNTS_FAVORITES_STR; break;
    case MENU_TREEPOPUP_ACCOUNT_VIEWOPEN: m_temp_view = VIEW_ACCOUNTS_OPEN_STR; break;
    case MENU_TREEPOPUP_ACCOUNT_VIEWCLOSED: m_temp_view = VIEW_ACCOUNTS_CLOSED_STR; break;
    }
    Model_Setting::instance().SetViewAccounts(m_temp_view);
    RefreshNavigationTree();

    //Restore settings
    Model_Setting::instance().SetViewAccounts(vAccts);
}

//----------------------------------------------------------------------------

void mmGUIFrame::createMenu()
{
    wxMenu* menu_file = new wxMenu;

    wxMenuItem* menuItemNew = new wxMenuItem(menu_file, MENU_NEW, _("&New Database..."), _("New Database"));
    wxMenuItem* menuItemOpen = new wxMenuItem(menu_file, MENU_OPEN, _("&Open Database...\tCtrl-O"), _("Open Database"));
    wxMenuItem* menuItemSaveAs = new wxMenuItem(menu_file, MENU_SAVE_AS, _("Save Database &As..."), _("Save Database As"));
    menu_file->Append(menuItemNew);
    menu_file->Append(menuItemOpen);
    menu_file->Append(menuItemSaveAs);
    menu_file->AppendSeparator();

    m_menuRecentFiles = new wxMenu;
    menu_file->Append(MENU_RECENT_FILES, _("&Recent Files"), m_menuRecentFiles);
    wxMenuItem* menuClearRecentFiles = new wxMenuItem(menu_file, MENU_RECENT_FILES_CLEAR, _("&Clear Recent Files"));
    menu_file->Append(menuClearRecentFiles);
    menu_file->AppendSeparator();

    wxMenu* importMenu = new wxMenu;
    menu_file->Append(MENU_IMPORT, _("&Import from"), importMenu);
    importMenu->Append(MENU_IMPORT_UNIVCSV, _("&CSV File..."), _("Import from CSV file"));
    importMenu->Append(MENU_IMPORT_XML, _("&XML File..."), _("Import from XML file (Excel format)"));
    importMenu->AppendSeparator();
    importMenu->Append(MENU_IMPORT_QIF, _("&QIF File..."), _("Import from QIF file"));
    importMenu->AppendSeparator();
    importMenu->Append(MENU_IMPORT_WEBAPP, _("&WebApp..."), _("Import from WebApp"));

    wxMenu* exportMenu = new wxMenu;
    menu_file->Append(MENU_EXPORT, _("&Export as"), exportMenu);
    exportMenu->Append(MENU_EXPORT_CSV, _("&CSV File..."), _("Export as CSV file"));
    exportMenu->Append(MENU_EXPORT_XML, _("&XML File..."), _("Export as XML file"));
    exportMenu->AppendSeparator();
    exportMenu->Append(MENU_EXPORT_MMEX, _("&MMEX CSV File..."), _("Export as fixed CSV file"));
    exportMenu->Append(MENU_EXPORT_JSON, _("&JSON File..."), _("Export as JSON file"));
    exportMenu->Append(MENU_EXPORT_QIF, _("&QIF File..."), _("Export as QIF file"));
    exportMenu->AppendSeparator();
    exportMenu->Append(MENU_EXPORT_HTML, _("&HTML File..."), _("Export as HTML file"));

    menu_file->AppendSeparator();

    wxMenuItem* menuItemPrint = new wxMenuItem(menu_file, wxID_PRINT,
        _("&Print..."), _("Print current view"));
    menu_file->Append(menuItemPrint);

    menu_file->AppendSeparator();

    wxMenuItem* menuItemQuit = new wxMenuItem(menu_file, wxID_EXIT,
        _("E&xit\tCtrl-Q"), _("Quit this program"));
    menu_file->Append(menuItemQuit);

    // Create the required menu items
    wxMenu* menuView = new wxMenu;
    wxMenuItem* menuItemToolbar = new wxMenuItem(menuView, MENU_VIEW_TOOLBAR,
        _("&Toolbar"), _("Show/Hide the toolbar"), wxITEM_CHECK);
    wxMenuItem* menuItemLinks = new wxMenuItem(menuView, MENU_VIEW_LINKS,
        _("&Navigator\tF5"), _("Show/Hide Navigator"), wxITEM_CHECK);
    wxMenuItem* menuItemHideShareAccounts = new wxMenuItem(menuView, MENU_VIEW_HIDE_SHARE_ACCOUNTS,
        _("&Display Share Accounts"), _("Show/Hide Share Accounts in Navigator"), wxITEM_CHECK);
    wxMenuItem* menuItemHideDeletedTransactions = new wxMenuItem(menuView, MENU_VIEW_HIDE_DELETED_TRANSACTIONS,
        _("&Display Deleted Transactions"), _("Show/Hide Deleted Transactions in Navigator"), wxITEM_CHECK);
    wxMenuItem* menuItemBudgetFinancialYears = new wxMenuItem(menuView, MENU_VIEW_BUDGET_FINANCIAL_YEARS,
        _("Budgets: As Financial &Years"), _("Display Budgets in Financial Year Format"), wxITEM_CHECK);
    wxMenuItem* menuItemBudgetTransferTotal = new wxMenuItem(menuView, MENU_VIEW_BUDGET_TRANSFER_TOTAL,
        _("Budgets: &Include Transfers in Totals"), _("Include the transfer transactions in the Budget Totals"), wxITEM_CHECK);
    wxMenuItem* menuItemBudgetCategorySummary = new wxMenuItem(menuView, MENU_VIEW_BUDGET_CATEGORY_SUMMARY,
        _("Budget Category Report: with &Summaries"), _("Include the category summaries in the Budget Category Summary"), wxITEM_CHECK);
    wxMenuItem* menuItemIgnoreFutureTransactions = new wxMenuItem(menuView, MENU_VIEW_IGNORE_FUTURE_TRANSACTIONS,
        _("Ignore F&uture Transactions"), _("Ignore Future transactions"), wxITEM_CHECK);
    wxMenuItem* menuItemShowToolTips = new wxMenuItem(menuView, MENU_VIEW_SHOW_TOOLTIPS,
        _("Show T&ooltips"), _("Show Tooltips"), wxITEM_CHECK);
    wxMenuItem* menuItemShowMoneyTips = new wxMenuItem(menuView, MENU_VIEW_SHOW_MONEYTIPS,
        _("Show &Money Tips"), _("Show Money Tips"), wxITEM_CHECK);

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
    wxMenuItem* menuItemToggleFullscreen = new wxMenuItem(menuView, MENU_VIEW_TOGGLE_FULLSCREEN
        , _("&Full Screen\tF11")
        , _("Toggle full screen")
        , wxITEM_CHECK);
    menuView->Append(menuItemToggleFullscreen);
#endif
wxMenuItem* menuItemResetView = new wxMenuItem(menuView, MENU_VIEW_RESET
        , _("&Reset View")
        , _("Reset view and dock tools"));
    menuView->Append(menuItemResetView);   

    menuView->AppendSeparator();
    wxMenuItem* menuItemLanguage = new wxMenuItem(menuView, MENU_LANG
        , _("User Interface &Language")
        , _("Change user interface language"));
    wxMenu* menuLang = new wxMenu;

    wxArrayString lang_files = wxTranslations::Get()->GetAvailableTranslations("mmex");
    std::map<wxString, std::pair<int, wxString>> langs;
    menuLang->AppendRadioItem(MENU_LANG + 1 + wxLANGUAGE_DEFAULT, _("System default"))
        ->Check(m_app->getGUILanguage() == wxLANGUAGE_DEFAULT);
    for (auto & file : lang_files)
    {
        const wxLanguageInfo* info = wxLocale::FindLanguageInfo(file);
        if (info)
            langs[wxGetTranslation(info->Description)] = std::make_pair(info->Language, info->CanonicalName);
    }
    langs[wxGetTranslation(wxLocale::GetLanguageName(wxLANGUAGE_ENGLISH_US))] = std::make_pair(wxLANGUAGE_ENGLISH_US, "en_US");
    for (auto const& lang : langs)
    {
        menuLang->AppendRadioItem(MENU_LANG + 1 + lang.second.first, lang.first, lang.second.second)
            ->Check(lang.second.first == m_app->getGUILanguage());
    }
    menuItemLanguage->SetSubMenu(menuLang);
    menuView->Append(menuItemLanguage);

    wxMenu* menuAccounts = new wxMenu;

    wxMenuItem* menuItemAcctList = new wxMenuItem(menuAccounts, MENU_HOMEPAGE
        , _("Dash&board"), _("Open Dashboard"));

    wxMenuItem* menuItemNewAcct = new wxMenuItem(menuAccounts, MENU_NEWACCT
        , _("&New Account..."), _("New Account"));

    wxMenuItem* menuItemAcctEdit = new wxMenuItem(menuAccounts, MENU_ACCTEDIT
        , _("&Edit Account..."), _("Edit Account"));

    wxMenuItem* menuItemReallocateAcct = new wxMenuItem(menuAccounts, MENU_ACCOUNT_REALLOCATE
        , _("&Change Account Type..."), _("Change the account type of an account"));

    wxMenuItem* menuItemAcctDelete = new wxMenuItem(menuAccounts, MENU_ACCTDELETE
        , _("&Delete Account..."), _("Delete Account from database"));

    menuAccounts->Append(menuItemAcctList);
    menuAccounts->AppendSeparator();
    menuAccounts->Append(menuItemNewAcct);
    menuAccounts->Append(menuItemAcctEdit);
    menuAccounts->Append(menuItemReallocateAcct);
    menuAccounts->Append(menuItemAcctDelete);

    // Tools Menu
    wxMenu* menuTools = new wxMenu;

    wxMenuItem* menuItemRates = new wxMenuItem(menuTools
        , MENU_RATES, _("Download &Rates"), _("Download Currency and Stock Rates"));
    menuTools->Append(menuItemRates);

    menuTools->AppendSeparator();

    wxMenuItem* menuItemPayee = new wxMenuItem(menuTools
        , MENU_ORGPAYEE, _("&Payee Manager..."), _("Payee Manager"));
    menuTools->Append(menuItemPayee);

    wxMenuItem* menuItemCateg = new wxMenuItem(menuTools
        , MENU_ORGCATEGS, _("&Category Manager..."), _("Category Manager"));
    menuTools->Append(menuItemCateg);

    wxMenuItem* menuItemTags = new wxMenuItem(menuTools
        , MENU_ORGTAGS, _("&Tag Manager..."), _("Tag Manager"));
    menuTools->Append(menuItemTags);

    wxMenuItem* menuItemCurrency = new wxMenuItem(menuTools, MENU_CURRENCY
        , _("Curre&ncy Manager..."), _("Currency Manager"));
    menuTools->Append(menuItemCurrency);

    wxMenuItem* menuItemCategoryRelocation = new wxMenuItem(menuTools
        , MENU_CATEGORY_RELOCATION, _("&Categories...")
        , _("Merge categories"));
    wxMenuItem* menuItemPayeeRelocation = new wxMenuItem(menuTools
        , MENU_PAYEE_RELOCATION, _("&Payees...")
        , _("Merge payees"));
    wxMenuItem* menuItemTagRelocation = new wxMenuItem(menuTools
        , MENU_TAG_RELOCATION, _("&Tags...")
        , _("Merge tags"));
    wxMenuItem* menuItemRelocation = new wxMenuItem(menuTools
        , MENU_RELOCATION, _("&Merge")
        , _("Merge categories, payees, and tags"));
    wxMenu* menuRelocation = new wxMenu;
    menuRelocation->Append(menuItemPayeeRelocation);
    menuRelocation->Append(menuItemCategoryRelocation);
    menuRelocation->Append(menuItemTagRelocation);
    menuItemRelocation->SetSubMenu(menuRelocation);
    menuTools->Append(menuItemRelocation);

    menuTools->AppendSeparator();

    wxMenuItem* menuItemBudgeting = new wxMenuItem(menuTools, MENU_BUDGETSETUPDIALOG
        , _("&Budget Planner..."), _("Budget Planner"));
    menuTools->Append(menuItemBudgeting);

    wxMenuItem* menuItemBillsDeposits = new wxMenuItem(menuTools, MENU_BILLSDEPOSITS
        , _("&Scheduled Transactions"), _("Bills and Deposits"));
    menuTools->Append(menuItemBillsDeposits);

    wxMenuItem* menuItemAssets = new wxMenuItem(menuTools, MENU_ASSETS
        , _("&Assets"), _("Assets"));
    menuTools->Append(menuItemAssets);

    menuTools->AppendSeparator();

    wxMenuItem* menuItemThemes = new wxMenuItem(menuTools, MENU_THEME_MANAGER
        , _("T&heme Manager..."), _("Theme Manager"));
    menuTools->Append(menuItemThemes);

    wxMenuItem* menuItemColumnOrder = new wxMenuItem(menuTools, MENU_COLUMN_ORDER
        , _("C&olumn Order Manager..."), _("Column Order Manager"));
    menuTools->Append(menuItemColumnOrder);

    menuTools->AppendSeparator();

    wxMenuItem* menuItemTransactions = new wxMenuItem(menuTools, MENU_TRANSACTIONREPORT
        , _("Tra&nsaction Report..."), _("Transaction Report"));
    menuTools->Append(menuItemTransactions);

    menuTools->AppendSeparator();

    wxMenuItem* menuItemGRM = new wxMenuItem(menuTools, wxID_VIEW_LIST
        , _("&General Report Manager..."), _("General Report Manager"));
    menuTools->Append(menuItemGRM);

    wxMenuItem* menuItemCF = new wxMenuItem(menuTools, wxID_BROWSE
        , _("C&ustom Field Manager..."), _("Custom Field Manager"));
    menuTools->Append(menuItemCF);

    menuTools->AppendSeparator();

    wxMenuItem* menuItemWA = new wxMenuItem(menuTools, MENU_REFRESH_WEBAPP
        , _("Refresh &WebApp..."), _("Refresh WebApp"));
    menuTools->Append(menuItemWA);
    menuTools->AppendSeparator();

    wxMenuItem* menuItemOptions = new wxMenuItem(menuTools, wxID_PREFERENCES
        , _("&Settings...\tAlt-F12"), _("Settings"));
    menuTools->Append(menuItemOptions);

    menuTools->AppendSeparator();

    wxMenu* menuDatabase = new wxMenu;
    wxMenuItem* menuItemConvertDB = new wxMenuItem(menuTools, MENU_CONVERT_ENC_DB
        , _("&Decrypt Encrypted Database...")
        , _("Convert encrypted database to unencrypted database"));
    wxMenuItem* menuItemChangeEncryptPassword = new wxMenuItem(menuTools, MENU_CHANGE_ENCRYPT_PASSWORD
        , _("Change Encrypted &Password...")
        , _("Change the password of an encrypted database"));
    wxMenuItem* menuItemVacuumDB = new wxMenuItem(menuTools, MENU_DB_VACUUM
        , _("&Optimize Database...")
        , _("Optimize database for space and performance"));
    wxMenuItem* menuItemCheckDB = new wxMenuItem(menuTools, MENU_DB_DEBUG
        , _("Database Check and De&bug...")
        , _("Generate database report or fix errors"));
    menuDatabase->Append(menuItemConvertDB);
    menuDatabase->Append(menuItemChangeEncryptPassword);
    menuDatabase->Append(menuItemVacuumDB);
    menuDatabase->Append(menuItemCheckDB);
    menuTools->AppendSubMenu(menuDatabase, _("&Database")
        , _("Database management"));
    menuItemChangeEncryptPassword->Enable(false);

    // Help Menu
    wxMenu* menuHelp = new wxMenu;

    wxMenuItem* menuItemHelp = new wxMenuItem(menuTools, wxID_HELP,
        _("&Help\tF1"), _("Show Help"));
    menuHelp->Append(menuItemHelp);

    //Community Submenu
    wxMenuItem* menuItemWebsite = new wxMenuItem(menuHelp, MENU_WEBSITE
        , _("&Website")
        , _("Visit MMEX website for the latest news and updates"));
    wxMenuItem* menuItemFacebook = new wxMenuItem(menuHelp, MENU_FACEBOOK
        , _("Face&book"), _("Visit MMEX Facebook page"));
    wxMenuItem* menuItemCrowdin = new wxMenuItem(menuHelp, MENU_CROWDIN
        , _("&Crowdin"), _("Help translate MMEX on Crowdin"));
    wxMenuItem* menuItemTwitter = new wxMenuItem(menuHelp, MENU_TWITTER
        , _("&X"), _("Follow MMEX on X"));
    wxMenuItem* menuItemYouTube = new wxMenuItem(menuHelp, MENU_YOUTUBE
        , _("&YouTube"), _("Watch MMEX videos on YouTube"));
    wxMenuItem* menuItemSlack = new wxMenuItem(menuHelp, MENU_SLACK
        , _("&Slack"), _("Communicate with the MMEX team online"));
    wxMenuItem* menuItemGitHub = new wxMenuItem(menuHelp, MENU_GITHUB
        , _("&GitHub"), _("Access open source code repository and track reported bug statuses"));
    wxMenuItem* menuItemWiki = new wxMenuItem(menuHelp, MENU_WIKI
        , _("W&iki"), _("Read and update MMEX wiki"));
    wxMenuItem* menuItemReportIssues = new wxMenuItem(menuHelp, MENU_REPORTISSUES
        , _("&Forum")
        , _("Visit MMEX forum to read and post comments and for support"));
    wxMenuItem* menuItemGooglePlay = new wxMenuItem(menuHelp, MENU_GOOGLEPLAY
        , _("MMEX for &Android")
        , _("Install MMEX for Android based smartphones and tablets"));
    wxMenuItem* menuItemNotify = new wxMenuItem(menuHelp, MENU_ANNOUNCEMENTMAILING
        , _("&Newsletter")
        , _("Visit the MMEX blog to view announcements"));
    wxMenuItem* menuItemRSS = new wxMenuItem(menuHelp, MENU_RSS
        , _("&RSS Feed"), _("Connect RSS web feed to news aggregator"));
    wxMenuItem* menuItemDonate = new wxMenuItem(menuHelp, MENU_DONATE
        , _("Make a &Donation")
        , _("Donate to the MMEX team to support infrastructure and development"));
    wxMenuItem* menuItemBuyCoffee = new wxMenuItem(menuHelp, MENU_BUY_COFFEE
        , _("&Buy us a Coffee")
        , _("Buy a key developer a coffee"));

    wxMenuItem* menuItemCommunity = new wxMenuItem(menuHelp, MENU_COMMUNITY
        , _("&Community")
        , _("Stay in touch with MMEX community"));

    wxMenu* menuCommunity = new wxMenu;
    menuCommunity->Append(menuItemWebsite);
    menuCommunity->Append(menuItemGitHub);
    menuCommunity->Append(menuItemCrowdin);
    menuCommunity->Append(menuItemWiki);
    menuCommunity->Append(menuItemNotify);
    menuCommunity->Append(menuItemReportIssues);
    menuCommunity->AppendSeparator();
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
        , _("Report a &Bug")
        , _("Report an error in application to the developers"));
    menuHelp->Append(menuItemReportBug);

    wxMenuItem* menuItemDiagnostics = new wxMenuItem(menuTools, MENU_DIAGNOSTICS
        , _("View &Diagnostics")
        , _("Help provide information to the developers"));
    menuHelp->Append(menuItemDiagnostics);

    wxMenuItem* menuItemAppStart = new wxMenuItem(menuTools, MENU_SHOW_APPSTART
        , _("&Show App Start Dialog"), _("App Start Dialog"));
    menuHelp->Append(menuItemAppStart);

    wxMenuItem* menuItemCheck = new wxMenuItem(menuTools, MENU_CHECKUPDATE
        , _("Check for &Updates"), _("Check For Updates"));
    menuHelp->Append(menuItemCheck);

    wxMenuItem* menuItemAbout = new wxMenuItem(menuTools, wxID_ABOUT
        , _("&About"), _("About dialog"));
    menuHelp->Append(menuItemAbout);

    menuBar_ = new wxMenuBar;
    menuBar_->Append(menu_file, _("&File"));
    menuBar_->Append(menuAccounts, _("&Accounts"));
    menuBar_->Append(menuTools, _("&Tools"));
    menuBar_->Append(menuView, _("&View"));
    menuBar_->Append(menuHelp, _("&Help"));
    SetMenuBar(menuBar_);

    menuBar_->Check(MENU_VIEW_HIDE_SHARE_ACCOUNTS, !Option::instance().HideShareAccounts());
    menuBar_->Check(MENU_VIEW_HIDE_DELETED_TRANSACTIONS, !Option::instance().HideDeletedTransactions());
    menuBar_->Check(MENU_VIEW_BUDGET_FINANCIAL_YEARS, Option::instance().BudgetFinancialYears());
    menuBar_->Check(MENU_VIEW_BUDGET_TRANSFER_TOTAL, Option::instance().BudgetIncludeTransfers());
    menuBar_->Check(MENU_VIEW_BUDGET_CATEGORY_SUMMARY, Option::instance().BudgetReportWithSummaries());
    menuBar_->Check(MENU_VIEW_IGNORE_FUTURE_TRANSACTIONS, Option::instance().getIgnoreFutureTransactions());
    menuBar_->Check(MENU_VIEW_SHOW_TOOLTIPS, Option::instance().getShowToolTips());
    menuBar_->Check(MENU_VIEW_SHOW_MONEYTIPS, Option::instance().getShowMoneyTips());
}
//----------------------------------------------------------------------------

void mmGUIFrame::CreateToolBar()
{
    const int toolbar_icon_size = Option::instance().getToolbarIconSize();
    const long style = wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_HORIZONTAL | wxAUI_TB_PLAIN_BACKGROUND;

    toolBar_ = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, style);
    toolBar_->SetToolBorderPadding(1);
    mmThemeMetaColour(toolBar_, meta::COLOR_LISTPANEL);
    //toolBar_->SetToolBitmapSize(wxSize(toolbar_icon_size, toolbar_icon_size));  // adjust tool size to match the icon size being used

    toolBar_->AddTool(MENU_NEW, _("New"), mmBitmapBundle(png::NEW_DB, toolbar_icon_size), _("New Database"));
    toolBar_->AddTool(MENU_OPEN, _("Open"), mmBitmapBundle(png::OPEN, toolbar_icon_size), _("Open Database"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(MENU_NEWACCT, _("New Account"), mmBitmapBundle(png::NEW_ACC, toolbar_icon_size), _("New Account"));
    toolBar_->AddTool(MENU_HOMEPAGE, _("Dashboard"), mmBitmapBundle(png::HOME, toolbar_icon_size), _("Open Dashboard"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(wxID_NEW, _("New"), mmBitmapBundle(png::NEW_TRX, toolbar_icon_size), _("New Transaction"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(MENU_ORGPAYEE, _("Payee Manager"), mmBitmapBundle(png::PAYEE, toolbar_icon_size), _("Payee Manager"));
    toolBar_->AddTool(MENU_ORGCATEGS, _("Category Manager"), mmBitmapBundle(png::CATEGORY, toolbar_icon_size), _("Category Manager"));
    toolBar_->AddTool(MENU_ORGTAGS, _("Tag Manager"), mmBitmapBundle(png::TAG, toolbar_icon_size), _("Tag Manager"));
    toolBar_->AddTool(MENU_CURRENCY, _("Currency Manager"), mmBitmapBundle(png::CURR, toolbar_icon_size), _("Currency Manager"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(MENU_TRANSACTIONREPORT, _("Transaction Report"), mmBitmapBundle(png::FILTER, toolbar_icon_size), _("Transaction Report"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(wxID_VIEW_LIST, _("General Report Manager"), mmBitmapBundle(png::GRM, toolbar_icon_size), _("General Report Manager"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(wxID_PREFERENCES, _("&Settings"), mmBitmapBundle(png::OPTIONS, toolbar_icon_size), _("Settings"));
    toolBar_->AddSeparator();

    wxString news_array;
    for (const auto& entry : websiteNewsArray_) {
        news_array += entry.Title + "\n";
    }
    if (news_array.empty()) {
        news_array = _("News");
    }
    const auto news_ico = (websiteNewsArray_.size() > 0)
        ? mmBitmapBundle(png::NEW_NEWS, toolbar_icon_size)
        : mmBitmapBundle(png::NEWS, toolbar_icon_size);
    toolBar_->AddTool(MENU_ANNOUNCEMENTMAILING, _("News"), news_ico, news_array);

    toolBar_->AddTool(MENU_RATES, _("Download Rates"), mmBitmapBundle(png::CURRATES, toolbar_icon_size), _("Download currency and stock rates"));

    toolBar_->AddSeparator();
    toolBar_->AddTool(MENU_VIEW_TOGGLE_FULLSCREEN, _("Full Screen\tF11"), mmBitmapBundle(png::FULLSCREEN, toolbar_icon_size), _("Toggle full screen"));

    toolBar_->AddSeparator();
    toolBar_->AddTool(wxID_PRINT, _("&Print"), mmBitmapBundle(png::PRINT, toolbar_icon_size), _("Print"));

    toolBar_->AddSeparator();
    toolBar_->AddTool(wxID_ABOUT, _("&About"), mmBitmapBundle(png::ABOUT, toolbar_icon_size), _("About"));
    toolBar_->AddTool(wxID_HELP, _("&Help\tF1"), mmBitmapBundle(png::HELP, toolbar_icon_size), _("Help"));

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
    if (m_db)
    {
        ShutdownDatabase();
        // Backup the database according to user requirements
        if (Option::instance().DatabaseUpdated() &&
            Model_Setting::instance().GetBoolSetting("BACKUPDB_UPDATE", false))
        {
            dbUpgrade::BackupDB(m_filename, dbUpgrade::BACKUPTYPE::CLOSE, Model_Setting::instance().GetIntSetting("MAX_BACKUP_FILES", 4));
            Option::instance().DatabaseUpdated(false);
        }
    }

    wxFileName checkExt(fileName);
    wxString password;
    bool passwordCheckPassed = true;
    if (checkExt.GetExt().Lower() == "emb" && wxFileName::FileExists(fileName))
    {
        wxString password_message = wxString::Format(_("Please enter password for Database\n\n%s"), fileName);
        password = !pwd.empty() ? pwd : wxGetPasswordFromUser(password_message, _("MMEX: Encrypted Database"));
        if (password.IsEmpty())
            passwordCheckPassed = false;
    }

    const wxString dialogErrorMessageHeading = _("Opening MMEX Database - Error");

    // Existing Database
    if (!openingNew
        && !fileName.IsEmpty()
        && wxFileName::FileExists(fileName)
        && passwordCheckPassed)
    {
        /* Do a backup before opening */
        if (Model_Setting::instance().GetBoolSetting("BACKUPDB", false))
        {
            dbUpgrade::BackupDB(fileName, dbUpgrade::BACKUPTYPE::START, Model_Setting::instance().GetIntSetting("MAX_BACKUP_FILES", 4));
        }

        m_db = mmDBWrapper::Open(fileName, password);
        // if the database pointer has been reset, the password is possibly incorrect
        if (!m_db) return false;

        m_commit_callback_hook = new CommitCallbackHook();
        m_db->SetCommitHook(m_commit_callback_hook.get());
        m_update_callback_hook = new UpdateCallbackHook();
        m_db->SetUpdateHook(m_update_callback_hook.get());

        //Check if DB upgrade needed
        if (dbUpgrade::isUpgradeDBrequired(m_db.get()))
        {
            // close & reopen database in debug mode for upgrade (bypassing SQLITE_CorruptRdOnly flag)
            ShutdownDatabase();
            m_db = mmDBWrapper::Open(fileName, password, true);
            //DB backup is handled inside UpgradeDB
            if (!dbUpgrade::UpgradeDB(m_db.get(), fileName))
            {
                int response = wxMessageBox(_("Have MMEX support provided a debug/patch file?"), _("MMEX upgrade"), wxYES_NO);
                if (response == wxYES)
                {
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

        wxString UID = Model_Infotable::instance().GetStringInfo("UID", wxEmptyString);
        if (UID.IsEmpty())
        {
            UID = Model_Setting::instance().GetStringSetting("UUID", wxEmptyString);
            Model_Infotable::instance().Set("UID", UID);
        }
        Model_Setting::instance().Set("UID", UID);

        // ** OBSOLETE **
        // Mantained only for really old compatibility reason and replaced by dbupgrade.cpp
        if (!Model_Infotable::instance().checkDBVersion())
        {
            const wxString note = wxString::Format("%s - %s", mmex::getProgramName(), _("No File opened"));
            this->SetTitle(note);
            wxMessageBox(_("Sorry. The Database version is too old or Database password is incorrect")
                , dialogErrorMessageHeading
                , wxOK | wxICON_EXCLAMATION);

            ShutdownDatabase();
            return false;
        }

        m_password = password;

        // WE CAN EVENTUALLY DELETE THIS CODE
        // Get Hidden Categories id from stored INFO string and move to Category/Subcategory tables
        wxString sSettings = Model_Infotable::instance().GetStringInfo("HIDDEN_CATEGS_ID", "");
        if (!sSettings.empty())
        {
            wxStringTokenizer token(sSettings, ";");
            Model_Category::instance().Savepoint();
            while (token.HasMoreTokens())
            {
                wxString catData = token.GetNextToken();
                int catID = 0;
                int subCatID = 0;
                if (2 == sscanf(catData.mb_str(),"*%d:%d*", &catID, &subCatID))
                {
                    if (subCatID == -1)
                    {
                        Model_Category::Data* cat = Model_Category::instance().get(catID);
                        if (cat && cat->CATEGID != -1)
                        {
                            cat->ACTIVE = 0;
                            Model_Category::instance().save(cat);
                        }
                    }
                    else
                    {
                        Model_Category::Data* subcat = Model_Category::instance().get(subCatID);
                        if (subcat && subcat->CATEGID != -1)
                        {
                            subcat->ACTIVE = 0;
                            Model_Category::instance().save(subcat);
                        }
                    }
                }
            }
            Model_Category::instance().ReleaseSavepoint();
            Model_Infotable::instance().Set("HIDDEN_CATEGS_ID", "");
        }
    }
    else if (openingNew) // New Database
    {
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
        if (!wizard->RunIt(true))
        {
            ShutdownDatabase();
            wxRemoveFile(fileName);
            return false;
        }

        wxButton* next = static_cast<wxButton*>(wizard->FindWindow(wxID_FORWARD)); //FIXME: 
        if (next) next->SetLabel(wxGetTranslation(wxString::FromUTF8(wxTRANSLATE("&Next →"))));

        SetDataBaseParameters(fileName);
        /* Jump to new account creation screen */
        wxCommandEvent evt;
        OnNewAccount(evt);
        return true;
    }
    else // open of existing database failed
    {
        const wxString note = wxString::Format("%s - %s", mmex::getProgramName(), _("No File opened"));
        this->SetTitle(note);

        wxString msgStr = _("Cannot locate previously opened database.\n");
        if (!passwordCheckPassed)
            msgStr = _("Password not entered for encrypted Database.\n");

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
        title = wxString::Format("%s [%s]", title, _("portable mode"));

    SetTitle(title);

    if (m_db)
    {
        m_filename = fileName;
        /* Set InfoTable Options into memory */
        Option::instance().LoadOptions();
    }
    else
    {
        m_filename.Clear();
        m_password.Clear();
    }
}
//----------------------------------------------------------------------------

bool mmGUIFrame::openFile(const wxString& fileName, bool openingNew, const wxString &password)
{ 
    menuBar_->FindItem(MENU_CHANGE_ENCRYPT_PASSWORD)->Enable(false);
    if (createDataStore(fileName, password, openingNew))
    {
        m_recentFiles->AddFileToHistory(fileName);
        menuEnableItems(true);
        menuPrintingEnable(false);

        if (m_db->IsEncrypted())
        {
            menuBar_->FindItem(MENU_CHANGE_ENCRYPT_PASSWORD)->Enable(true);
        }

        if (!m_app->GetSilentParam()) {
            db_lockInPlace = Model_Infotable::instance().GetBoolInfo("ISUSED", false);
            if (db_lockInPlace) {
                int response = wxMessageBox(_(
                    "The database you are trying to open has been marked as opened by another instance of MMEX.\n"
                    "To avoid data loss or conflict, it's strongly recommended that you close all other applications that may be using the database.\n\n"
                    "If nothing else is running, it's possible that the database was left open as a result of a crash during previous usage of MMEX.\n\n"
                    "Would you like to continue to open this database?")
                    , _("MMEX Instance Check"), wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
                if (response == wxNO)
                    return false;
            }
        }

        Model_Infotable::instance().Set("ISUSED", true);
        db_lockInPlace = false;
        autoRepeatTransactionsTimer_.Start(REPEAT_TRANS_DELAY_TIME, wxTIMER_ONE_SHOT);
    }
    else return false;

    return true;
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnNew(wxCommandEvent& /*event*/)
{
    autoRepeatTransactionsTimer_.Stop();
    wxFileDialog dlg(this,
        _("Choose database file to create"),
        wxEmptyString,
        wxEmptyString,
        _("MMEX Database")+" (*.mmb)|*.mmb",
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT
    );

    if (dlg.ShowModal() != wxID_OK)
        return;

    wxString fileName = dlg.GetPath();

    if (!fileName.EndsWith(".mmb"))
        fileName += ".mmb";

    SetDatabaseFile(fileName, true);
    Model_Setting::instance().Set("LASTFILENAME", fileName);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnOpen(wxCommandEvent& /*event*/)
{
    autoRepeatTransactionsTimer_.Stop();
    wxString fileName = wxFileSelector(_("Choose database file to open")
        , wxEmptyString, wxEmptyString, wxEmptyString
        , _("MMEX Database")+" (*.mmb)|*.mmb|"+_("Encrypted MMEX Database")+" (*.emb)|*.emb"
        , wxFD_FILE_MUST_EXIST | wxFD_OPEN
        , this
    );

    if (!fileName.empty())
    {
        SetDatabaseFile(fileName);
        saveSettings();
        if (m_db)
        {
            autocleanDeletedTransactions();
            if (!Model_Stock::instance().all().empty() && Model_Setting::instance().GetBoolSetting("REFRESH_STOCK_QUOTES_ON_OPEN", false))
            {
                wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_RATES);
                this->GetEventHandler()->AddPendingEvent(evt);
            }
        }
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnConvertEncryptedDB(wxCommandEvent& /*event*/)
{
    wxString encFileName = wxFileSelector(_("Choose Encrypted database file to open")
        , wxEmptyString, wxEmptyString, wxEmptyString
        , _("Encrypted MMEX Database")+" (*.emb)|*.emb"
        , wxFD_FILE_MUST_EXIST
        , this
    );

    if (encFileName.empty())
        return;

    wxString password = wxGetPasswordFromUser(_("Enter password for database"), _("MMEX: Encrypted Database"));
    if (password.empty())
        return;

    wxFileDialog dlg(this
        , _("Choose database file to Save As")
        , wxEmptyString
        , wxEmptyString
        , _("MMEX Database")+" (*.mmb)|*.mmb"
        , wxFD_SAVE | wxFD_OVERWRITE_PROMPT
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

    mmErrorDialogs::MessageError(this, _("Converted database!"), _("MMEX message"));
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnChangeEncryptPassword(wxCommandEvent& /*event*/)
{
    wxString password_change_heading = _("MMEX: Encryption Password Change");
    wxString password_message = wxString::Format(_("New password for database:")+"\n\n%s", m_filename);

    wxPasswordEntryDialog dlg(this, password_message, password_change_heading);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxString new_password = dlg.GetValue();
        if (new_password.IsEmpty())
        {
            wxMessageBox(_("New password must not be empty."), password_change_heading, wxOK | wxICON_WARNING);
        }
        else
        {
            wxPasswordEntryDialog confirm_dlg(this, _("Please confirm new password"), password_change_heading);
            if (confirm_dlg.ShowModal() == wxID_OK)
            {
                wxString confirm_password = confirm_dlg.GetValue();
                if (!confirm_password.IsEmpty() && (new_password == confirm_password))
                {
                    wxSQLite3CipherSQLCipher cipher;
                    cipher.InitializeVersionDefault(4);
                    cipher.SetLegacy(true);

                    m_db->ReKey(cipher, confirm_password);
                    wxMessageBox(_("Password change completed"), password_change_heading);
                }
                else
                {
                    wxMessageBox(_("Confirm password failed."), password_change_heading);
                }
            }
        }
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnVacuumDB(wxCommandEvent& /*event*/)
{
    wxMessageDialog msgDlg(this
        , wxString::Format("%s\n\n%s", _("Backup database before optimization."), _("Do you want to proceed?"))
        , _("DB Optimization"), wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
    if (msgDlg.ShowModal() == wxID_YES)
    {
        const wxString SizeBefore = wxFileName(m_filename).GetHumanReadableSize();
        m_db->Vacuum();
        const wxString SizeAfter = wxFileName(m_filename).GetHumanReadableSize();
        wxMessageBox(wxString::Format(
            _("Database Optimization Completed!\n\n"
                "Size before: %1$s\n"
                "Size after: %2$s\n"), SizeBefore, SizeAfter),
            _("DB Optimization"));
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnDebugDB(wxCommandEvent& /*event*/)
{
    wxASSERT(m_db);
    wxString resultMessage;
    wxSQLite3Statement stmt = m_db->PrepareStatement("PRAGMA integrity_check;");
    if (stmt.IsReadOnly())
    {
        try
        {
            wxSQLite3ResultSet rs = stmt.ExecuteQuery();
            int columnCount = rs.GetColumnCount();
            while (rs.NextRow())
            {
                wxString strRow = "";
                for (int i = 0; i < columnCount; ++i)
                    strRow << rs.GetAsString(i);
                resultMessage << strRow + wxTextFile::GetEOL();
            }
        }
        catch (const wxSQLite3Exception& e)
        {
            wxMessageBox(_("Query error, please contact MMEX support!") + "\n\n" + e.GetMessage(), _("MMEX debug error"), wxOK | wxICON_ERROR);
            return;
        }
    }

    if (!resultMessage.IsEmpty())
    {
        wxTextEntryDialog checkDlg(this, _("Result of database integrity check:"), _("Database Check"), resultMessage.Trim(), wxOK | wxTE_MULTILINE);
        checkDlg.SetIcon(mmex::getProgramIcon());
        wxTextCtrl* textCtrl = dynamic_cast<wxTextCtrl*>(checkDlg.FindWindow(3000));
        if (textCtrl)
        {
            textCtrl->SetEditable(false);
            textCtrl->SetInsertionPointEnd();
        }

        wxButton* ok = static_cast<wxButton*>(checkDlg.FindWindow(wxID_OK));
        if (ok)
            ok->SetLabel(wxGetTranslation(g_OkLabel));

        checkDlg.Centre();
        checkDlg.ShowModal();
    }

    wxMessageDialog msgDlg(this
        , wxString::Format("%s\n\n%s", _("Please use this function only if requested by MMEX support and you have been supplied with a .mmdbg debug file"), _("Do you want to proceed?"))
        , _("Database Debug"), wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);

    msgDlg.SetYesNoLabels(_("&Yes"), _("&No"));

    if (msgDlg.ShowModal() == wxID_YES)
    {
        dbUpgrade::SqlFileDebug(m_db.get());
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnSaveAs(wxCommandEvent& /*event*/)
{
    wxASSERT(m_db);

    if (m_filename.empty())
    {
        wxASSERT(false);
        return;
    }

    wxFileDialog dlg(this,
        _("Save database file as"),
        wxEmptyString,
        wxEmptyString,
        _("MMEX Database")+" (*.mmb)|*.mmb|"+_("Encrypted MMEX Database")+" (*.emb)|*.emb",
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

    if (newFileName == oldFileName) // on case-sensitive FS uses case-sensitive comparison
    {
        wxMessageDialog dlgMsg(this, _("Can't copy file to itself"), _("Save database file as"), wxOK | wxICON_WARNING);
        dlgMsg.ShowModal();
        return;
    }

    // prepare to copy
    wxString new_password;
    bool rekey = encrypt ^ m_db->IsEncrypted();

    if (encrypt)
    {
        if (rekey)
        {
            new_password = wxGetPasswordFromUser(_("Enter password for new database"), _("MMEX: Encrypted Database"));
            if (new_password.empty())
                return;
        }
        else
        {
            new_password = m_password;
        }
    }

    // copying db
    ShutdownDatabase(); // database must be closed before copying its file

    if (!wxCopyFile(oldFileName.GetFullPath(), newFileName.GetFullPath(), true))  // true -> overwrite if file exists
        return;

    if (rekey) // encrypt or reset encryption
    {
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
    int account_id = dlg.get_last_imported_acc();
    RefreshNavigationTree();
    if (account_id > 0)
    {
        setGotoAccountID(account_id, -1);
        Model_Account::Data* account = Model_Account::instance().get(account_id);
        setAccountNavTreeSection(account->ACCOUNTNAME);
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_GOTOACCOUNT);
        this->GetEventHandler()->AddPendingEvent(evt);
    }
    else
    {
        refreshPanelData();
    }

}
//----------------------------------------------------------------------------

void mmGUIFrame::OnImportUniversalCSV(wxCommandEvent& /*event*/)
{
    if (Model_Account::instance().all().empty())
    {
        wxMessageBox(_("No account available to import"), _("Universal CSV Import"), wxOK | wxICON_WARNING);
        return;
    }

    mmUnivCSVDialog univCSVDialog(this, mmUnivCSVDialog::DIALOG_TYPE_IMPORT_CSV, gotoAccountID_);
    univCSVDialog.ShowModal();
    if (univCSVDialog.isImportCompletedSuccessfully())
    {
        Model_Account::Data* account = Model_Account::instance().get(univCSVDialog.ImportedAccountID());
        createCheckingAccountPage(univCSVDialog.ImportedAccountID());
        if (account) setAccountNavTreeSection(account->ACCOUNTNAME);
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnImportXML(wxCommandEvent& /*event*/)
{
    if (Model_Account::instance().all().empty())
    {
        wxMessageBox(_("No account available to import"), _("Universal CSV Import"), wxOK | wxICON_WARNING);
        return;
    }

    mmUnivCSVDialog univCSVDialog(this, mmUnivCSVDialog::DIALOG_TYPE_IMPORT_XML, gotoAccountID_);
    univCSVDialog.ShowModal();
    if (univCSVDialog.isImportCompletedSuccessfully())
    {
        Model_Account::Data* account = Model_Account::instance().get(univCSVDialog.ImportedAccountID());
        createCheckingAccountPage(univCSVDialog.ImportedAccountID());
        if (account) setAccountNavTreeSection(account->ACCOUNTNAME);
    }
}

//----------------------------------------------------------------------------

void mmGUIFrame::OnImportWebApp(wxCommandEvent& /*event*/)
{
    mmWebAppDialog dlg(this, false);
    if (dlg.ShowModal() == wxID_HELP) {
        helpFileIndex_ = mmex::HTML_WEBAPP;
        createHelpPage(helpFileIndex_);
        setNavTreeSection(_("Help"));
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

    if (wizard->acctID_ != -1)
    {
        Model_Account::Data* account = Model_Account::instance().get(wizard->acctID_);
        mmNewAcctDialog dlg(account, this);
        dlg.ShowModal();
        if (account->ACCOUNTTYPE == Model_Account::all_type()[Model_Account::ASSET])
        {
            wxMessageBox(_(
                "Asset Accounts hold Asset transactions\n\n"
                "Asset transactions are created within the Assets View\n"
                "after the selection of the Asset within that view.\n\n"
                "Asset Accounts can also hold normal transactions to regular accounts."
            ), _("Asset Account Creation"));
        }

        if (account->ACCOUNTTYPE == Model_Account::all_type()[Model_Account::SHARES])
        {
            wxMessageBox(wxGetTranslation(wxString::FromUTF8(wxTRANSLATE(
                "Share Accounts hold Share transactions\n\n"
                "Share transactions are created within the Stock Portfolio View\n"
                "after the selection of the Company Stock within the associated view.\n\n"
                "These accounts only become visible after associating a Stock to the Share Account\n"
                "Or by using the Menu View → 'Display Share Accounts'\n"
                "Share Accounts can also hold normal transactions to regular account."
            ))), _("Share Account Creation"));
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
    int id = panelCurrent_->GetId();
    wxLogDebug("Panel ID: %d", id);

    switch (id)
    {
    case mmID_HOMEPAGE:
        createHomePage();
        break;
    case mmID_CHECKING:
        wxDynamicCast(panelCurrent_, mmCheckingPanel)->RefreshList();
        break;
    case mmID_ALLTRANSACTIONS:
        wxDynamicCast(panelCurrent_, mmCheckingPanel)->RefreshList();
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
    case mmID_DELETEDTRANSACTIONS:
        wxDynamicCast(panelCurrent_, mmCheckingPanel)->RefreshList();
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
    if (id.IsOk())
    {
        wxTreeEvent evt(wxEVT_TREE_SEL_CHANGED, m_nav_tree_ctrl, id);
        m_nav_tree_ctrl->GetEventHandler()->AddPendingEvent(evt);
    }
}

void mmGUIFrame::OnOrgCategories(wxCommandEvent& /*event*/)
{
    mmCategDialog dlg(this, false, -1);
    dlg.ShowModal();
    if (dlg.getRefreshRequested())
    {
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
    if (dlg.getRefreshRequested())
    {
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
    if (dlg.getRefreshRequested())
    {
        activeReport_ = false;
        refreshPanelData();
        RefreshNavigationTree();
    }
    
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnNewTransaction(wxCommandEvent& event)
{
    if (m_db)
    {
        if (Model_Account::instance().all_checking_account_names().empty()) return;
        mmTransDialog dlg(this, gotoAccountID_, 0, 0);

        int i = dlg.ShowModal();
        if (i != wxID_CANCEL)
        {
            gotoAccountID_ = dlg.GetAccountID();
            gotoTransID_ = dlg.GetTransactionID();
            Model_Account::Data * account = Model_Account::instance().get(gotoAccountID_);
            if (account)
            {
                createCheckingAccountPage(gotoAccountID_);
                setAccountNavTreeSection(account->ACCOUNTNAME);
            }

            if (i == wxID_NEW)
                OnNewTransaction(event);
        }
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnTransactionReport(wxCommandEvent& WXUNUSED(event))
{
    if (!m_db) return;
    if (Model_Account::instance().all().empty()) return;

    const auto filter_settings = Model_Infotable::instance().GetArrayStringSetting("TRANSACTIONS_FILTER");

    wxSharedPtr<mmFilterTransactionsDialog> dlg(new mmFilterTransactionsDialog(this, -1, true));
    bool is_ok = (dlg->ShowModal() == wxID_OK);
    if (filter_settings != Model_Infotable::instance().GetArrayStringSetting("TRANSACTIONS_FILTER")) {
        DoRecreateNavTreeControl();
    }
    if (is_ok) {
        mmReportTransactions* rs = new mmReportTransactions(dlg);
        setNavTreeSection(_("Transaction Report"));
        createReportsPage(rs, true);
    }
}

void mmGUIFrame::OnBudgetSetupDialog(wxCommandEvent& /*event*/)
{
    if (m_db)
    {
        const auto a = Model_Budgetyear::instance().all(Model_Budgetyear::COL_BUDGETYEARNAME).to_json();
        mmBudgetYearDialog(this).ShowModal();
        const auto b = Model_Budgetyear::instance().all(Model_Budgetyear::COL_BUDGETYEARNAME).to_json();
        if (a != b) {
            DoRecreateNavTreeControl(true);
        }
        setNavTreeSection(_("Budget Planner"));
    }
}

void mmGUIFrame::OnGeneralReportManager(wxCommandEvent& /*event*/)
{
    if (!m_db) return;

    mmGeneralReportManager dlg(this, m_db.get());
    dlg.ShowModal();
    RefreshNavigationTree();
}

void mmGUIFrame::OnOptions(wxCommandEvent& /*event*/)
{
    if (!m_db.get()) return;

    mmOptionsDialog systemOptions(this, this->m_app);
    if (systemOptions.ShowModal() == wxID_OK)
    {
        //set the View Menu Option items the same as the options saved.
        menuBar_->FindItem(MENU_VIEW_BUDGET_FINANCIAL_YEARS)->Check(Option::instance().BudgetFinancialYears());
        menuBar_->FindItem(MENU_VIEW_BUDGET_TRANSFER_TOTAL)->Check(Option::instance().BudgetIncludeTransfers());
        menuBar_->FindItem(MENU_VIEW_BUDGET_CATEGORY_SUMMARY)->Check(Option::instance().BudgetReportWithSummaries());
        menuBar_->FindItem(MENU_VIEW_IGNORE_FUTURE_TRANSACTIONS)->Check(Option::instance().getIgnoreFutureTransactions());
        menuBar_->FindItem(MENU_VIEW_SHOW_TOOLTIPS)->Check(Option::instance().getShowToolTips());
        menuBar_->FindItem(MENU_VIEW_SHOW_MONEYTIPS)->Check(Option::instance().getShowMoneyTips());
        menuBar_->Refresh();
        menuBar_->Update();
        refreshPanelData();
        RefreshNavigationTree();

        // Reset columns of the checking panel in case the time columns was added/removed
        int id = panelCurrent_->GetId();
        if (id == mmID_CHECKING || id == mmID_ALLTRANSACTIONS || id == mmID_DELETEDTRANSACTIONS)
            wxDynamicCast(panelCurrent_, mmCheckingPanel)->ResetColumnView();

        const wxString& sysMsg = _("Settings have been updated.") + "\n\n"
            + _("Some settings take effect only after an application restart.");
        wxMessageBox(sysMsg, _("Settings"), wxOK | wxICON_INFORMATION);
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnCustomFieldsManager(wxCommandEvent& WXUNUSED(event))
{
    if (!m_db) return;
    mmCustomFieldListDialog dlg(this);
    dlg.ShowModal();
    createHomePage();
}

void mmGUIFrame::OnThemeManager(wxCommandEvent& /*event*/)
{
    mmThemesDialog dlg(this);
    dlg.ShowModal();
}

void mmGUIFrame::OnColumnOrderManager(wxCommandEvent& /*event*/)
{
    mmColumnsDialog dlg(this);
    dlg.ShowModal();
}

bool mmGUIFrame::OnRefreshWebApp(bool is_silent)
{
    if (mmWebApp::MMEX_WebApp_UpdateAccount()
        && mmWebApp::MMEX_WebApp_UpdateCategory()
        && mmWebApp::MMEX_WebApp_UpdatePayee()) {
        if (!is_silent) {
            wxMessageBox(_("Accounts, Payees, and Categories Updated"), _("Refresh WebApp"), wxOK | wxICON_INFORMATION);
        }
        return true;
    }
    else {
        if (!is_silent) {
            wxMessageBox(_("Issue encountered updating WebApp, check Web server and WebApp settings"),
                _("Refresh WebApp"), wxOK | wxICON_ERROR);
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
    setNavTreeSection(_("Help"));
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
    switch (event.GetId())
    {
    case MENU_FACEBOOK: url = mmex::weblink::Facebook; break;
    case MENU_TWITTER: url = mmex::weblink::Twitter; break;
    case MENU_WEBSITE: url = mmex::weblink::WebSite; break;
    case MENU_WIKI: url = mmex::weblink::Wiki; break;
    case MENU_DONATE: url = mmex::weblink::Donate; break;
    case MENU_CROWDIN: url = mmex::weblink::Crowdin; break;
    case MENU_REPORTISSUES: url = mmex::weblink::Forum; break;
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
    Model_Setting::instance().Set(INIDB_NEWS_LAST_READ_DATE, wxDate::Today().FormatISODate());
    wxLaunchDefaultBrowser(mmex::weblink::News);

    int toolbar_icon_size = Option::instance().getToolbarIconSize();
    //toolBar_->SetToolBitmapSize(wxSize(toolbar_icon_size, toolbar_icon_size));
    toolBar_->SetToolBitmap(MENU_ANNOUNCEMENTMAILING, mmBitmapBundle(png::NEWS, toolbar_icon_size));

    const auto b = toolBar_->FindTool(MENU_ANNOUNCEMENTMAILING);
    if (b) b->SetShortHelp(_("News"));
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnReportBug(wxCommandEvent& WXUNUSED(event))
{
    mmPrintableBase* br = new mmBugReport();
    setNavTreeSection(_("Reports"));
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
    switch (end_mod)
    {
    case wxID_FILE1:
    {
        wxFileName fname(Model_Setting::instance().getLastDbPath());
        if (fname.IsOk()) {
            SetDatabaseFile(fname.GetFullPath());
        }
        break;
    }
    case wxID_OPEN:
    {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_OPEN);
        AddPendingEvent(evt);
        break;
    }
    case wxID_NEW:
    {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_NEW);
        AddPendingEvent(evt);
        break;
    }
    case wxID_SETUP:
    {
        auto language = Option::instance().getLanguageID(true);
        const auto langName = language == wxLANGUAGE_DEFAULT ? _("System default") : wxLocale::GetLanguageName(language);
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
    wxString fileName = wxFileSelector(_("Choose HTML file to Export")
        , wxEmptyString, wxEmptyString, wxEmptyString
        , "*.html", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (!fileName.empty())
    {
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
    if (id == mmID_HOMEPAGE)
    {
        mmHomePagePanel* homePage = wxDynamicCast(panelCurrent_, mmHomePagePanel);
        homePage->createHtml();
    }
    else
    {
        DoWindowsFreezeThaw(homePanel_);
        wxSizer *sizer = cleanupHomePanel();
        panelCurrent_ = new mmHomePagePanel(homePanel_
            , this, mmID_HOMEPAGE
            , wxDefaultPosition, wxDefaultSize
            , wxNO_BORDER | wxTAB_TRAVERSAL
        );
        sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
        homePanel_->Layout();
        DoWindowsFreezeThaw(homePanel_);
    }

    if (m_nav_tree_ctrl->GetRootItem().IsOk()) {
        m_nav_tree_ctrl->SelectItem(m_nav_tree_ctrl->GetRootItem());
    }
    m_nav_tree_ctrl->SetEvtHandlerEnabled(true);

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
    panelCurrent_ = new mmReportsPanel(rs
        , cleanup, homePanel_, this, mmID_REPORTS
        , wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);

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
    if (panelCurrent_->GetId() == mmID_BILLS)
    {
        mmBillsDepositsPanel* billsDepositsPanel = wxDynamicCast(panelCurrent_, mmBillsDepositsPanel);
        billsDepositsPanel->RefreshList();
    }
    else
    {
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
    setNavTreeSection(_("Scheduled Transactions"));
}
//----------------------------------------------------------------------------

void mmGUIFrame::createBudgetingPage(int budgetYearID)
{
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String("Budget Panel");

    const auto time = wxDateTime::UNow();

    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);
    if (panelCurrent_->GetId() == mmID_BUDGET)
    {
        mmBudgetingPanel* budgetingPage = wxDynamicCast(panelCurrent_, mmBudgetingPanel);
        budgetingPage->DisplayBudgetingDetails(budgetYearID);
    }
    else
    {
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

void mmGUIFrame::createAllTransactionsPage()
{
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String("All Transactions");

    const auto time = wxDateTime::UNow();

    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);
    if (panelCurrent_->GetId() == mmID_ALLTRANSACTIONS)
    {
        mmCheckingPanel* checkingAccountPage = wxDynamicCast(panelCurrent_, mmCheckingPanel);
        checkingAccountPage->RefreshList();
    }
    else
    {

        DoWindowsFreezeThaw(homePanel_);
        wxSizer *sizer = cleanupHomePanel();
        panelCurrent_ = new mmCheckingPanel(homePanel_, this, -1, mmID_ALLTRANSACTIONS);
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
    m_nav_tree_ctrl->SetFocus();
}

void mmGUIFrame::createDeletedTransactionsPage()
{
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String("Deleted Transactions");

    const auto time = wxDateTime::UNow();

    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);
    if (panelCurrent_->GetId() == mmID_DELETEDTRANSACTIONS)
    {
        mmCheckingPanel* checkingAccountPage = wxDynamicCast(panelCurrent_, mmCheckingPanel);
        checkingAccountPage->RefreshList();
    }
    else
    {

        DoWindowsFreezeThaw(homePanel_);
        wxSizer* sizer = cleanupHomePanel();
        panelCurrent_ = new mmCheckingPanel(homePanel_, this, -2, mmID_DELETEDTRANSACTIONS);
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
    m_nav_tree_ctrl->SetFocus();
}

void mmGUIFrame::createCheckingAccountPage(int accountID)
{
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String("Checking Panel");

    const auto time = wxDateTime::UNow();

    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);

    // Check if the credit balance needs to be displayed or not
    // If this differs from before then we need to rebuild
    Model_Account::Data* account = Model_Account::instance().get(accountID);
    bool newCreditDisplayed = (0 == account->CREDITLIMIT) ? false : true;

    if (panelCurrent_->GetId() == mmID_CHECKING && (newCreditDisplayed == creditDisplayed_))
    {
        mmCheckingPanel* checkingAccountPage = wxDynamicCast(panelCurrent_, mmCheckingPanel);
        checkingAccountPage->RefreshList();
        checkingAccountPage->DisplayAccountDetails(accountID);
    }
    else
    {
        DoWindowsFreezeThaw(homePanel_);
        creditDisplayed_ = (0 == account->CREDITLIMIT) ? false : true;
        wxSizer *sizer = cleanupHomePanel();
        panelCurrent_ = new mmCheckingPanel(homePanel_, this, accountID, mmID_CHECKING);
        sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
        homePanel_->Layout();
        DoWindowsFreezeThaw(homePanel_);
    }

    json_writer.Key("seconds");
    json_writer.Double((wxDateTime::UNow() - time).GetMilliseconds().ToDouble() / 1000);
    json_writer.EndObject();

    Model_Usage::instance().AppendToUsage(wxString::FromUTF8(json_buffer.GetString()));

    menuPrintingEnable(true);
    if (gotoTransID_ > 0)
    {
        wxDynamicCast(panelCurrent_, mmCheckingPanel)->SetSelectedTransaction(gotoTransID_);
        gotoTransID_ = -1;
    }
    m_nav_tree_ctrl->SetEvtHandlerEnabled(true);
    m_nav_tree_ctrl->SetFocus();
}

void mmGUIFrame::createStocksAccountPage(int accountID)
{
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String("Stock Panel");

    const auto time = wxDateTime::UNow();

    if (panelCurrent_->GetId() == mmID_STOCKS)
    {
        mmStocksPanel* stocksPage = wxDynamicCast(panelCurrent_, mmStocksPanel);
        stocksPage->DisplayAccountDetails(accountID);
    }
    else
    {
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
        proper_type = Model_Account::type(acc) != Model_Account::INVESTMENT;
    if (proper_type)
        createCheckingAccountPage(gotoAccountID_);
    m_nav_tree_ctrl->Refresh();
}

void mmGUIFrame::OnGotoStocksAccount(wxCommandEvent& WXUNUSED(event))
{
    bool proper_type = false;
    Model_Account::Data *acc = Model_Account::instance().get(gotoAccountID_);
    if (acc)
        proper_type = Model_Account::type(acc) == Model_Account::INVESTMENT;
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
    else
    {
        DoWindowsFreezeThaw(homePanel_);
        wxSizer* sizer = cleanupHomePanel();
        panelCurrent_ = new mmAssetsPanel(this, homePanel_, mmID_ASSETS);
        sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
        homePanel_->Layout();
        DoWindowsFreezeThaw(homePanel_);
        menuPrintingEnable(true);
        setNavTreeSection(_("Assets"));
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
            .Title(_("Downloading stock prices from Yahoo"))
            .Text(_("Please wait..."))
            .Foreground(*wxWHITE)
            .Background(wxColour(0, 102, 51))
            .Transparency(4 * wxALPHA_OPAQUE / 5)
            );
#else
        (_("Downloading stock prices from Yahoo"), this);
#endif
    wxString msg;
    getOnlineCurrencyRates(msg);
    wxLogDebug("%s", msg);

    Model_Stock::Data_Set stock_list = Model_Stock::instance().all();
    if (!stock_list.empty()) {

        std::map<wxString, double> symbols;
        for (const auto& stock : stock_list)
        {
            const wxString symbol = stock.SYMBOL.Upper();
            if (symbol.IsEmpty()) continue;
            symbols[symbol] = stock.CURRENTPRICE;
        }

        std::map<wxString, double> stocks_data;
        if (get_yahoo_prices(symbols, stocks_data, "", msg, yahoo_price_type::SHARES))
        {

            Model_StockHistory::instance().Savepoint();
            for (auto& s : stock_list)
            {
                std::map<wxString, double>::const_iterator it = stocks_data.find(s.SYMBOL.Upper());
                if (it == stocks_data.end()) {
                    continue;
                }

                double dPrice = it->second;

                if (dPrice != 0)
                {
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
            strLastUpdate.Printf(_("%1$s on %2$s"), wxDateTime::Now().FormatTime()
                , mmGetDateForDisplay(wxDateTime::Now().FormatISODate()));
            Model_Infotable::instance().Set("STOCKS_LAST_REFRESH_DATETIME", strLastUpdate);
        }

        wxLogDebug("%s", msg);
    }

    refreshPanelData();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnEditAccount(wxCommandEvent& /*event*/)
{
    const auto &accounts = Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME);
    if (accounts.empty())
    {
        wxMessageBox(_("No account available to edit!"), _("Accounts"), wxOK | wxICON_WARNING);
        return;
    }

    mmSingleChoiceDialog scd(this, _("Choose Account to Edit"), _("Accounts"), accounts);
    if (scd.ShowModal() == wxID_OK)
    {
        Model_Account::Data* account = Model_Account::instance().get(scd.GetStringSelection());
        mmNewAcctDialog dlg(account, this);
        if (dlg.ShowModal() == wxID_OK) {
            RefreshNavigationTree();
        }
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnDeleteAccount(wxCommandEvent& /*event*/)
{
    const auto &accounts = Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME);
    if (accounts.empty())
    {
        wxMessageBox(_("No account available to delete!"), _("Accounts"), wxOK | wxICON_WARNING);
        return;
    }

    mmSingleChoiceDialog scd(this, _("Choose Account to Delete"), _("Accounts"), accounts);
    if (scd.ShowModal() == wxID_OK)
    {
        Model_Account::Data* account = Model_Account::instance().get(scd.GetStringSelection());
        wxString deletingAccountName = wxString::Format(
            _("Are you sure you want to delete\n%1$s account: %2$s?")
            , wxGetTranslation(account->ACCOUNTTYPE)
            , account->ACCOUNTNAME);
        wxMessageDialog msgDlg(this, deletingAccountName, _("Confirm Account Deletion"),
            wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION);
        if (msgDlg.ShowModal() == wxID_YES)
        {
            Model_Account::instance().remove(account->id());
            mmAttachmentManage::DeleteAllAttachments(Model_Attachment::reftype_desc(Model_Attachment::BANKACCOUNT), account->id());
        }
    }
    DoRecreateNavTreeControl(true);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnReallocateAccount(wxCommandEvent& WXUNUSED(event))
{
    mmSingleChoiceDialog account_choice(this
        , _("Select account"), _("Change Account Type")
        , Model_Account::instance().all_checking_account_names());

    if (account_choice.ShowModal() == wxID_OK)
    {
        Model_Account::Data* account = Model_Account::instance().get(account_choice.GetStringSelection());
        if (account)
            ReallocateAccount(account->ACCOUNTID);
    }
}

void mmGUIFrame::ReallocateAccount(int accountID)
{
    Model_Account::Data* account = Model_Account::instance().get(accountID);

    wxArrayString types = Model_Account::instance().all_type();
    types.Remove(Model_Account::all_type()[Model_Account::INVESTMENT]);
    types.Remove(account->ACCOUNTTYPE);
    wxArrayString t;
    for (const auto &entry : types)
        t.Add(wxGetTranslation(entry));

    mmSingleChoiceDialog type_choice(this
        , wxString::Format(_("Select new account type for %s"), account->ACCOUNTNAME)
        , _("Change Account Type"), t);

    if (type_choice.ShowModal() == wxID_OK)
    {
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
    Model_Setting::instance().Set("SHOWTOOLBAR", event.IsChecked());
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
    Option::instance().HideShareAccounts(!Option::instance().HideShareAccounts());
    RefreshNavigationTree();
}

void mmGUIFrame::OnHideDeletedTransactions(wxCommandEvent& WXUNUSED(event))
{
    Option::instance().HideDeletedTransactions(!Option::instance().HideDeletedTransactions());
    RefreshNavigationTree();
}

void mmGUIFrame::RefreshNavigationTree()
{
    // Save currently selected item data
    mmTreeItemData* iData = nullptr;
    wxString sectionName;
    wxTreeItemId selection = m_nav_tree_ctrl->GetSelection();
    if (selection.IsOk() && selectedItemData_ && selection != m_nav_tree_ctrl->GetRootItem())
    {
        iData = new mmTreeItemData(*selectedItemData_);
        // also save current section
        wxTreeItemId parentID = m_nav_tree_ctrl->GetItemParent(selection);
        if (parentID.IsOk())
            sectionName = m_nav_tree_ctrl->GetItemText(parentID);
    }
    DoRecreateNavTreeControl();
    // Find and reselect the previously selected item
    if (iData)
    {
        // search for the item first under the selected section
        wxTreeItemId navTreeID = getTreeItemfor(m_nav_tree_ctrl->GetRootItem(), sectionName);
        if (navTreeID.IsOk())
            navTreeID = findItemByData(navTreeID, *iData);
        // if we didn't find it search all nodes from root
        if (!navTreeID.IsOk())
            navTreeID = findItemByData(m_nav_tree_ctrl->GetRootItem(), *iData);
        if (navTreeID.IsOk())
        {
            m_nav_tree_ctrl->EnsureVisible(navTreeID);
            m_nav_tree_ctrl->SelectItem(navTreeID);
        }
    }
    delete(iData);
}

wxTreeItemId mmGUIFrame::findItemByData(wxTreeItemId itemId, mmTreeItemData& searchData)
{
    // Check if the current item's data matches the search data
    if (*dynamic_cast<mmTreeItemData*>(m_nav_tree_ctrl->GetItemData(itemId)) == searchData)
        return itemId;

    wxTreeItemIdValue searchCookie;
    wxTreeItemId childId = m_nav_tree_ctrl->GetFirstChild(itemId, searchCookie);
    while (childId.IsOk())
    {
        // Recursively search for the item in the child subtree
        wxTreeItemId foundId = findItemByData(childId, searchData);
        if (foundId.IsOk())
        {
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
    Option::instance().BudgetFinancialYears(!Option::instance().BudgetFinancialYears());
    refreshPanelData();
}

void mmGUIFrame::OnViewBudgetTransferTotal(wxCommandEvent& WXUNUSED(event))
{
    Option::instance().BudgetIncludeTransfers(!Option::instance().BudgetIncludeTransfers());
    refreshPanelData();
}

void mmGUIFrame::OnViewBudgetCategorySummary(wxCommandEvent& WXUNUSED(event))
{
    Option::instance().BudgetReportWithSummaries(!Option::instance().BudgetReportWithSummaries());
    refreshPanelData();
}

void mmGUIFrame::OnViewIgnoreFutureTransactions(wxCommandEvent& WXUNUSED(event))
{
    Option::instance().IgnoreFutureTransactions(!Option::instance().getIgnoreFutureTransactions());
    RefreshNavigationTree();
}

void mmGUIFrame::OnViewShowToolTips(wxCommandEvent& WXUNUSED(event))
{
    Option::instance().ShowToolTips(!Option::instance().getShowToolTips());
    RefreshNavigationTree();
}

void mmGUIFrame::OnViewShowMoneyTips(wxCommandEvent& WXUNUSED(event))
{
    Option::instance().ShowMoneyTips(!Option::instance().getShowMoneyTips());
    RefreshNavigationTree();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnCategoryRelocation(wxCommandEvent& /*event*/)
{
    relocateCategoryDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxString msgStr;
        msgStr << _("Merge categories completed") << "\n\n"
            << wxString::Format(_("Records have been updated in the database: %i"),
                dlg.updatedCategoriesCount());
        wxMessageBox(msgStr, _("Merge categories result"));
        refreshPanelData();
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnPayeeRelocation(wxCommandEvent& /*event*/)
{
    relocatePayeeDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxString msgStr;
        msgStr << _("Merge payees completed") << "\n\n"
            << wxString::Format(_("Records have been updated in the database: %i"),
                dlg.updatedPayeesCount())
            << "\n\n";
        wxMessageBox(msgStr, _("Merge payees result"));
        refreshPanelData();
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnTagRelocation(wxCommandEvent& /*event*/)
{
    relocateTagDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxString msgStr;
        msgStr << _("Merge tags completed") << "\n\n"
            << wxString::Format(_("Records have been updated in the database: %i"),
                dlg.updatedTagsCount())
            << "\n\n";
        wxMessageBox(msgStr, _("Merge tags result"));
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
    wxDateSpan days = wxDateSpan::Days(Model_Setting::instance().GetIntSetting("DELETED_TRANS_RETAIN_DAYS", 30));
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
            const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
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

    if (openFile(dbFileName, newDatabase))
    {
        DoRecreateNavTreeControl(true);
        mmLoadColorsFromDatabase();
    }
    else
    {
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
    if (file.FileExists())
    {
        SetDatabaseFile(file_name);
        saveSettings();
    }
    else
    {
        wxMessageBox(wxString::Format(_("File %s not found"), file_name), _("Error"), wxOK | wxICON_ERROR);
        m_recentFiles->RemoveFileFromHistory(fileNum);
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnClearRecentFiles(wxCommandEvent& /*event*/)
{
    m_recentFiles->Clear();
    m_recentFiles->AddFileToHistory(m_filename);
}

void mmGUIFrame::setGotoAccountID(int account_id, long transID)
{
    gotoAccountID_ = account_id;
    gotoTransID_ = transID;
}

void mmGUIFrame::OnToggleFullScreen(wxCommandEvent& WXUNUSED(event))
{
#if (wxMAJOR_VERSION >= 3 && wxMINOR_VERSION >= 0)
    this->ShowFullScreen(!IsFullScreen());
#endif
}

void mmGUIFrame::OnResetView(wxCommandEvent& WXUNUSED(event))
{
    Model_Setting::instance().Set("SHOWTOOLBAR", true);
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
        mmErrorDialogs::MessageWarning(this
            , _("The language for this application has been changed. "
                "The change will take effect the next time the application is started.")
            , _("Language change"));
}

void mmGUIFrame::OnKeyDown(wxTreeEvent& event)
{
    if (selectedItemData_)
    {
        auto data = selectedItemData_->getString();

        int key_code = event.GetKeyCode();

        if (key_code == WXK_RETURN || key_code == WXK_NUMPAD_ENTER)
        {
            if (data == "item@Transaction Report")
            {
                OnTransactionReport(event);
            }
        }
    }
    event.Skip();
}

void mmGUIFrame::DoUpdateBudgetNavigation(wxTreeItemId& parent_item)
{
    const auto all_budgets = Model_Budgetyear::instance().all(Model_Budgetyear::COL_BUDGETYEARNAME);
    if (!all_budgets.empty())
    {
        std::map <wxString, int> years;

        wxRegEx pattern_year(R"(^([0-9]{4})$)");
        wxRegEx pattern_month(R"(^([0-9]{4})-([0-9]{2})$)");

        for (const auto& e : all_budgets)
        {
            const wxString& name = e.BUDGETYEARNAME;
            if (pattern_year.Matches(name))
            {
                years[name] = e.BUDGETYEARID;

            }
            else
            {
                if (pattern_month.Matches(name)) {
                    wxString root_year = pattern_month.GetMatch(name, 1);
                    if (years.find(root_year) == years.end()) {
                        years[root_year] = e.BUDGETYEARID;
                    }
                }
            }
        }

        for (const auto& entry : years)
        {
            wxTreeItemId year_budget;
            for (const auto& e : all_budgets)
            {
                if (entry.second == e.BUDGETYEARID) {
                    year_budget = m_nav_tree_ctrl->AppendItem(parent_item, e.BUDGETYEARNAME, img::CALENDAR_PNG, img::CALENDAR_PNG);
                    m_nav_tree_ctrl->SetItemData(year_budget, new mmTreeItemData(mmTreeItemData::BUDGET, e.BUDGETYEARID));
                }
                else if (pattern_month.Matches(e.BUDGETYEARNAME) && pattern_month.GetMatch(e.BUDGETYEARNAME, 1) == entry.first)
                {
                    wxTreeItemId month_budget = m_nav_tree_ctrl->AppendItem(year_budget, e.BUDGETYEARNAME, img::CALENDAR_PNG, img::CALENDAR_PNG);
                    m_nav_tree_ctrl->SetItemData(month_budget, new mmTreeItemData(mmTreeItemData::BUDGET, e.BUDGETYEARID));
                }
            }
        }
    }
}
