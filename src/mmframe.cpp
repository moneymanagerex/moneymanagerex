/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2012 Stefano Giorgio
 Copyright (C) 2013 Nikolay
 Copyright (C) 2014, 2017 James Higley
 Copyright (C) 2014 Guan Lisheng (guanlisheng@gmail.com)

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
#include "paths.h"
#include "payeedialog.h"
#include "relocatecategorydialog.h"
#include "relocatepayeedialog.h"
#include "recentfiles.h"
#include "stockspanel.h"
#include "transdialog.h"
#include "util.h"
#include "webapp.h"
#include "webappdialog.h"
#include "wizard_newdb.h"
#include "wizard_newaccount.h"
#include "wizard_update.h"

#include "reports/transactions.h"

#include "import_export/qif_export.h"
#include "import_export/qif_import_gui.h"
#include "import_export/univcsvdialog.h"

#include "Model_Setting.h"
#include "Model_Attachment.h"
#include "Model_Usage.h"
#include "Model_StockHistory.h"
#include "Model_Translink.h"
#include "Model_Budgetyear.h"
#include "Model_Shareinfo.h"
#include "Model_CurrencyHistory.h"
#include "Model_Infotable.h"

#include <stack>

//----------------------------------------------------------------------------

int REPEAT_TRANS_DELAY_TIME = 7000; // 7 seconds
//----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(mmGUIFrame, wxFrame)
EVT_MENU(MENU_NEW, mmGUIFrame::OnNew)
EVT_MENU(MENU_OPEN, mmGUIFrame::OnOpen)
EVT_MENU(MENU_SAVE_AS, mmGUIFrame::OnSaveAs)
EVT_MENU(MENU_EXPORT_CSV, mmGUIFrame::OnExportToCSV)
EVT_MENU(MENU_EXPORT_XML, mmGUIFrame::OnExportToXML)
EVT_MENU(MENU_EXPORT_QIF, mmGUIFrame::OnExportToQIF)
EVT_MENU(MENU_EXPORT_WEBAPP, mmGUIFrame::OnExportToWebApp)
EVT_MENU(MENU_IMPORT_QIF, mmGUIFrame::OnImportQIF)
EVT_MENU(MENU_IMPORT_UNIVCSV, mmGUIFrame::OnImportUniversalCSV)
EVT_MENU(MENU_IMPORT_XML, mmGUIFrame::OnImportXML)
EVT_MENU(MENU_IMPORT_WEBAPP, mmGUIFrame::OnImportWebApp)
EVT_MENU(wxID_EXIT, mmGUIFrame::OnQuit)
EVT_MENU(MENU_NEWACCT, mmGUIFrame::OnNewAccount)
EVT_MENU(MENU_ACCTLIST, mmGUIFrame::OnAccountList)
EVT_MENU(MENU_ACCTEDIT, mmGUIFrame::OnEditAccount)
EVT_MENU(MENU_ACCTDELETE, mmGUIFrame::OnDeleteAccount)
EVT_MENU(MENU_ACCOUNT_REALLOCATE, mmGUIFrame::OnReallocateAccount)
EVT_MENU(MENU_ORGCATEGS, mmGUIFrame::OnOrgCategories)
EVT_MENU(MENU_ORGPAYEE, mmGUIFrame::OnOrgPayees)
EVT_MENU(wxID_PREFERENCES, mmGUIFrame::OnOptions)
EVT_MENU(wxID_NEW, mmGUIFrame::OnNewTransaction)
EVT_MENU(wxID_REFRESH, mmGUIFrame::refreshPanelData)
EVT_MENU(MENU_BUDGETSETUPDIALOG, mmGUIFrame::OnBudgetSetupDialog)
EVT_MENU(wxID_HELP, mmGUIFrame::OnHelp)
EVT_MENU(MENU_CHECKUPDATE, mmGUIFrame::OnCheckUpdate)
EVT_MENU(MENU_ANNOUNCEMENTMAILING, mmGUIFrame::OnBeNotified)
EVT_MENU_RANGE(MENU_FACEBOOK, MENU_TWITTER, mmGUIFrame::OnSimpleURLOpen)
EVT_MENU(MENU_REPORT_BUG, mmGUIFrame::OnReportBug)
EVT_MENU(wxID_ABOUT, mmGUIFrame::OnAbout)
EVT_MENU(wxID_PRINT, mmGUIFrame::OnPrintPage)
EVT_MENU(MENU_SHOW_APPSTART, mmGUIFrame::OnShowAppStartDialog)
EVT_MENU(MENU_EXPORT_HTML, mmGUIFrame::OnExportToHtml)
EVT_MENU(MENU_BILLSDEPOSITS, mmGUIFrame::OnBillsDeposits)
EVT_MENU(MENU_SET_PASSWORD, mmGUIFrame::OnSetPassword)
EVT_MENU(MENU_REMOVE_PASSWORD, mmGUIFrame::OnRemovePassword)
EVT_MENU(MENU_DB_VACUUM, mmGUIFrame::OnVacuumDB)
EVT_MENU(MENU_DB_DEBUG, mmGUIFrame::OnDebugDB)

EVT_MENU(MENU_ASSETS, mmGUIFrame::OnAssets)
EVT_MENU(MENU_CURRENCY, mmGUIFrame::OnCurrency)
EVT_MENU(MENU_TRANSACTIONREPORT, mmGUIFrame::OnTransactionReport)
EVT_MENU(wxID_VIEW_LIST, mmGUIFrame::OnGeneralReportManager)
EVT_MENU(wxID_BROWSE, mmGUIFrame::OnCustomFieldsManager)
EVT_MENU(MENU_TREEPOPUP_LAUNCHWEBSITE, mmGUIFrame::OnLaunchAccountWebsite)
EVT_MENU(MENU_TREEPOPUP_ACCOUNTATTACHMENTS, mmGUIFrame::OnAccountAttachments)
EVT_MENU(MENU_VIEW_TOOLBAR, mmGUIFrame::OnViewToolbar)
EVT_MENU(MENU_VIEW_LINKS, mmGUIFrame::OnViewLinks)
EVT_MENU(MENU_VIEW_HIDE_SHARE_ACCOUNTS, mmGUIFrame::OnHideShareAccounts)
EVT_MENU(MENU_VIEW_BUDGET_FINANCIAL_YEARS, mmGUIFrame::OnViewBudgetFinancialYears)
EVT_MENU(MENU_VIEW_BUDGET_TRANSFER_TOTAL, mmGUIFrame::OnViewBudgetTransferTotal)
EVT_MENU(MENU_VIEW_BUDGET_SETUP_SUMMARY, mmGUIFrame::OnViewBudgetSetupSummary)
EVT_MENU(MENU_VIEW_BUDGET_CATEGORY_SUMMARY, mmGUIFrame::OnViewBudgetCategorySummary)
EVT_MENU(MENU_VIEW_IGNORE_FUTURE_TRANSACTIONS, mmGUIFrame::OnViewIgnoreFutureTransactions)

EVT_MENU(MENU_CATEGORY_RELOCATION, mmGUIFrame::OnCategoryRelocation)
EVT_MENU(MENU_PAYEE_RELOCATION, mmGUIFrame::OnPayeeRelocation)

EVT_UPDATE_UI(MENU_VIEW_TOOLBAR, mmGUIFrame::OnViewToolbarUpdateUI)
EVT_UPDATE_UI(MENU_VIEW_LINKS, mmGUIFrame::OnViewLinksUpdateUI)
EVT_MENU(MENU_TREEPOPUP_NEW, mmGUIFrame::OnNewTransaction)
EVT_MENU(MENU_TREEPOPUP_EDIT, mmGUIFrame::OnPopupEditAccount)
EVT_MENU(MENU_TREEPOPUP_REALLOCATE, mmGUIFrame::OnPopupReallocateAccount)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_BASE_BALANCE, mmGUIFrame::OnPopupAccountBaseBalance)
EVT_MENU(MENU_TREEPOPUP_DELETE, mmGUIFrame::OnPopupDeleteAccount)

EVT_TREE_ITEM_MENU(wxID_ANY, mmGUIFrame::OnItemMenu)
EVT_TREE_ITEM_ACTIVATED(ID_NAVTREECTRL, mmGUIFrame::OnItemRightClick)
EVT_TREE_ITEM_EXPANDED(ID_NAVTREECTRL, mmGUIFrame::OnTreeItemExpanded)
EVT_TREE_ITEM_COLLAPSED(ID_NAVTREECTRL, mmGUIFrame::OnTreeItemCollapsed)

EVT_MENU(MENU_GOTOACCOUNT, mmGUIFrame::OnGotoAccount)
EVT_MENU(MENU_STOCKS, mmGUIFrame::OnGotoStocksAccount)

/* Navigation Panel */
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_NEW, mmGUIFrame::OnNewAccount)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_DELETE, mmGUIFrame::OnDeleteAccount)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_EDIT, mmGUIFrame::OnEditAccount)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_LIST, mmGUIFrame::OnAccountList)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_EXPORT2CSV, mmGUIFrame::OnExportToCSV)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_EXPORT2XML, mmGUIFrame::OnExportToXML)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_EXPORT2QIF, mmGUIFrame::OnExportToQIF)
//EVT_MENU(MENU_TREEPOPUP_ACCOUNT_IMPORTQIF, mmGUIFrame::OnImportQIF)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_IMPORTUNIVCSV, mmGUIFrame::OnImportUniversalCSV)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_IMPORTXML, mmGUIFrame::OnImportXML)
EVT_MENU_RANGE(MENU_TREEPOPUP_ACCOUNT_VIEWALL, MENU_TREEPOPUP_ACCOUNT_VIEWCLOSED, mmGUIFrame::OnViewAccountsTemporaryChange)

/*Automatic processing of repeat transactions*/
EVT_TIMER(AUTO_REPEAT_TRANSACTIONS_TIMER_ID, mmGUIFrame::OnAutoRepeatTransactionsTimer)

/* Recent Files */
EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, mmGUIFrame::OnRecentFiles)
EVT_MENU(MENU_RECENT_FILES_CLEAR, mmGUIFrame::OnClearRecentFiles)
EVT_MENU(MENU_VIEW_TOGGLE_FULLSCREEN, mmGUIFrame::OnToggleFullScreen)
EVT_CLOSE(mmGUIFrame::OnClose)

EVT_MENU_RANGE(MENU_TREEPOPUP_HIDE_SHOW_REPORT, MENU_TREEPOPUP_HIDE_SHOW_REPORT32, mmGUIFrame::OnHideShowReport)
EVT_MENU_RANGE(MENU_LANG+1, MENU_LANG_MAX, mmGUIFrame::OnChangeGUILanguage)

wxEND_EVENT_TABLE()
//----------------------------------------------------------------------------

mmGUIFrame::mmGUIFrame(mmGUIApp* app, const wxString& title
    , const wxPoint& pos
    , const wxSize& size)
    : wxFrame(0, -1, title, pos, size)
    , m_app(app)
    , gotoAccountID_(-1)
    , gotoTransID_(-1)
    , activeReport_(false)
    , autoRepeatTransactionsTimer_(this, AUTO_REPEAT_TRANSACTIONS_TIMER_ID)
    , panelCurrent_(nullptr)
    , homePanel_(nullptr)
    , m_nav_tree_ctrl(nullptr)
    , menuBar_(nullptr)
    , toolBar_(nullptr)
    , selectedItemData_(nullptr)
    , helpFileIndex_(-1)
    , homePage_(nullptr)
    , checkingAccountPage_(nullptr)
    , billsDepositsPanel_(nullptr)
    , budgetingPage_(nullptr)
    , m_hide_share_accounts(true)
    , m_commit_callback_hook(nullptr)
    , m_update_callback_hook(nullptr)
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
    wxFileName dbpath = m_app->m_optParam;
    if (!dbpath.IsOk())
    {
        from_scratch = Model_Setting::instance().GetBoolSetting("SHOWBEGINAPP", true);
        if (from_scratch)
            dbpath = wxGetEmptyString();
        else
            dbpath = Model_Setting::instance().getLastDbPath();
    }

    //Read news
    getNewsRSS(g_WebsiteNewsList);

    /* Create the Controls for the frame */
    createMenu();
    createToolBar();
    // Disable menu items incase no database is established.
    menuEnableItems(false);
    createControls();

#if wxUSE_STATUSBAR
    CreateStatusBar();
#endif // wxUSE_STATUSBAR
    m_recentFiles = new mmFileHistory(); // TODO Max files
    m_recentFiles->UseMenu(m_menuRecentFiles);
    m_recentFiles->Load();

    // Load perspective
    const wxString auiPerspective = Model_Setting::instance()
        .GetStringSetting("AUIPERSPECTIVE", wxEmptyString);
    m_mgr.LoadPerspective(auiPerspective);

    // add the toolbars to the manager
    m_mgr.AddPane(toolBar_, wxAuiPaneInfo().
        Name("toolbar").ToolbarPane().Top()
        .LeftDockable(false).RightDockable(false).MinSize(1000, -1)
        .Show(Model_Setting::instance().GetBoolSetting("SHOWTOOLBAR", true)));

    // change look and feel of wxAuiManager
    m_mgr.GetArtProvider()->SetMetric(16, 0);
    m_mgr.GetArtProvider()->SetMetric(3, 1);

    // "commit" all changes made to wxAuiManager
    m_mgr.GetPane("Navigation").Caption(_("Navigation"));
    m_mgr.GetPane("toolbar").Caption(_("Toolbar"));
    m_mgr.Update();

    // Show license agreement at first open
    if (Model_Setting::instance().GetStringSetting(INIDB_SEND_USAGE_STATS, "") == "")
    {
        mmAboutDialog(this, 4).ShowModal();
        Option::instance().SendUsageStatistics(true);
    }

    //Check for new version at startup
    if (Model_Setting::instance().GetBoolSetting("UPDATECHECK", true))
        mmUpdate::checkUpdates(true, this);

    //Show appstart
    if (from_scratch || !dbpath.IsOk())
    {
        menuEnableItems(false);
        showBeginAppDialog(dbpath.GetFullName().IsEmpty());
    }
    else
        SetDatabaseFile(dbpath.GetFullPath());

    const wxAcceleratorEntry entries [] =
    {
        wxAcceleratorEntry(wxACCEL_NORMAL, WXK_F9, wxID_NEW),
        wxAcceleratorEntry(wxACCEL_NORMAL, WXK_F5, wxID_REFRESH),
    };

    wxAcceleratorTable tab(sizeof(entries) / sizeof(*entries), entries);
    SetAcceleratorTable(tab);
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
    delete m_recentFiles;
    if (!m_filename.IsEmpty()) // Exiting before file is opened
        saveSettings();

    wxTreeItemId rootitem = m_nav_tree_ctrl->GetRootItem();
    cleanupNavTreeControl(rootitem);
    m_mgr.UnInit();

    /* Delete the GUI */
    cleanupHomePanel(false);
    ShutdownDatabase();
    /// Update the database according to user requirements
    if (Option::instance().DatabaseUpdated() && Model_Setting::instance().GetBoolSetting("BACKUPDB_UPDATE", false))
    {
        dbUpgrade::BackupDB(m_filename, dbUpgrade::BACKUPTYPE::CLOSE, Model_Setting::instance().GetIntSetting("MAX_BACKUP_FILES", 4));
    }
}

void mmGUIFrame::ShutdownDatabase()
{
    if (m_db)
    {
        m_db->SetCommitHook(nullptr);
        m_db->Close();
        delete m_commit_callback_hook;
        delete m_update_callback_hook;
        m_db.reset();
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
        m_nav_tree_ctrl->SetItemData(item, 0);
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
        if (accountName == m_nav_tree_ctrl->GetItemText(navTreeID))
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
    return accountNotFound;
}

//----------------------------------------------------------------------------
void mmGUIFrame::setAccountNavTreeSection(const wxString& accountName)
{
    if (setAccountInSection(wxTRANSLATE("Bank Accounts"), accountName)) return;
    if (setAccountInSection(wxTRANSLATE("Credit Card Accounts"), accountName)) return;
    if (setAccountInSection(wxTRANSLATE("Term Accounts"), accountName)) return;
    if (setAccountInSection(wxTRANSLATE("Stock Portfolios"), accountName)) return;
    if (setAccountInSection(wxTRANSLATE("Cash Accounts"), accountName)) return;
    if (setAccountInSection(wxTRANSLATE("Loan Accounts"), accountName)) return;
    if (setAccountInSection(wxTRANSLATE("Assets"), accountName)) return;
    if (setAccountInSection(wxTRANSLATE("Crypto Wallets"), accountName)) return;
}

//----------------------------------------------------------------------------
void mmGUIFrame::OnAutoRepeatTransactionsTimer(wxTimerEvent& WXUNUSED(event))
{
    //WebApp check
    if (mmWebApp::WebApp_CheckEnabled())
    {
        if (mmWebApp::WebApp_CheckGuid() && mmWebApp::WebApp_CheckApiVersion())
        {
            mmWebApp::WebApp_UpdateAccount();
            mmWebApp::WebApp_UpdatePayee();
            mmWebApp::WebApp_UpdateCategory();

            mmWebApp::WebTranVector dummy;
            if (mmWebApp::WebApp_DownloadNewTransaction(dummy, true))
            {
                mmWebAppDialog dlg(this);
                dlg.ShowModal();
                if (dlg.getRefreshRequested())
                    refreshPanelData();
            }
        }
    }

    //Auto recurring transaction
    bool continueExecution = false;

    Model_Billsdeposits::AccountBalance bal;
    Model_Billsdeposits& bills = Model_Billsdeposits::instance();
    for (const auto& q1 : bills.all())
    {
        bills.decode_fields(q1);
        bool allow_transaction = bills.AllowTransaction(q1, bal);
        const wxDateTime payment_date = bills.TRANSDATE(q1);
        if (bills.autoExecuteManual() && bills.requireExecution())
        {
            if (allow_transaction && bills.allowExecution())
            {
                continueExecution = true;
                mmBDDialog repeatTransactionsDlg(this, q1.BDID, false, true);
                repeatTransactionsDlg.SetDialogHeader(_("Auto Repeat Transactions"));
                if (repeatTransactionsDlg.ShowModal() == wxID_OK)
                {
                    refreshPanelData();
                }
                else // stop repeat executions from occurring
                    continueExecution = false;
            }
        }

        if (allow_transaction && bills.autoExecuteSilent() && bills.requireExecution())
        {
            if (bills.allowExecution())
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
                tran->SUBCATEGID = q1.SUBCATEGID;
                tran->TRANSDATE = payment_date.FormatISODate();

                int transID = Model_Checking::instance().save(tran);

                Model_Splittransaction::Cache checking_splits;
                for (const auto &item : Model_Billsdeposits::splittransaction(q1))
                {
                    Model_Splittransaction::Data *split = Model_Splittransaction::instance().create();
                    split->TRANSID = transID;
                    split->CATEGID = item.CATEGID;
                    split->SUBCATEGID = item.SUBCATEGID;
                    split->SPLITTRANSAMOUNT = item.SPLITTRANSAMOUNT;
                    checking_splits.push_back(split);
                }
                Model_Splittransaction::instance().save(checking_splits);
            }
            Model_Billsdeposits::instance().completeBDInSeries(q1.BDID);
            createHomePage();
        }
    }

    if (continueExecution)
    {
        autoRepeatTransactionsTimer_.Start(5, wxTIMER_ONE_SHOT);
    }
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
    Model_Setting::instance().Set("ISMAXIMIZED", (bool)this->IsMaximized());
    Model_Setting::instance().ReleaseSavepoint();
}
//----------------------------------------------------------------------------

void mmGUIFrame::menuEnableItems(bool enable)
{
    menuBar_->FindItem(MENU_SAVE_AS)->Enable(enable);
    menuBar_->FindItem(MENU_EXPORT)->Enable(enable);
    menuBar_->FindItem(MENU_NEWACCT)->Enable(enable);
    menuBar_->FindItem(MENU_ACCTLIST)->Enable(enable);
    menuBar_->FindItem(MENU_ACCTEDIT)->Enable(enable);
    menuBar_->FindItem(MENU_ACCOUNT_REALLOCATE)->Enable(enable);
    menuBar_->FindItem(MENU_ACCTDELETE)->Enable(enable);

    menuBar_->FindItem(MENU_ORGCATEGS)->Enable(enable);
    menuBar_->FindItem(MENU_ORGPAYEE)->Enable(enable);
    menuBar_->FindItem(MENU_CATEGORY_RELOCATION)->Enable(enable);
    menuBar_->FindItem(MENU_PAYEE_RELOCATION)->Enable(enable);
    menuBar_->FindItem(wxID_VIEW_LIST)->Enable(enable);
    menuBar_->FindItem(wxID_BROWSE)->Enable(enable);
    menuBar_->FindItem(MENU_SET_PASSWORD)->Enable(enable);

    menuBar_->FindItem(MENU_IMPORT)->Enable(enable);
    menuBar_->FindItem(wxID_PRINT)->Enable(enable);
    menuBar_->FindItem(wxID_PREFERENCES)->Enable(enable);
    menuBar_->FindItem(MENU_BILLSDEPOSITS)->Enable(enable);
    menuBar_->FindItem(MENU_CURRENCY)->Enable(enable);
    menuBar_->FindItem(MENU_ASSETS)->Enable(enable);
    menuBar_->FindItem(MENU_BUDGETSETUPDIALOG)->Enable(enable);
    menuBar_->FindItem(MENU_TRANSACTIONREPORT)->Enable(enable);

    menuBar_->FindItem(MENU_VIEW_HIDE_SHARE_ACCOUNTS)->Enable(enable);
    menuBar_->FindItem(MENU_VIEW_BUDGET_FINANCIAL_YEARS)->Enable(enable);
    menuBar_->FindItem(MENU_VIEW_BUDGET_TRANSFER_TOTAL)->Enable(enable);
    menuBar_->FindItem(MENU_VIEW_BUDGET_SETUP_SUMMARY)->Enable(enable);
    menuBar_->FindItem(MENU_VIEW_BUDGET_CATEGORY_SUMMARY)->Enable(enable);
    menuBar_->FindItem(MENU_VIEW_IGNORE_FUTURE_TRANSACTIONS)->Enable(enable);

    for (int r = 0; r < Option::instance().ReportCount(); r++)
    {
        menuBar_->FindItem(MENU_TREEPOPUP_HIDE_SHOW_REPORT + r)->Enable(enable);
    }

    menuBar_->FindItem(MENU_DB_VACUUM)->Enable(enable);
    menuBar_->FindItem(MENU_DB_DEBUG)->Enable(enable);

    toolBar_->EnableTool(MENU_NEWACCT, enable);
    toolBar_->EnableTool(MENU_ACCTLIST, enable);
    toolBar_->EnableTool(MENU_ORGPAYEE, enable);
    toolBar_->EnableTool(MENU_ORGCATEGS, enable);
    toolBar_->EnableTool(MENU_CURRENCY, enable);
    toolBar_->EnableTool(wxID_VIEW_LIST, enable);
    toolBar_->EnableTool(wxID_BROWSE, enable);
    toolBar_->EnableTool(MENU_TRANSACTIONREPORT, enable);
    toolBar_->EnableTool(wxID_PREFERENCES, enable);
    toolBar_->EnableTool(wxID_NEW, enable);
    toolBar_->EnableTool(wxID_PRINT, enable);
}
//----------------------------------------------------------------------------

void mmGUIFrame::menuPrintingEnable(bool enable)
{
    menuBar_->FindItem(wxID_PRINT)->Enable(enable);
    menuBar_->FindItem(MENU_EXPORT_HTML)->Enable(enable);
}
//----------------------------------------------------------------------------

void mmGUIFrame::createControls()
{
#if defined (__WXGTK__) || defined (__WXMAC__)
    // Under GTK, row lines look ugly
    m_nav_tree_ctrl = new wxTreeCtrl( this, ID_NAVTREECTRL,
        wxDefaultPosition, wxSize(100, 100));
#else
    m_nav_tree_ctrl = new wxTreeCtrl(this, ID_NAVTREECTRL,
        wxDefaultPosition, wxSize(100, 100), wxTR_SINGLE | wxTR_HAS_BUTTONS | wxTR_ROW_LINES);
#endif

    m_nav_tree_ctrl->AssignImageList(navtree_images_list());
    m_nav_tree_ctrl->Connect(ID_NAVTREECTRL, wxEVT_TREE_SEL_CHANGED, wxTreeEventHandler(mmGUIFrame::OnSelChanged), nullptr, this);

    homePanel_ = new wxPanel(this, wxID_ANY,
        wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxTR_SINGLE | wxNO_BORDER);

    m_mgr.AddPane(m_nav_tree_ctrl, wxAuiPaneInfo()
        . Name("Navigation")
        . BestSize(wxSize(200, 100)).MinSize(wxSize(100, 100))
        . Left());

    m_mgr.AddPane(homePanel_, wxAuiPaneInfo()
        . Name("Home").Caption("Home")
        . CenterPane().PaneBorder(false));
}
//----------------------------------------------------------------------------

void mmGUIFrame::updateNavTreeControl()
{
    windowsFreezeThaw(m_nav_tree_ctrl);
    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);
    wxTreeItemId root = m_nav_tree_ctrl->GetRootItem();
    cleanupNavTreeControl(root);
    m_nav_tree_ctrl->DeleteAllItems();

    root = m_nav_tree_ctrl->AddRoot(_("Home Page"), img::HOUSE_PNG, img::HOUSE_PNG);
    m_nav_tree_ctrl->SetItemData(root, new mmTreeItemData("Home Page"));
    m_nav_tree_ctrl->SetItemBold(root, true);
    m_nav_tree_ctrl->SetFocus();

    wxTreeItemId accounts = m_nav_tree_ctrl->AppendItem(root, _("Bank Accounts")
        , img::SAVINGS_ACC_NORMAL_PNG, img::SAVINGS_ACC_NORMAL_PNG);
    m_nav_tree_ctrl->SetItemData(accounts, new mmTreeItemData("Bank Accounts"));
    m_nav_tree_ctrl->SetItemBold(accounts, true);

    wxTreeItemId cardAccounts = m_nav_tree_ctrl->AppendItem(root, _("Credit Card Accounts")
        , img::CARD_ACC_PNG, img::CARD_ACC_PNG);
    m_nav_tree_ctrl->SetItemData(cardAccounts, new mmTreeItemData("Credit Card Accounts"));
    m_nav_tree_ctrl->SetItemBold(cardAccounts, true);

    wxTreeItemId cashAccounts = m_nav_tree_ctrl->AppendItem(root, _("Cash Accounts")
        , img::CASH_ACC_NORMAL_PNG, img::CASH_ACC_NORMAL_PNG);
    m_nav_tree_ctrl->SetItemData(cashAccounts, new mmTreeItemData("Cash Accounts"));
    m_nav_tree_ctrl->SetItemBold(cashAccounts, true);

    wxTreeItemId loanAccounts = m_nav_tree_ctrl->AppendItem(root, _("Loan Accounts")
        , img::LOAN_ACC_NORMAL_PNG, img::LOAN_ACC_NORMAL_PNG);
    m_nav_tree_ctrl->SetItemData(loanAccounts, new mmTreeItemData("Loan Accounts"));
    m_nav_tree_ctrl->SetItemBold(loanAccounts, true);

    wxTreeItemId termAccounts = m_nav_tree_ctrl->AppendItem(root, _("Term Accounts")
        , img::TERMACCOUNT_PNG, img::TERMACCOUNT_PNG);
    m_nav_tree_ctrl->SetItemData(termAccounts, new mmTreeItemData("Term Accounts"));
    m_nav_tree_ctrl->SetItemBold(termAccounts, true);
    
    wxTreeItemId stocks = m_nav_tree_ctrl->AppendItem(root, _("Stock Portfolios")
        , img::STOCK_ACC_PNG, img::STOCK_ACC_PNG);
    m_nav_tree_ctrl->SetItemData(stocks, new mmTreeItemData("Stocks"));
    m_nav_tree_ctrl->SetItemBold(stocks, true);

    wxTreeItemId shareAccounts = m_nav_tree_ctrl->AppendItem(root, _("Share Accounts")
        , img::STOCK_ACC_PNG, img::STOCK_ACC_PNG);
    m_nav_tree_ctrl->SetItemData(shareAccounts, new mmTreeItemData("Share Accounts"));
    m_nav_tree_ctrl->SetItemBold(shareAccounts, true);

    wxTreeItemId assets = m_nav_tree_ctrl->AppendItem(root, _("Assets")
        , img::ASSET_PNG, img::ASSET_PNG);
    m_nav_tree_ctrl->SetItemData(assets, new mmTreeItemData("Assets"));
    m_nav_tree_ctrl->SetItemBold(assets, true);
    
    wxTreeItemId cryptoAccounts = m_nav_tree_ctrl->AppendItem(root, _("Crypto Wallets")
        , img::CRYPTO_PNG, img::CRYPTO_PNG);
    m_nav_tree_ctrl->SetItemData(cryptoAccounts, new mmTreeItemData("Crypto Wallets"));
    m_nav_tree_ctrl->SetItemBold(cryptoAccounts, true);

    wxTreeItemId bills = m_nav_tree_ctrl->AppendItem(root, _("Recurring Transactions")
        , img::SCHEDULE_PNG, img::SCHEDULE_PNG);
    m_nav_tree_ctrl->SetItemData(bills, new mmTreeItemData("Bills & Deposits"));
    m_nav_tree_ctrl->SetItemBold(bills, true);

    wxTreeItemId budgeting = m_nav_tree_ctrl->AppendItem(root, _("Budget Setup")
        , img::CALENDAR_PNG, img::CALENDAR_PNG);
    m_nav_tree_ctrl->SetItemData(budgeting, new mmTreeItemData("Budgeting"));
    m_nav_tree_ctrl->SetItemBold(budgeting, true);

    const DB_Table_BUDGETYEAR::Data_Set all_budgets 
        = Model_Budgetyear::instance().all(Model_Budgetyear::COL_BUDGETYEARNAME);
    bool have_budget = (all_budgets.size() > 0);
    for (const auto& e : Model_Budgetyear::instance().all(Model_Budgetyear::COL_BUDGETYEARNAME))
    {
        int id = e.BUDGETYEARID;
        const wxString& name = e.BUDGETYEARNAME;

        wxTreeItemId bYear = m_nav_tree_ctrl->AppendItem(budgeting
            , name, img::CALENDAR_PNG, img::CALENDAR_PNG);
        m_nav_tree_ctrl->SetItemData(bYear, new mmTreeItemData(id, true));
    }

    wxTreeItemId reports = m_nav_tree_ctrl->AppendItem(root
        , _("Reports"), img::PIECHART_PNG, img::PIECHART_PNG);
    m_nav_tree_ctrl->SetItemBold(reports, true);
    m_nav_tree_ctrl->SetItemData(reports, new mmTreeItemData("Reports"));

    this->updateReportNavigation(reports, have_budget);

    ///////////////////////////////////////////////////////////////////

    wxTreeItemId help = m_nav_tree_ctrl->AppendItem(root, _("Help")
        , img::HELP_PNG, img::HELP_PNG);
    m_nav_tree_ctrl->SetItemData(help, new mmTreeItemData("Help"));
    m_nav_tree_ctrl->SetItemBold(help, true);

    if (m_db)
    {
        /* Start Populating the dynamic data */
        wxString vAccts = Model_Setting::instance().ViewAccounts();
        wxASSERT(vAccts == VIEW_ACCOUNTS_ALL_STR || vAccts == VIEW_ACCOUNTS_FAVORITES_STR 
            || vAccts == VIEW_ACCOUNTS_OPEN_STR || vAccts == VIEW_ACCOUNTS_CLOSED_STR);

        for (const auto& account : Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME))
        {
            if ((vAccts == VIEW_ACCOUNTS_OPEN_STR) && (Model_Account::status(account) != Model_Account::OPEN))
                continue;
            else if (vAccts == VIEW_ACCOUNTS_FAVORITES_STR && !Model_Account::FAVORITEACCT(account))
                continue;
            else if (vAccts == VIEW_ACCOUNTS_CLOSED_STR && (Model_Account::status(account) == Model_Account::OPEN))
                continue;

            int selectedImage = Option::instance().AccountImageId(account.ACCOUNTID);

            wxTreeItemId tacct;

            switch (Model_Account::type(account))
            {
            case Model_Account::INVESTMENT:
                {
                    tacct = m_nav_tree_ctrl->AppendItem(stocks
                        , account.ACCOUNTNAME, selectedImage, selectedImage);
                    // find all the accounts associated with this stock portfolio
                    Model_Stock::Data_Set stock_account_list = Model_Stock::instance()
                        .find(Model_Stock::HELDAT(account.ACCOUNTID));
                    // Put the names of the Stock_entry names as children of the stock account.
                    for (const auto &stock_entry : stock_account_list)
                    {
                        if (Model_Translink::HasShares(stock_entry.STOCKID))
                        {
                            wxTreeItemId se = m_nav_tree_ctrl->AppendItem(tacct
                                , stock_entry.STOCKNAME, selectedImage, selectedImage);
                            int account_id = stock_entry.STOCKID;
                            if (Model_Translink::ShareAccountId(account_id))
                            {
                                m_nav_tree_ctrl->SetItemData(se, new mmTreeItemData(account_id, false));
                            }
                        }
                    }
                }
                break;
            case Model_Account::CRYPTO:
                tacct = m_nav_tree_ctrl->AppendItem(cryptoAccounts
                    , account.ACCOUNTNAME, selectedImage, selectedImage);
                break;
            case Model_Account::SHARES:
                tacct = m_nav_tree_ctrl->AppendItem(shareAccounts
                    , account.ACCOUNTNAME, selectedImage, selectedImage);
                break;
            case Model_Account::ASSET:
                tacct = m_nav_tree_ctrl->AppendItem(assets
                    , account.ACCOUNTNAME, selectedImage, selectedImage);
                break;
            case Model_Account::TERM:
                tacct = m_nav_tree_ctrl->AppendItem(termAccounts
                    , account.ACCOUNTNAME, selectedImage, selectedImage);
                break;
            case Model_Account::CREDIT_CARD:
                tacct = m_nav_tree_ctrl->AppendItem(cardAccounts
                    , account.ACCOUNTNAME, selectedImage, selectedImage);
                break;
            case Model_Account::CASH:
                tacct = m_nav_tree_ctrl->AppendItem(cashAccounts
                    , account.ACCOUNTNAME, selectedImage, selectedImage);
                break;
            case Model_Account::LOAN:
                tacct = m_nav_tree_ctrl->AppendItem(loanAccounts
                    , account.ACCOUNTNAME, selectedImage, selectedImage);
                break;
            default:
                tacct = m_nav_tree_ctrl->AppendItem(accounts
                    , account.ACCOUNTNAME, selectedImage, selectedImage);
                break;
            }

            //TODO: const wxString account_string = wxString::Format("%s_%i", account.ACCOUNTTYPE, account.ACCOUNTID);
            // m_nav_tree_ctrl->SetItemData(tacct, new mmTreeItemData(account_string));
            m_nav_tree_ctrl->SetItemData(tacct, new mmTreeItemData(account.ACCOUNTID, false));
        }

        loadNavTreeItemsStatus();
        if (!m_nav_tree_ctrl->ItemHasChildren(accounts)) m_nav_tree_ctrl->Delete(accounts);
        if (!m_nav_tree_ctrl->ItemHasChildren(cardAccounts)) m_nav_tree_ctrl->Delete(cardAccounts);
        if (!m_nav_tree_ctrl->ItemHasChildren(termAccounts)) m_nav_tree_ctrl->Delete(termAccounts);
        if (!m_nav_tree_ctrl->ItemHasChildren(stocks)) m_nav_tree_ctrl->Delete(stocks);
        if (!m_nav_tree_ctrl->ItemHasChildren(cashAccounts)) m_nav_tree_ctrl->Delete(cashAccounts);
        if (!m_nav_tree_ctrl->ItemHasChildren(loanAccounts)) m_nav_tree_ctrl->Delete(loanAccounts);
        if (!m_nav_tree_ctrl->ItemHasChildren(cryptoAccounts)) m_nav_tree_ctrl->Delete(cryptoAccounts);

        if (!m_nav_tree_ctrl->ItemHasChildren(shareAccounts) || m_hide_share_accounts)
        {
            m_nav_tree_ctrl->Delete(shareAccounts);
        }
    }
    windowsFreezeThaw(m_nav_tree_ctrl);
    m_nav_tree_ctrl->SelectItem(root);
    m_nav_tree_ctrl->Refresh();
    m_nav_tree_ctrl->Update();
    m_nav_tree_ctrl->SetEvtHandlerEnabled(true);
}

void mmGUIFrame::loadNavTreeItemsStatus()
{
    /* Load Nav Tree Control */
    SetEvtHandlerEnabled(false);
    wxTreeItemId root = m_nav_tree_ctrl->GetRootItem();
    m_nav_tree_ctrl->Expand(root);

    const wxString& str = Model_Infotable::instance().GetStringInfo("NAV_TREE_STATUS", "");
    Document json_doc;
    if (json_doc.Parse(str.c_str()).HasParseError()) {
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
            const wxString& nav_key = iData->getString();
            wxLogDebug("-%s-", nav_key);
            if (json_doc.HasMember(nav_key.c_str()))
            {
                Value json_key(nav_key.c_str(), json_doc.GetAllocator());
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

void mmGUIFrame::OnTreeItemCollapsed(wxTreeEvent& event)
{
    mmTreeItemData* iData =
        dynamic_cast<mmTreeItemData*>(m_nav_tree_ctrl->GetItemData(event.GetItem()));
    if (!iData) return;
    navTreeStateToJson();
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
        if (iData && iData->isStringData() && m_nav_tree_ctrl->IsExpanded(next))
        {
            json_writer.Key(iData->getString().c_str());
            json_writer.Bool(m_nav_tree_ctrl->IsExpanded(next));
        }
    };
    json_writer.EndObject();

    const wxString nav_tree_status = json_buffer.GetString();
    wxLogDebug("=========== navTreeStateToJson =============================");
    wxLogDebug(nav_tree_status);
    Model_Infotable::instance().Set("NAV_TREE_STATUS", nav_tree_status);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnSelChanged(wxTreeEvent& event)
{
    menuPrintingEnable(false);
    wxTreeItemId selectedItem = event.GetItem();
    if (!selectedItem) return;

    m_nav_tree_ctrl->Update();

    mmTreeItemData* iData = dynamic_cast<mmTreeItemData*>(m_nav_tree_ctrl->GetItemData(selectedItem));
    selectedItemData_ = iData;
    if (!iData) return;

    activeReport_ = false;
    if (!iData->isStringData())
    {
        if (iData->isBudgetingNode())
        {
            int year = iData->getData();
            createBudgetingPage(year);
        }
        else
        {
            int data = iData->getData();
            Model_Account::Data* account = Model_Account::instance().get(data);
            if (account)
            {
                gotoAccountID_ = data;
                if (Model_Account::type(account) != Model_Account::INVESTMENT)
                    createCheckingAccountPage(gotoAccountID_);
                else
                    createStocksAccountPage(gotoAccountID_);
                m_nav_tree_ctrl->SetFocus();
                menuPrintingEnable(true);
            }
            else
            {
                /* cannot find accountid */
                wxASSERT(false);
            }
        }
    }
    else
    {
        helpFileIndex_ = -1;
        const wxString data = iData->getString();
        if (data == "item@Help")
            helpFileIndex_ = mmex::HTML_INDEX;
        else if (data == "item@Stocks")
            helpFileIndex_ = mmex::HTML_INVESTMENT;
        else if (data == "item@Budgeting")
            helpFileIndex_ = mmex::HTML_BUDGET;
        else if (data == "item@Reports")
            helpFileIndex_ = mmex::HTML_CUSTOM_SQL;

        if (helpFileIndex_ > -1)
        {
            createHelpPage();
            return;
        }

        if (!m_db) return;

        if (data == "item@Home Page")
        {
            createHomePage();
            return;
        }
        wxCommandEvent *evt = 0;
        if (data == "item@Assets")
            evt = new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, MENU_ASSETS);
        else if (data == "item@Bills & Deposits")
            evt = new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, MENU_BILLSDEPOSITS);
        else if (data == "item@Transaction Report")
            evt = new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, MENU_TRANSACTIONREPORT);
        if (evt)
        {
            AddPendingEvent(*evt);
            delete evt;
            return;
        }
        activeReport_ = true;
        createReportsPage(iData->get_report(), false);
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnLaunchAccountWebsite(wxCommandEvent& WXUNUSED(event))
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

void mmGUIFrame::OnAccountAttachments(wxCommandEvent& WXUNUSED(event))
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
void mmGUIFrame::OnPopupEditAccount(wxCommandEvent& WXUNUSED(event))
{
    if (selectedItemData_)
    {
        int data = selectedItemData_->getData();
        Model_Account::Data* account = Model_Account::instance().get(data);
        if (account)
        {
            mmNewAcctDialog dlg(account, this);
            if (dlg.ShowModal() == wxID_OK)
            {
                updateNavTreeControl();
                createHomePage(); //TODO: refreshPanelData(); and change selection
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

void mmGUIFrame::OnPopupAccountBaseBalance(wxCommandEvent& WXUNUSED(event))
{
    if (selectedItemData_)
    {
        int account_id = selectedItemData_->getData();
        Model_Account::Data* account = Model_Account::instance().get(account_id);
        Model_Currency::Data* acc_currency = Model_Account::currency(account);
        Model_Currency::Data* base_currency = Model_Currency::GetBaseCurrency();

        double acc_bal = Model_Account::balance(account);
        double acc_base_bal = acc_bal * Model_CurrencyHistory::getDayRate(acc_currency->CURRENCYID);

        wxString message = wxString::Format(
            _("Account: %s\n\n"
              "Balance at currency %s: %s\n"
              "Balance at currency %s: %s"),
            account->ACCOUNTNAME,
            acc_currency->CURRENCY_SYMBOL, Model_Currency::toCurrency(acc_bal, acc_currency),
            base_currency->CURRENCY_SYMBOL, Model_Currency::toCurrency(acc_base_bal));
        wxMessageBox(message, _("Foreign Currency Account Balance"));
    }
}

//----------------------------------------------------------------------------

void mmGUIFrame::OnPopupDeleteAccount(wxCommandEvent& WXUNUSED(event))
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
                updateNavTreeControl();
                createHomePage();
            }
        }
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnItemMenu(wxTreeEvent& event)
{
    wxTreeItemId selectedItem = event.GetItem();
    if (menuBar_->FindItem(MENU_ORGCATEGS)->IsEnabled())
        showTreePopupMenu(selectedItem, event.GetPoint());
    else
        wxMessageBox(_("MMEX has been opened without an active database.")
            , _("MMEX: Menu Popup Error"), wxOK | wxICON_EXCLAMATION);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnItemRightClick(wxTreeEvent& event)
{
    wxTreeItemId selectedItem = event.GetItem();
    m_nav_tree_ctrl->SelectItem(selectedItem);
}
//----------------------------------------------------------------------------

void mmGUIFrame::showTreePopupMenu(const wxTreeItemId& id, const wxPoint& pt)
{
    mmTreeItemData* iData = dynamic_cast<mmTreeItemData*>(m_nav_tree_ctrl->GetItemData(id));
    if (iData) selectedItemData_ = iData;
    else return;

    if (!iData->isStringData())
    {
        int data = iData->getData();
        if (!iData->isBudgetingNode())
        {
            Model_Account::Data* account = Model_Account::instance().get(data);
            if (account)
            {
                wxMenu menu;
                //                  menu.Append(MENU_TREEPOPUP_GOTO, _("&Go To.."));
                if (account->ACCOUNTTYPE != Model_Account::all_type()[Model_Account::INVESTMENT])
                {
                    gotoAccountID_ = account->ACCOUNTID;

                    menu.Append(MENU_TREEPOPUP_NEW, _("&New Transaction"));
                    menu.AppendSeparator();
                }
                menu.Append(MENU_TREEPOPUP_EDIT, _("&Edit Account"));
                menu.Append(MENU_TREEPOPUP_REALLOCATE, _("&Reallocate Account"));
                menu.Append(MENU_TREEPOPUP_DELETE, _("&Delete Account"));
                menu.AppendSeparator();
                menu.Append(MENU_TREEPOPUP_ACCOUNT_BASE_BALANCE, _("&Foreign Currency Balance"));
                menu.Append(MENU_TREEPOPUP_LAUNCHWEBSITE, _("&Launch Account Website"));
                // Enable menu item only if a website exists for the account.
                bool webStatus = !account->WEBSITE.IsEmpty();
                menu.Enable(MENU_TREEPOPUP_LAUNCHWEBSITE, webStatus);
                menu.Append(MENU_TREEPOPUP_ACCOUNTATTACHMENTS, _("&Organize Attachments"));

                if (Model_Account::type(account) == Model_Account::INVESTMENT)
                    menu.Enable(MENU_TREEPOPUP_REALLOCATE, false);

                PopupMenu(&menu, pt);
            }
        }
    }
    else
    {
        if (iData->getString() == "item@Budgeting")
        {
            wxCommandEvent e;
            OnBudgetSetupDialog(e);
        }
        else if (iData->getString() == "item@Reports")
        {
            wxMenu menu;
            menu.Append(wxID_VIEW_LIST, _("General Report Manager"));
            wxMenu *hideShowReport = new wxMenu;
            for (int r = 0; r < Option::instance().ReportCount(); r++)
            {
                hideShowReport->Append(MENU_TREEPOPUP_HIDE_SHOW_REPORT + r, Option::instance().ReportFullName(r), wxEmptyString, wxITEM_CHECK);
                hideShowReport->Check(MENU_TREEPOPUP_HIDE_SHOW_REPORT + r, !Option::instance().HideReport(r));
            }
            menu.AppendSubMenu(hideShowReport, _("Hide/Show Report"));
            PopupMenu(&menu, pt);
        }
        else if (iData->getString() == "item@Bank Accounts" ||
            iData->getString() == "item@Cash Accounts" ||
            iData->getString() == "item@Loan Accounts" ||
            iData->getString() == "item@Term Accounts" ||
            iData->getString() == "item@Credit Card Accounts" ||
            iData->getString() == "item@Stocks")
        {
            // Create for Account types: Bank, Cash, Loan, Credit Card, Term & Stocks 
            wxMenu menu;
            menu.Append(MENU_TREEPOPUP_ACCOUNT_NEW, _("New &Account"));
            menu.Append(MENU_TREEPOPUP_ACCOUNT_DELETE, _("&Delete Account"));
            menu.Append(MENU_TREEPOPUP_ACCOUNT_EDIT, _("&Edit Account"));
            menu.Append(MENU_TREEPOPUP_ACCOUNT_LIST, _("Account &List (Home)"));
            menu.AppendSeparator();

            // Create only for Account types: Bank, Cash, Loan & Credit Card
            if ((iData->getString() != "item@Term Accounts") && (iData->getString() != "item@Stocks"))
            {
                wxMenu *exportTo = new wxMenu;
                exportTo->Append(MENU_TREEPOPUP_ACCOUNT_EXPORT2CSV, _("&CSV Files..."));
                exportTo->Append(MENU_TREEPOPUP_ACCOUNT_EXPORT2XML, _("&XML Files..."));
                exportTo->Append(MENU_TREEPOPUP_ACCOUNT_EXPORT2QIF, _("&QIF Files..."));
                menu.AppendSubMenu(exportTo, _("&Export"));
                wxMenu *importFrom = new wxMenu;
                importFrom->Append(MENU_TREEPOPUP_ACCOUNT_IMPORTUNIVCSV, _("&CSV Files..."));
                importFrom->Append(MENU_TREEPOPUP_ACCOUNT_IMPORTXML, _("&XML Files..."), _("Import from XML (Excel format)"));
                importFrom->Append(MENU_TREEPOPUP_ACCOUNT_IMPORTQIF, _("&QIF Files..."));
                menu.AppendSubMenu(importFrom, _("&Import"));
                menu.AppendSeparator();
            }

            wxMenu *viewAccounts = new wxMenu;
            viewAccounts->Append(MENU_TREEPOPUP_ACCOUNT_VIEWALL, _("All"));
            viewAccounts->Append(MENU_TREEPOPUP_ACCOUNT_VIEWFAVORITE, _("Favorites"));
            viewAccounts->Append(MENU_TREEPOPUP_ACCOUNT_VIEWOPEN, _("Open"));
            viewAccounts->Append(MENU_TREEPOPUP_ACCOUNT_VIEWCLOSED, _("Closed"));
            menu.AppendSubMenu(viewAccounts, _("Accounts Visible"));
            PopupMenu(&menu, pt);
        }
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnViewAccountsTemporaryChange(wxCommandEvent& e)
{
    int evt_id = e.GetId();
    //Get current settings for view accounts
    const wxString vAccts = Model_Setting::instance().ViewAccounts();
    wxString temp_view = VIEW_ACCOUNTS_ALL_STR;
    //Set view ALL & Refresh Navigation Panel
    switch (evt_id) 
    {
    case MENU_TREEPOPUP_ACCOUNT_VIEWALL: temp_view = VIEW_ACCOUNTS_ALL_STR; break;
    case MENU_TREEPOPUP_ACCOUNT_VIEWFAVORITE: temp_view = VIEW_ACCOUNTS_FAVORITES_STR; break;
    case MENU_TREEPOPUP_ACCOUNT_VIEWOPEN: temp_view = VIEW_ACCOUNTS_OPEN_STR; break;
    case MENU_TREEPOPUP_ACCOUNT_VIEWCLOSED: temp_view = VIEW_ACCOUNTS_CLOSED_STR; break;
    }
    Model_Setting::instance().SetViewAccounts(temp_view);
    updateNavTreeControl();
    createHomePage();

    //Restore settings
    Model_Setting::instance().SetViewAccounts(vAccts);
}

//----------------------------------------------------------------------------

void mmGUIFrame::createBudgetingPage(int budgetYearID)
{
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String("Budget Panel");
    
    json_writer.Key("start");
    json_writer.String(wxDateTime::Now().FormatISOCombined().c_str());

    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);
    if (panelCurrent_->GetId() == mmID_BUDGET)
    {
        budgetingPage_->DisplayBudgetingDetails(budgetYearID);
    }
    else
    {
        windowsFreezeThaw(homePanel_);
        wxSizer *sizer = cleanupHomePanel();

        budgetingPage_ = new mmBudgetingPanel(budgetYearID
            , homePanel_, mmID_BUDGET, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
        panelCurrent_ = budgetingPage_;

        sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
        homePanel_->Layout();
        windowsFreezeThaw(homePanel_);
    }

    json_writer.Key("end");
    json_writer.String(wxDateTime::Now().FormatISOCombined().c_str());
    json_writer.EndObject();

    Model_Usage::instance().AppendToUsage(json_buffer.GetString());

    menuPrintingEnable(true);
    m_nav_tree_ctrl->SetEvtHandlerEnabled(true);
}
//----------------------------------------------------------------------------

void mmGUIFrame::createHomePage()
{
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String("Home Page");
    
    json_writer.Key("start");
    json_writer.String(wxDateTime::Now().FormatISOCombined().c_str());

    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);
    int id = panelCurrent_ ? panelCurrent_->GetId() : -1;
    /* Update home page details only if it is being displayed */
    if (id == mmID_HOMEPAGE)
    {
        homePage_->createHTML();
    }
    else
    {
        windowsFreezeThaw(homePanel_);
        wxSizer *sizer = cleanupHomePanel();
        homePage_ = new mmHomePagePanel(homePanel_
            , this, mmID_HOMEPAGE
            , wxDefaultPosition, wxDefaultSize
            , wxNO_BORDER | wxTAB_TRAVERSAL
        );
        panelCurrent_ = homePage_;
        sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
        homePanel_->Layout();
        windowsFreezeThaw(homePanel_);
    }
    if (m_nav_tree_ctrl->GetRootItem().IsOk())
        m_nav_tree_ctrl->SelectItem(m_nav_tree_ctrl->GetRootItem());
    m_nav_tree_ctrl->SetEvtHandlerEnabled(true);

    json_writer.Key("end");
    json_writer.String(wxDateTime::Now().FormatISOCombined().c_str());
    json_writer.EndObject();

    const auto  j = json_buffer.GetString();
    Model_Usage::instance().AppendToUsage(j);
}
//----------------------------------------------------------------------------

void mmGUIFrame::createReportsPage(mmPrintableBase* rs, bool cleanup)
{
    if (!rs) return;
    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);

    windowsFreezeThaw(homePanel_);
    wxSizer *sizer = cleanupHomePanel();
    panelCurrent_ = new mmReportsPanel(rs
        , cleanup, homePanel_, this, mmID_REPORTS
        , wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);

    sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
    homePanel_->Layout();
    windowsFreezeThaw(homePanel_);

    menuPrintingEnable(true);
    m_nav_tree_ctrl->SetEvtHandlerEnabled(true);
}
//----------------------------------------------------------------------------

void mmGUIFrame::createHelpPage()
{
    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);
    windowsFreezeThaw(homePanel_);
    wxSizer *sizer = cleanupHomePanel();
    panelCurrent_ = new mmHelpPanel(homePanel_, this, wxID_HELP
        , wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
    sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
    homePanel_->Layout();
    windowsFreezeThaw(homePanel_);
    menuPrintingEnable(true);
    m_nav_tree_ctrl->SetEvtHandlerEnabled(true);
}
//----------------------------------------------------------------------------

void mmGUIFrame::createMenu()
{
    wxMenu *menu_file = new wxMenu;

    wxMenuItem* menuItemNew = new wxMenuItem(menu_file, MENU_NEW, _("&New Database\tCtrl-N"), _("New Database"));
    menuItemNew->SetBitmap(mmBitmap(png::NEW_DB));
    wxMenuItem* menuItemOpen = new wxMenuItem(menu_file, MENU_OPEN, _("&Open Database\tCtrl-O"), _("Open Database"));
    menuItemOpen->SetBitmap(mmBitmap(png::OPEN));
    wxMenuItem* menuItemSaveAs = new wxMenuItem(menu_file, MENU_SAVE_AS, _("Save Database &As"), _("Save Database As"));
    menuItemSaveAs->SetBitmap(mmBitmap(png::SAVEAS));
    menu_file->Append(menuItemNew);
    menu_file->Append(menuItemOpen);
    menu_file->Append(menuItemSaveAs);
    menu_file->AppendSeparator();

    m_menuRecentFiles = new wxMenu;
    menu_file->Append(MENU_RECENT_FILES, _("&Recent Files..."), m_menuRecentFiles);
    wxMenuItem* menuClearRecentFiles = new wxMenuItem(menu_file, MENU_RECENT_FILES_CLEAR, _("&Clear Recent Files"));
    menuClearRecentFiles->SetBitmap(mmBitmap(png::CLEARLIST));
    menu_file->Append(menuClearRecentFiles);
    menu_file->AppendSeparator();

    wxMenu* exportMenu = new wxMenu;
    exportMenu->Append(MENU_EXPORT_CSV, _("&CSV Files..."), _("Export to CSV"));
    exportMenu->Append(MENU_EXPORT_XML, _("&XML Files..."), _("Export to XML"));
    exportMenu->Append(MENU_EXPORT_QIF, _("&QIF Files..."), _("Export to QIF"));
    exportMenu->Append(MENU_EXPORT_HTML, _("&Report to HTML"), _("Export to HTML"));
    exportMenu->Append(MENU_EXPORT_WEBAPP, _("&Force WebApp sync"), _("Force sync of accounts, payees and categories to WebApp"));
    menu_file->Append(MENU_EXPORT, _("&Export"), exportMenu);

    wxMenu* importMenu = new wxMenu;
    importMenu->Append(MENU_IMPORT_UNIVCSV, _("&CSV Files..."), _("Import from any CSV file"));
    importMenu->Append(MENU_IMPORT_XML, _("&XML Files..."), _("Import from XML (Excel format)"));
    importMenu->Append(MENU_IMPORT_QIF, _("&QIF Files..."), _("Import from QIF"));
    importMenu->Append(MENU_IMPORT_WEBAPP, _("&WebApp..."), _("Import from WebApp"));
    menu_file->Append(MENU_IMPORT, _("&Import"), importMenu);

    menu_file->AppendSeparator();

    wxMenuItem* menuItemPrint = new wxMenuItem(menu_file, wxID_PRINT,
        _("&Print..."), _("Print current view"));
    menuItemPrint->SetBitmap(mmBitmap(png::PRINT));
    menu_file->Append(menuItemPrint);

    menu_file->AppendSeparator();
    wxMenuItem* menuItemQuit = new wxMenuItem(menu_file, wxID_EXIT);
    menuItemQuit->SetBitmap(mmBitmap(png::EXIT));
    menu_file->Append(menuItemQuit);

    // Create the required menu items
    wxMenu *menuView = new wxMenu;
    wxMenuItem* menuItemToolbar = new wxMenuItem(menuView, MENU_VIEW_TOOLBAR,
        _("&Toolbar"), _("Show/Hide the toolbar"), wxITEM_CHECK);
    wxMenuItem* menuItemLinks = new wxMenuItem(menuView, MENU_VIEW_LINKS,
        _("&Navigation"), _("Show/Hide the Navigation tree control"), wxITEM_CHECK);
    wxMenuItem* menuItemHideShareAccounts = new wxMenuItem(menuView, MENU_VIEW_HIDE_SHARE_ACCOUNTS,
        _("&Display Share Accounts"), _("Show/Hide Share Accounts in the navigation tree"), wxITEM_CHECK);

    wxMenuItem* menuItemBudgetFinancialYears = new wxMenuItem(menuView, MENU_VIEW_BUDGET_FINANCIAL_YEARS,
        _("Budgets: As &Financial Years"), _("Display Budgets in Financial Year Format"), wxITEM_CHECK);
    wxMenuItem* menuItemBudgetTransferTotal = new wxMenuItem(menuView, MENU_VIEW_BUDGET_TRANSFER_TOTAL,
        _("Budgets: &Include Transfers in Totals"), _("Include the transfer transactions in the Budget Totals"), wxITEM_CHECK);
    wxMenuItem* menuItemBudgetSetupWithoutSummary = new wxMenuItem(menuView, MENU_VIEW_BUDGET_SETUP_SUMMARY,
        _("Budget Setup: &Without Summaries"), _("Display the Budget Setup without category summaries"), wxITEM_CHECK);
    wxMenuItem* menuItemBudgetCategorySummary = new wxMenuItem(menuView, MENU_VIEW_BUDGET_CATEGORY_SUMMARY,
        _("Budget Category Report: with &Summaries"), _("Include the category summaries in the Budget Category Summary"), wxITEM_CHECK);
    wxMenuItem* menuItemIgnoreFutureTransactions = new wxMenuItem(menuView, MENU_VIEW_IGNORE_FUTURE_TRANSACTIONS,
        _("Ignore F&uture Transactions"), _("Ignore Future transactions"), wxITEM_CHECK);
    //Add the menu items to the menu bar
    menuView->Append(menuItemToolbar);
    menuView->Append(menuItemLinks);
    menuView->Append(menuItemHideShareAccounts);
    menuView->AppendSeparator();
    menuView->Append(menuItemBudgetFinancialYears);
    menuView->Append(menuItemBudgetTransferTotal);
    menuView->AppendSeparator();
    menuView->Append(menuItemBudgetSetupWithoutSummary);
    menuView->Append(menuItemBudgetCategorySummary);
    menuView->AppendSeparator();
    menuView->Append(menuItemIgnoreFutureTransactions);
#if (wxMAJOR_VERSION >= 3 && wxMINOR_VERSION >= 0)
    wxMenuItem* menuItemToggleFullscreen = new wxMenuItem(menuView, MENU_VIEW_TOGGLE_FULLSCREEN
        , _("Toggle Fullscreen\tF11"), _("Toggle Fullscreen"));
    menuItemToggleFullscreen->SetBitmap(mmBitmap(png::FULLSCREEN));
    menuView->AppendSeparator();
    menuView->Append(menuItemToggleFullscreen);
#endif
    menuView->AppendSeparator();

    wxMenuItem* menuItemLanguage = new wxMenuItem(menuView, MENU_LANG
        , _("Switch Application Language")
        , _("Change language used for MMEX GUI"));
    menuItemLanguage->SetBitmap(mmBitmap(png::LANG));
    wxMenu *menuLang = new wxMenu;

    wxArrayString lang_files = wxTranslations::Get()->GetAvailableTranslations("mmex");
    std::map<wxString, std::pair<int, wxString>> langs;
    menuLang->AppendRadioItem(MENU_LANG+1+wxLANGUAGE_DEFAULT, _("system default"))
        ->Check(m_app->getGUILanguage()==wxLANGUAGE_DEFAULT);
    for (auto & file : lang_files)
    {
        const wxLanguageInfo* info = wxLocale::FindLanguageInfo(file);
        if (info)
            langs[info->Description]=std::make_pair(info->Language,info->CanonicalName);
    }
    langs[wxLocale::GetLanguageName(wxLANGUAGE_ENGLISH_US)]=std::make_pair(wxLANGUAGE_ENGLISH_US,"en_US");
    for (auto const& lang : langs)
    {
        menuLang->AppendRadioItem(MENU_LANG + 1 + lang.second.first, lang.first, lang.second.second)
            ->Check(lang.second.first == m_app->getGUILanguage());
    }
    menuItemLanguage->SetSubMenu(menuLang);
    menuView->Append(menuItemLanguage);

    wxMenu *hideShowReport = new wxMenu;
    for (int r = 0; r < Option::instance().ReportCount(); r++)
    {
        hideShowReport->Append(MENU_TREEPOPUP_HIDE_SHOW_REPORT + r, Option::instance().ReportFullName(r), wxEmptyString, wxITEM_CHECK);
        hideShowReport->Check(MENU_TREEPOPUP_HIDE_SHOW_REPORT + r, !Option::instance().HideReport(r));
    }
    menuView->AppendSubMenu(hideShowReport, _("Hide/Show Report"));
    wxMenu *menuAccounts = new wxMenu;

    wxMenuItem* menuItemAcctList = new wxMenuItem(menuAccounts, MENU_ACCTLIST
        , _("Account &List"), _("Show Account List"));
    menuItemAcctList->SetBitmap(mmBitmap(png::HOME));

    wxMenuItem* menuItemNewAcct = new wxMenuItem(menuAccounts, MENU_NEWACCT
        , _("New &Account"), _("New Account"));
    menuItemNewAcct->SetBitmap(mmBitmap(png::NEW_ACC));

    wxMenuItem* menuItemAcctEdit = new wxMenuItem(menuAccounts, MENU_ACCTEDIT
        , _("&Edit Account"), _("Edit Account"));
    menuItemAcctEdit->SetBitmap(mmBitmap(png::EDIT_ACC));

    wxMenuItem* menuItemReallocateAcct = new wxMenuItem(menuAccounts, MENU_ACCOUNT_REALLOCATE
        , _("&Reallocate Account"), _("Change the account type of an account."));
    menuItemReallocateAcct->SetBitmap(mmBitmap(png::REALLOCATE_ACC));

    wxMenuItem* menuItemAcctDelete = new wxMenuItem(menuAccounts, MENU_ACCTDELETE
        , _("&Delete Account"), _("Delete Account from database"));
    menuItemAcctDelete->SetBitmap(mmBitmap(png::DELETE_ACC));

    menuAccounts->Append(menuItemAcctList);
    menuAccounts->AppendSeparator();
    menuAccounts->Append(menuItemNewAcct);
    menuAccounts->Append(menuItemAcctEdit);
    menuAccounts->Append(menuItemReallocateAcct);
    menuAccounts->Append(menuItemAcctDelete);

    // Tools Menu
    wxMenu *menuTools = new wxMenu;

    wxMenuItem* menuItemCateg = new wxMenuItem(menuTools
        , MENU_ORGCATEGS, _("Organize &Categories..."), _("Organize Categories"));
    menuItemCateg->SetBitmap(mmBitmap(png::CATEGORY));
    menuTools->Append(menuItemCateg);

    wxMenuItem* menuItemPayee = new wxMenuItem(menuTools
        , MENU_ORGPAYEE, _("Organize &Payees..."), _("Organize Payees"));
    menuItemPayee->SetBitmap(mmBitmap(png::PAYEE));
    menuTools->Append(menuItemPayee);

    wxMenuItem* menuItemCurrency = new wxMenuItem(menuTools, MENU_CURRENCY
        , _("Organize Currency..."), _("Organize Currency"));
    menuItemCurrency->SetBitmap(mmBitmap(png::CURR));
    menuTools->Append(menuItemCurrency);

    wxMenuItem* menuItemCategoryRelocation = new wxMenuItem(menuTools
        , MENU_CATEGORY_RELOCATION, _("&Categories...")
        , _("Reassign all categories to another category"));
    menuItemCategoryRelocation->SetBitmap(mmBitmap(png::CATEGORY_RELOCATION));
    wxMenuItem* menuItemPayeeRelocation = new wxMenuItem(menuTools
        , MENU_PAYEE_RELOCATION, _("&Payees...")
        , _("Reassign all payees to another payee"));
    menuItemPayeeRelocation->SetBitmap(mmBitmap(png::PAYEE_RELOCATION));
    wxMenuItem* menuItemRelocation = new wxMenuItem(menuTools
        , MENU_RELOCATION, _("Relocation of...")
        , _("Relocate Categories && Payees"));
    menuItemRelocation->SetBitmap(mmBitmap(png::RELOCATION));
    wxMenu *menuRelocation = new wxMenu;
    menuRelocation->Append(menuItemCategoryRelocation);
    menuRelocation->Append(menuItemPayeeRelocation);
    menuItemRelocation->SetSubMenu(menuRelocation);
    menuTools->Append(menuItemRelocation);

    menuTools->AppendSeparator();

    wxMenuItem* menuItemBudgeting = new wxMenuItem(menuTools, MENU_BUDGETSETUPDIALOG
        , _("&Budget Setup"), _("Budget Setup"));
    menuItemBudgeting->SetBitmap(mmBitmap(png::BUDGET));
    menuTools->Append(menuItemBudgeting);

    wxMenuItem* menuItemBillsDeposits = new wxMenuItem(menuTools, MENU_BILLSDEPOSITS
        , _("&Recurring Transactions"), _("Bills && Deposits"));
    menuItemBillsDeposits->SetBitmap(mmBitmap(png::RECURRING));
    menuTools->Append(menuItemBillsDeposits);

    wxMenuItem* menuItemAssets = new wxMenuItem(menuTools, MENU_ASSETS
        , _("&Assets"), _("Assets"));
    menuItemAssets->SetBitmap(mmBitmap(png::ASSET));
    menuTools->Append(menuItemAssets);

    wxMenuItem* menuItemTransactions = new wxMenuItem(menuTools, MENU_TRANSACTIONREPORT
        , _("&Transaction Report Filter..."), _("Transaction Report Filter"));
    menuItemTransactions->SetBitmap(mmBitmap(png::FILTER));
    menuTools->Append(menuItemTransactions);

    menuTools->AppendSeparator();

    wxMenuItem* menuItemGRM = new wxMenuItem(menuTools, wxID_VIEW_LIST
        , _("&General Report Manager..."), _("General Report Manager"));
    menuItemGRM->SetBitmap(mmBitmap(png::GRM));
    menuTools->Append(menuItemGRM);

    wxMenuItem* menuItemCF = new wxMenuItem(menuTools, wxID_BROWSE
        , _("&Custom Fields Manager..."), _("Custom Fields Manager"));
    menuItemCF->SetBitmap(mmBitmap(png::CUSTOM));
    menuTools->Append(menuItemCF);

    menuTools->AppendSeparator();

    wxMenuItem* menuItemOptions = new wxMenuItem(menuTools, wxID_PREFERENCES
        , _("&Options..."), _("Show the Options Dialog"));
    menuItemOptions->SetBitmap(mmBitmap(png::OPTIONS));
    menuTools->Append(menuItemOptions);

    menuTools->AppendSeparator();

    wxMenu *menuDatabase = new wxMenu;
    wxMenuItem* menuItemSetPassword = new wxMenuItem(menuTools, MENU_SET_PASSWORD
        , _("Set &Password")
        , _("Encrypt database or change password for database file"));
    menuItemSetPassword->SetBitmap(mmBitmap(png::ENCRYPT_DB_EDIT));
    wxMenuItem* menuItemRemovePassword = new wxMenuItem(menuTools, MENU_REMOVE_PASSWORD
        , _("Remove Password")
        , _("Remove password and decrypt database file"));
    menuItemRemovePassword->SetBitmap(mmBitmap(png::ENCRYPT_DB));
    wxMenuItem* menuItemVacuumDB = new wxMenuItem(menuTools, MENU_DB_VACUUM
        , _("Optimize &Database")
        , _("Optimize database space and performance"));
    menuItemVacuumDB->SetBitmap(mmBitmap(png::EMPTY));
    wxMenuItem* menuItemCheckDB = new wxMenuItem(menuTools, MENU_DB_DEBUG
        , _("Database Debug")
        , _("Generate database report or fix errors"));
    menuItemCheckDB->SetBitmap(mmBitmap(png::EMPTY));
    menuDatabase->Append(menuItemSetPassword);
    menuDatabase->Append(menuItemRemovePassword);
    menuDatabase->Append(menuItemVacuumDB);
    menuDatabase->Append(menuItemCheckDB);
    menuTools->AppendSubMenu(menuDatabase, _("Database")
        , _("Database management"));
    menuItemRemovePassword->Enable(false);

    // Help Menu
    wxMenu *menuHelp = new wxMenu;

    wxMenuItem* menuItemHelp = new wxMenuItem(menuHelp, wxID_HELP,
        _("&Help\tF1"), _("Read the User Manual"));
    menuItemHelp->SetBitmap(mmBitmap(png::HELP));
    menuHelp->Append(menuItemHelp);
    //Community Submenu
    wxMenuItem* menuItemWebsite = new wxMenuItem(menuHelp, MENU_WEBSITE
        , _("Website")
        , _("Open the Money Manager EX website for latest news, updates etc"));
    // menuItemFacebook->SetBitmap(mmBitmap(png::WEBSITE));
    wxMenuItem* menuItemFacebook = new wxMenuItem(menuHelp, MENU_FACEBOOK
        , _("Facebook"), _("Visit us on Facebook"));
    menuItemFacebook->SetBitmap(mmBitmap(png::FACEBOOK));
    wxMenuItem* menuItemTwitter = new wxMenuItem(menuHelp, MENU_TWITTER
        , _("Twitter"), _("Follow us on Twitter"));
    // menuItemTwitter->SetBitmap(mmBitmap(png::TWITTER));
    wxMenuItem* menuItemYouTube = new wxMenuItem(menuHelp, MENU_YOUTUBE
        , _("YouTube"), _("Watch free video materials about MMEX"));
    // menuItemYouTube->SetBitmap(mmBitmap(png::YOUTUBE));
    wxMenuItem* menuItemSlack = new wxMenuItem(menuHelp, MENU_SLACK
        , _("Slack"), _("Communicate online with MMEX team from your desktop or mobile device"));
    menuItemSlack->SetBitmap(mmBitmap(png::SLACK));
    wxMenuItem* menuItemGitHub = new wxMenuItem(menuHelp, MENU_GITHUB
        , _("GitHub"), _("Access open source code repository and track reported bug statuses"));
    menuItemGitHub->SetBitmap(mmBitmap(png::GITHUB));
    wxMenuItem* menuItemWiki = new wxMenuItem(menuHelp, MENU_WIKI
        , _("Wiki pages"), _("Read and update wiki pages"));
    // menuItemWiki->SetBitmap(mmBitmap(png::WIKI));
    wxMenuItem* menuItemReportIssues = new wxMenuItem(menuHelp, MENU_REPORTISSUES
        , _("Forum")
        , _("Visit the MMEX forum to see existing user comments or report new issues with the software"));
    menuItemReportIssues->SetBitmap(mmBitmap(png::FORUM));
    wxMenuItem* menuItemGooglePlay = new wxMenuItem(menuHelp, MENU_GOOGLEPLAY
        , _("Google Play")
        , _("Get free Android version and run MMEX on your smart phone or tablet"));
    menuItemGooglePlay->SetBitmap(mmBitmap(png::GOOGLE_PLAY));
    wxMenuItem* menuItemNotify = new wxMenuItem(menuHelp, MENU_ANNOUNCEMENTMAILING
        , _("&Newsletter")
        , _("Subscribe to e-mail newsletter or view existing announcements"));
    menuItemNotify->SetBitmap(mmBitmap(png::NEWS));
    wxMenuItem* menuItemRSS = new wxMenuItem(menuHelp, MENU_RSS
        , _("RSS Feed"), _("Connect RSS web feed to news aggregator"));
    menuItemRSS->SetBitmap(mmBitmap(png::NEWS));
    wxMenuItem* menuItemDonate = new wxMenuItem(menuHelp, MENU_DONATE
        , _("Donate via PayPal")
        , _("Donate the team to support infrastructure etc"));
    menuItemDonate->SetBitmap(mmBitmap(png::PP));
    wxMenuItem* menuItemBuyCoffee = new wxMenuItem(menuHelp, MENU_BUY_COFFEE
        , _("Buy us a Coffee")
        , _("Buy key developer a coffee"));
    // menuItemDonate->SetBitmap(mmBitmap(png::COFFEE));

    wxMenuItem* menuItemCommunity = new wxMenuItem(menuHelp, MENU_COMMUNITY
        , _("Community")
        , _("Stay in touch with MMEX community"));
    menuItemCommunity->SetBitmap(mmBitmap(png::COMMUNITY));
    wxMenu *menuCommunity = new wxMenu;
    menuCommunity->Append(menuItemWebsite);
    menuCommunity->Append(menuItemReportIssues);
    menuCommunity->Append(menuItemWiki);
    menuCommunity->Append(menuItemGitHub);
    menuCommunity->Append(menuItemSlack);
    menuCommunity->Append(menuItemFacebook);
    menuCommunity->Append(menuItemTwitter);
    menuCommunity->Append(menuItemYouTube);
    menuCommunity->Append(menuItemGooglePlay);
    menuCommunity->Append(menuItemNotify);
    menuCommunity->Append(menuItemRSS);
    menuCommunity->Append(menuItemDonate);
    menuCommunity->Append(menuItemBuyCoffee);
    menuItemCommunity->SetSubMenu(menuCommunity);
    menuHelp->Append(menuItemCommunity);

    wxMenuItem* menuItemReportBug = new wxMenuItem(menuHelp, MENU_REPORT_BUG
        , _("Report a Bug")
        , _("Report an error in application to the developers"));
    menuItemReportBug->SetBitmap(mmBitmap(png::BUG));
    menuHelp->Append(menuItemReportBug);

    wxMenuItem* menuItemAppStart = new wxMenuItem(menuHelp, MENU_SHOW_APPSTART
        , _("Reopen &Start-up Dialog"), _("Show application start-up dialog"));
    menuItemAppStart->SetBitmap(mmBitmap(png::APPSTART));
    menuHelp->Append(menuItemAppStart);

    wxMenuItem* menuItemCheck = new wxMenuItem(menuHelp, MENU_CHECKUPDATE
        , _("Check for &Updates"), _("Check if a new MMEX version is available"));
    menuItemCheck->SetBitmap(mmBitmap(png::UPDATE));
    menuHelp->Append(menuItemCheck);

    wxMenuItem* menuItemAbout = new wxMenuItem(menuHelp, wxID_ABOUT
        , _("&About..."), _("Show about dialog"));
    menuItemAbout->SetBitmap(mmBitmap(png::ABOUT));
    menuHelp->Append(menuItemAbout);

    menuBar_ = new wxMenuBar;
    menuBar_->Append(menu_file, _("&File"));
    menuBar_->Append(menuAccounts, _("&Accounts"));
    menuBar_->Append(menuTools, _("&Tools"));
    menuBar_->Append(menuView, _("&View"));
    menuBar_->Append(menuHelp, _("&Help"));

    SetMenuBar(menuBar_);

    menuBar_->Check(MENU_VIEW_BUDGET_FINANCIAL_YEARS, Option::instance().BudgetFinancialYears());
    menuBar_->Check(MENU_VIEW_BUDGET_TRANSFER_TOTAL, Option::instance().BudgetIncludeTransfers());
    menuBar_->Check(MENU_VIEW_BUDGET_SETUP_SUMMARY, Option::instance().BudgetSetupWithoutSummaries());
    menuBar_->Check(MENU_VIEW_BUDGET_CATEGORY_SUMMARY, Option::instance().BudgetReportWithSummaries());
    menuBar_->Check(MENU_VIEW_IGNORE_FUTURE_TRANSACTIONS, Option::instance().IgnoreFutureTransactions());
}
//----------------------------------------------------------------------------

void mmGUIFrame::createToolBar()
{
    long style = wxTB_FLAT | wxTB_NODIVIDER;

    toolBar_ = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, style, "ToolBar");

    toolBar_->AddTool(MENU_NEW, _("New"), mmBitmap(png::NEW_DB), _("New Database"));
    toolBar_->AddTool(MENU_OPEN, _("Open"), mmBitmap(png::OPEN), _("Open Database"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(MENU_NEWACCT, _("New Account"), mmBitmap(png::NEW_ACC), _("New Account"));
    toolBar_->AddTool(MENU_ACCTLIST, _("Home Page"), mmBitmap(png::HOME), _("Show Home Page"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(MENU_ORGCATEGS, _("Organize Categories"), mmBitmap(png::CATEGORY), _("Show Organize Categories Dialog"));
    toolBar_->AddTool(MENU_ORGPAYEE, _("Organize Payees"), mmBitmap(png::PAYEE), _("Show Organize Payees Dialog"));
    toolBar_->AddTool(MENU_CURRENCY, _("Organize Currency"), mmBitmap(png::CURR), _("Show Organize Currency Dialog"));
    toolBar_->AddTool(wxID_VIEW_LIST, _("General Report Manager"), mmBitmap(png::GRM), _("General Report Manager"));
    toolBar_->AddTool(wxID_BROWSE, _("Custom Fields Manager"), mmBitmap(png::CUSTOM), _("Custom Fields Manager"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(MENU_TRANSACTIONREPORT, _("Transaction Report Filter"), mmBitmap(png::FILTER), _("Transaction Report Filter"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(wxID_PREFERENCES, _("&Options..."), mmBitmap(png::OPTIONS), _("Show the Options Dialog"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(wxID_NEW, _("New"), mmBitmap(png::NEW_TRX), _("New Transaction"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(wxID_ABOUT, _("&About..."), mmBitmap(png::ABOUT), _("Show about dialog"));
    toolBar_->AddTool(wxID_HELP, _("&Help\tF1"), mmBitmap(png::HELP), _("Show the Help file"));
    
    wxString news_array;
    for (const auto& entry : g_WebsiteNewsList)
        news_array += entry.Title + "\n";
    if (news_array.empty()) news_array = _("Register/View Release &Notifications");
    const wxBitmap news_ico = (g_WebsiteNewsList.size() > 0) 
        ? mmBitmap(png::NEW_NEWS) 
        : mmBitmap(png::NEWS);
    toolBar_->AddSeparator();
    toolBar_->AddTool(MENU_ANNOUNCEMENTMAILING, _("News"), news_ico, news_array);

    toolBar_->AddSeparator();
    toolBar_->AddTool(MENU_VIEW_TOGGLE_FULLSCREEN, _("Toggle Fullscreen\tF11"), mmBitmap(png::FULLSCREEN), _("Toggle Fullscreen"));

    toolBar_->AddSeparator();
    toolBar_->AddTool(wxID_PRINT, _("&Print..."), mmBitmap(png::PRINT), _("Print current view"));

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
    m_all_models.push_back(&Model_Subcategory::instance(m_db.get())); // subcategory must be initialized before category
    m_all_models.push_back(&Model_Category::instance(m_db.get()));
    m_all_models.push_back(&Model_Billsdeposits::instance(m_db.get()));
    m_all_models.push_back(&Model_Splittransaction::instance(m_db.get()));
    m_all_models.push_back(&Model_Budgetsplittransaction::instance(m_db.get()));
    m_all_models.push_back(&Model_Budget::instance(m_db.get()));
    m_all_models.push_back(&Model_Report::instance(m_db.get()));
    m_all_models.push_back(&Model_Attachment::instance(m_db.get()));
    m_all_models.push_back(&Model_CustomFieldData::instance(m_db.get()));
    m_all_models.push_back(&Model_CustomField::instance(m_db.get()));
    m_all_models.push_back(&Model_Translink::instance(m_db.get()));
    m_all_models.push_back(&Model_Shareinfo::instance(m_db.get()));
}

bool mmGUIFrame::createDataStore(const wxString& fileName, const bool openingNew, const bool encrypt, const wxString& pwd)
{
    const wxFileName checkFile(fileName);

    if (!openingNew && !checkFile.FileExists()) // Nothing to open
    {
        wxMessageBox(_("Cannot locate database file to open.")
            + "\n" + fileName,
            _("Opening MMEX Database - Error"), wxOK | wxICON_ERROR);
        return false;
    }

    if (m_db)
    {
        this->SetTitle(mmex::getCaption(mmex::isPortableMode() ? _("[portable mode]") : ""));
        resetNavTreeControl();
        cleanupHomePanel(false);
        menuEnableItems(false);
        menuPrintingEnable(false);

        ShutdownDatabase();
        /// Backup the database according to user requirements
        if (Option::instance().DatabaseUpdated() &&
            Model_Setting::instance().GetBoolSetting("BACKUPDB_UPDATE", false))
        {
            dbUpgrade::BackupDB(m_filename, dbUpgrade::BACKUPTYPE::CLOSE, Model_Setting::instance().GetIntSetting("MAX_BACKUP_FILES", 4));
            Option::instance().DatabaseUpdated(false);
        }
    }

    if (!openingNew) // Existing Database
    {
        /* Do a backup before opening */
        if (Model_Setting::instance().GetBoolSetting("BACKUPDB", false))
        {
            dbUpgrade::BackupDB(fileName, dbUpgrade::BACKUPTYPE::START, Model_Setting::instance().GetIntSetting("MAX_BACKUP_FILES", 4));
        }

        m_db = mmDBWrapper::Open(fileName, pwd);
        // if the database pointer has been reset, the password is possibly incorrect
        if (!m_db) return false;

        m_commit_callback_hook = new CommitCallbackHook();
        m_db->SetCommitHook(m_commit_callback_hook);
        m_update_callback_hook = new UpdateCallbackHook();
        m_db->SetUpdateHook(m_update_callback_hook);

        //Check if DB upgrade needed
        if (dbUpgrade::CheckUpgradeDB(m_db.get()))
        {
            //DB backup is handled inside UpgradeDB
            if (!dbUpgrade::UpgradeDB(m_db.get(), fileName))
            {
                int response = wxMessageBox(_("Have MMEX support provided you a debug/patch file?"), _("MMEX upgrade"), wxYES_NO);
                if (response == wxYES)
                    dbUpgrade::SqlFileDebug(m_db.get());
                ShutdownDatabase();
                return false;
            }
        }

        InitializeModelTables();
    }
    else // New Database
    {
        wxString password(pwd);
        if (encrypt && password.IsEmpty())
        {
            password = readPasswordFromUser(openingNew);
            if (password.IsEmpty())
                return false;
        }

        // Remove file so we can replace it instead of opening it
        if (checkFile.FileExists())
            wxRemoveFile(fileName);

        m_db = mmDBWrapper::Open(fileName, encrypt ? password : "");
        m_commit_callback_hook = new CommitCallbackHook();
        m_db->SetCommitHook(m_commit_callback_hook);
        m_update_callback_hook = new UpdateCallbackHook();
        m_db->SetUpdateHook(m_update_callback_hook);

        dbUpgrade::InitializeVersion(m_db.get());
        InitializeModelTables();

        mmNewDatabaseWizard* wizard = new mmNewDatabaseWizard(this);
        wizard->CenterOnParent();
        wizard->RunIt(true);
        wxButton* next = (wxButton*) wizard->FindWindow(wxID_FORWARD); //FIXME: 
        if (next) next->SetLabel(_("&Next ->"));

        SetDataBaseParameters(fileName);
        /* Jump to new account creation screen */
        wxCommandEvent evt;
        OnNewAccount(evt);
        return true;
    }

    SetDataBaseParameters(fileName);

    return true;
}
//----------------------------------------------------------------------------

void mmGUIFrame::SetDataBaseParameters(const wxString& fileName)
{
    SetTitle(mmex::getCaption(fileName +
        (mmex::isPortableMode() ? " " + _("[portable mode]") : "")));

    if (m_db)
    {
        m_filename = fileName;
        /* Set InfoTable Options into memory */
        Option::instance().LoadOptions();
    }
    else
    {
        m_filename.Clear();
    }
}
//----------------------------------------------------------------------------

bool mmGUIFrame::openFile(const wxString& fileName, bool openingNew, bool encrypt, const wxString &password)
{
    menuBar_->FindItem(MENU_REMOVE_PASSWORD)->Enable(false);
    if (createDataStore(fileName, openingNew, encrypt, password)) {
        m_recentFiles->AddFileToHistory(fileName);
        menuEnableItems(true);
        menuPrintingEnable(false);
        autoRepeatTransactionsTimer_.Start(REPEAT_TRANS_DELAY_TIME, wxTIMER_ONE_SHOT);

        if (m_db->IsEncrypted()) {
            menuBar_->FindItem(MENU_REMOVE_PASSWORD)->Enable(true);
        }
    }
    else {
        return false;
    }

    return true;
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnNew(wxCommandEvent& WXUNUSED(event))
{
    autoRepeatTransactionsTimer_.Stop();
    wxFileDialog dlg(this,
        _("Choose database file to create"),
        wxEmptyString,
        wxEmptyString,
        _("MMEX database files (*.mmb)") + "|*.mmb"
            + (wxSQLite3Database::HasEncryptionSupport()
                ? "|" + _("Encrypted MMEX database files (*.mmb)") + "|*.mmb"
                : wxString()),
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT
        );

    if (dlg.ShowModal() != wxID_OK)
        return;

    const bool encrypted = dlg.GetFilterIndex() != 0; // Encrypted selected
    wxFileName newFileName(dlg.GetPath());
    newFileName.SetExt("mmb");

    SetDatabaseFile(newFileName.GetFullPath(), true, encrypted);
    Model_Setting::instance().Set("LASTFILENAME", newFileName.GetFullPath());
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnOpen(wxCommandEvent& WXUNUSED(event))
{
    autoRepeatTransactionsTimer_.Stop();
    wxString fileName = wxFileSelector(_("Choose database file to open")
        , wxEmptyString, wxEmptyString, wxEmptyString
        , _("MMEX database files (*.mmb)") + "|*.mmb;*.emb"
        , wxFD_FILE_MUST_EXIST | wxFD_OPEN
        , this
        );

    if (!fileName.empty())
    {
        SetDatabaseFile(fileName);
        saveSettings();
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnSetPassword(wxCommandEvent& WXUNUSED(event))
{
    wxString new_password = readPasswordFromUser(true);
    if (!new_password.IsEmpty())
    {
        m_db->ReKey(new_password);
        menuBar_->FindItem(MENU_REMOVE_PASSWORD)->Enable(true);
        wxMessageBox(_("Password change completed."), _("Encryption Password Change"));
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnRemovePassword(wxCommandEvent& WXUNUSED(event))
{
    int ans = wxMessageBox(
        _("Please confirm database decryption and password removal."),
        _("Encryption Password Removal"),
        wxOK | wxCANCEL | wxCANCEL_DEFAULT | wxICON_AUTH_NEEDED | wxICON_QUESTION);

    if (ans==wxOK)
    {
        m_db->ReKey(wxEmptyString);
        menuBar_->FindItem(MENU_REMOVE_PASSWORD)->Enable(false);
        wxMessageBox(_("Password removed and database decrypted."), _("Encryption Password Removal"));
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnVacuumDB(wxCommandEvent& WXUNUSED(event))
{
    wxMessageDialog msgDlg(this
        , wxString::Format("%s\n\n%s",_("Make sure you have a backup of DB before optimize it"),_("Do you want to proceed?"))
        , _("DB Optimization"), wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
    if (msgDlg.ShowModal() == wxID_YES)
    {
        const wxString SizeBefore = wxFileName(m_filename).GetHumanReadableSize();
        m_db->Vacuum();
        const wxString SizeAfter = wxFileName(m_filename).GetHumanReadableSize();
        wxMessageBox(wxString::Format( 
            _("Database Optimization Completed!\n\n"
            "Size before: %s\n"
            "Size after: %s\n"), SizeBefore, SizeAfter),
            _("DB Optimization"));
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnDebugDB(wxCommandEvent& WXUNUSED(event))
{
    wxMessageDialog msgDlg(this
        , wxString::Format("%s\n\n%s", _("Please use this function only if explicitly requested by MMEX support"), _("Do you want to proceed?"))
        , _("DB Debug"), wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
    if (msgDlg.ShowModal() == wxID_YES)
    {
        dbUpgrade::SqlFileDebug(m_db.get());
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnSaveAs(wxCommandEvent& WXUNUSED(event))
{
    wxASSERT(m_db);
    wxASSERT(!m_filename.empty());

    wxFileDialog dlg(this,
        _("Save database file as"),
        wxEmptyString,
        wxEmptyString,
        _("MMEX database files (*.mmb)") + "|*.mmb"
            + (wxSQLite3Database::HasEncryptionSupport()
                ? "|" + _("Encrypted MMEX database files (*.mmb)") + "|*.mmb"
                : wxString()),
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT
        );

    wxFileName newFileName;
    bool encrypted, fileOK;
    wxString msg;
    do {
        if (dlg.ShowModal() != wxID_OK) return;

        fileOK=false;
        encrypted = dlg.GetFilterIndex() != 0; // -> Encrypted selected
        newFileName=dlg.GetPath();
        newFileName.SetExt("mmb");

        if (newFileName == m_filename) // on case-sensitive FS uses case-sensitive comparison
            msg = _("Can't copy file to itself");
        else if (newFileName.FileExists() && !wxRemoveFile(newFileName.GetFullPath()))
            msg = _("Can't overwrite selected file");
        else fileOK=true;
        if (!fileOK)
        {
            wxMessageDialog dlgMsg(this, msg, _("Save database file as"), wxOK | wxICON_WARNING);
            dlgMsg.ShowModal();
        }

    } while (!fileOK);

    // prepare to copy
    wxString password;
    if (encrypted)
    {
        password = readPasswordFromUser(true);
        if (password.empty())
            return;
    }

    // create backup database
    try {
        m_db->Backup(newFileName.GetFullPath(), password);
    }
    catch (wxSQLite3Exception& e) {
        wxMessageBox(e.GetMessage());
        return;
    }

    // load new database file created
    SetDatabaseFile(newFileName.GetFullPath(), false, password.IsEmpty(), password);
}

//----------------------------------------------------------------------------

void mmGUIFrame::OnExportToCSV(wxCommandEvent& WXUNUSED(event))
{
    mmUnivCSVDialog(this, mmUnivCSVDialog::DIALOG_TYPE_EXPORT_CSV).ShowModal();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnExportToXML(wxCommandEvent& WXUNUSED(event))
{
    mmUnivCSVDialog(this, mmUnivCSVDialog::DIALOG_TYPE_EXPORT_XML).ShowModal();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnExportToQIF(wxCommandEvent& WXUNUSED(event))
{
    mmQIFExportDialog dlg(this);
    dlg.ShowModal();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnExportToWebApp(wxCommandEvent& WXUNUSED(event))
{
    if (mmWebApp::WebApp_CheckEnabled())
    {
        if (mmWebApp::WebApp_CheckGuid() && mmWebApp::WebApp_CheckApiVersion())
        {
            mmWebApp::WebApp_UpdateAccount();
            mmWebApp::WebApp_UpdatePayee();
            mmWebApp::WebApp_UpdateCategory();
            wxMessageBox(_("Accounts, payees and categories successfully syncronized to WebApp!"), _("WebApp export"));
        }
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnImportQIF(wxCommandEvent& WXUNUSED(event))
{
    mmQIFImportDialog dlg(this, gotoAccountID_);
    dlg.ShowModal();
    int account_id = dlg.get_last_imported_acc();
    updateNavTreeControl();
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

void mmGUIFrame::OnImportUniversalCSV(wxCommandEvent& WXUNUSED(event))
{
    if (Model_Account::instance().all().empty())
    {
        wxMessageBox(_("No account available to import to"), _("Universal CSV Import"), wxOK | wxICON_WARNING);
        return;
    }

    mmUnivCSVDialog univCSVDialog(this, mmUnivCSVDialog::DIALOG_TYPE_IMPORT_CSV);
    univCSVDialog.ShowModal();
    if (univCSVDialog.ImportCompletedSuccessfully())
    {
        Model_Account::Data* account = Model_Account::instance().get(univCSVDialog.ImportedAccountID());
        createCheckingAccountPage(univCSVDialog.ImportedAccountID());
        if (account) setAccountNavTreeSection(account->ACCOUNTNAME);
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnImportXML(wxCommandEvent& WXUNUSED(event))
{
    if (Model_Account::instance().all().empty())
    {
        wxMessageBox(_("No account available to import to"), _("Universal CSV Import"), wxOK | wxICON_WARNING);
        return;
    }

    mmUnivCSVDialog univCSVDialog(this, mmUnivCSVDialog::DIALOG_TYPE_IMPORT_XML);
    univCSVDialog.ShowModal();
    if (univCSVDialog.ImportCompletedSuccessfully())
    {
        Model_Account::Data* account = Model_Account::instance().get(univCSVDialog.ImportedAccountID());
        createCheckingAccountPage(univCSVDialog.ImportedAccountID());
        if (account) setAccountNavTreeSection(account->ACCOUNTNAME);
    }
}

//----------------------------------------------------------------------------

void mmGUIFrame::OnImportWebApp(wxCommandEvent& WXUNUSED(event))
{
    if (mmWebApp::WebApp_CheckEnabled())
    {
        if (mmWebApp::WebApp_CheckGuid() && mmWebApp::WebApp_CheckApiVersion())
        {
            mmWebAppDialog dlg(this);
            dlg.ShowModal();
            if (dlg.getRefreshRequested())
                refreshPanelData();
        }
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close(TRUE);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnNewAccount(wxCommandEvent& WXUNUSED(event))
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
            wxMessageBox(_(
                "Share Accounts hold Share transactions\n\n"
                "Share transactions are created within the Stock Portfolio View\n"
                "after the selection of the Company Stock within the associated view.\n\n"
                "These accounts only become visible after associating a Stock to the Share Account\n"
                "Or by using the Menu View --> 'Display Share Accounts'\n"
                "Share Accounts can also hold normal transactions to regular account."
                ), _("Share Account Creation"));
        }

        updateNavTreeControl();
    }

    createHomePage();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnAccountList(wxCommandEvent& WXUNUSED(event))
{
    createHomePage();
}
//----------------------------------------------------------------------------

void mmGUIFrame::refreshPanelData(wxCommandEvent& WXUNUSED(event))
{
    refreshPanelData();
}
void mmGUIFrame::refreshPanelData()
{
    int id = panelCurrent_->GetId();
    wxLogDebug("Panel ID: %d", id);
    if (id == mmID_HOMEPAGE) //6000
        createHomePage();
    else if (id == mmID_CHECKING)
        checkingAccountPage_->RefreshList();
    else if (id == mmID_ASSETS) 
        { /*Nothing to do;*/ }
    else if (id == mmID_BILLS)
        billsDepositsPanel_->RefreshList();
    else if (id == mmID_BUDGET)
        budgetingPage_->RefreshList();
    else if (id == mmID_REPORTS)
    {
        if (activeReport_) //TODO: budget reports and transaction report
        {
            mmReportsPanel* rp = dynamic_cast<mmReportsPanel*>(panelCurrent_);
            if (rp) createReportsPage(rp->getPrintableBase(), false);
        }
    }
    else if (id == wxID_HELP)
        createHelpPage();

}

void mmGUIFrame::OnOrgCategories(wxCommandEvent& WXUNUSED(event))
{
    mmCategDialog dlg(this, -1, false);
    dlg.ShowModal();
    if (dlg.getRefreshRequested())
    {
        refreshPanelData();
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnOrgPayees(wxCommandEvent& WXUNUSED(event))
{
    mmPayeeDialog dlg(this,false);
    dlg.ShowModal();
    if (dlg.getRefreshRequested())
    {
        refreshPanelData();
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnNewTransaction(wxCommandEvent& WXUNUSED(event))
{
    if (m_db)
    {
        if (Model_Account::instance().all_checking_account_names().empty()) return;
        mmTransDialog dlg(this, gotoAccountID_, 0, 0);

        if (dlg.ShowModal() == wxID_OK)
        {
            gotoAccountID_ = dlg.GetAccountID();
            gotoTransID_ = dlg.GetTransactionID();
            Model_Account::Data * account = Model_Account::instance().get(gotoAccountID_);
            if (account)
            {
                createCheckingAccountPage(gotoAccountID_);
                setAccountNavTreeSection(account->ACCOUNTNAME);
            }
        }
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnBudgetSetupDialog(wxCommandEvent& WXUNUSED(event))
{
    if (m_db)
    {
        mmBudgetYearDialog(this).ShowModal();
        updateNavTreeControl();
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnTransactionReport(wxCommandEvent& WXUNUSED(event))
{
    if (!m_db) return;
    if (Model_Account::instance().all().empty()) return;

    mmFilterTransactionsDialog* dlg = new mmFilterTransactionsDialog(this, gotoAccountID_);
    if (dlg->ShowModal() == wxID_OK)
    {
        mmReportTransactions* rs = new mmReportTransactions(dlg->getAccountID(), dlg);
        createReportsPage(rs, true);
        setNavTreeSection(_("Reports"));
    }
}


void mmGUIFrame::OnCustomFieldsManager(wxCommandEvent& WXUNUSED(event)) 
{
    if (!m_db) return;
    const wxString& ref_type = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);

    mmCustomFieldListDialog dlg(this, ref_type);
    dlg.ShowModal();
    createHomePage();
}

void mmGUIFrame::OnGeneralReportManager(wxCommandEvent& WXUNUSED(event))
{
    if (!m_db) return;

    mmGeneralReportManager dlg(this, m_db.get());
    dlg.ShowModal();
    updateNavTreeControl();
    createHomePage(); //FIXME: refreshPanelData() crash if GRM opened then closed;
}

//----------------------------------------------------------------------------

void mmGUIFrame::OnOptions(wxCommandEvent& WXUNUSED(event))
{
    if (!m_db.get()) return;

    mmOptionsDialog systemOptions(this, this->m_app);
    if (systemOptions.ShowModal() == wxID_OK)
    {
        //set the View Menu Option items the same as the options saved.
        menuBar_->FindItem(MENU_VIEW_BUDGET_FINANCIAL_YEARS)->Check(Option::instance().BudgetFinancialYears());
        menuBar_->FindItem(MENU_VIEW_BUDGET_TRANSFER_TOTAL)->Check(Option::instance().BudgetIncludeTransfers());
        menuBar_->FindItem(MENU_VIEW_BUDGET_SETUP_SUMMARY)->Check(Option::instance().BudgetSetupWithoutSummaries());
        menuBar_->FindItem(MENU_VIEW_BUDGET_CATEGORY_SUMMARY)->Check(Option::instance().BudgetReportWithSummaries());
        menuBar_->FindItem(MENU_VIEW_IGNORE_FUTURE_TRANSACTIONS)->Check(Option::instance().IgnoreFutureTransactions());
        menuBar_->Refresh();
        menuBar_->Update();

        updateNavTreeControl();
        createHomePage();

        const wxString& sysMsg = _("MMEX Options have been updated.") + "\n\n";
        wxMessageBox(sysMsg, _("MMEX Options"), wxOK | wxICON_INFORMATION);
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnHelp(wxCommandEvent& WXUNUSED(event))
{
    helpFileIndex_ = mmex::HTML_INDEX;
    createHelpPage();
    setNavTreeSection(_("Help"));
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnCheckUpdate(wxCommandEvent& WXUNUSED(event))
{
    mmUpdate::checkUpdates(false, this);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnBeNotified(wxCommandEvent& WXUNUSED(event))
{
    Model_Setting::instance().Set(INIDB_NEWS_LAST_READ_DATE, wxDate::Today().FormatISODate());
    wxLaunchDefaultBrowser(mmex::weblink::News);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnSimpleURLOpen(wxCommandEvent& event)
{
    wxString url;
    switch(event.GetId())
    {
    case MENU_FACEBOOK: url=mmex::weblink::Facebook; break;
    case MENU_TWITTER: url=mmex::weblink::Twitter; break;
    case MENU_WEBSITE: url=mmex::weblink::WebSite; break;
    case MENU_WIKI: url=mmex::weblink::Wiki; break;
    case MENU_DONATE: url=mmex::weblink::Donate; break;
    case MENU_REPORTISSUES: url=mmex::weblink::Forum; break;
    case MENU_GOOGLEPLAY: url=mmex::weblink::GooglePlay; break;
    case MENU_BUY_COFFEE: url=mmex::weblink::SquareCashGuan; break;
    case MENU_RSS: url=mmex::weblink::NewsRSS; break;
    case MENU_YOUTUBE: url=mmex::weblink::YouTube; break;
    case MENU_GITHUB: url=mmex::weblink::GitHub; break;
    case MENU_SLACK: url=mmex::weblink::Slack; break;
    }
    if (!url.IsEmpty()) wxLaunchDefaultBrowser(url);
}

//----------------------------------------------------------------------------

void mmGUIFrame::OnReportBug(wxCommandEvent& WXUNUSED(event))
{
    std::vector<wxString> texts = {
        _("Please follow these tasks before submitting new bug:"),
        _("1. Use Help->Check for Updates in MMEX to get latest version - your problem can be fixed already."),
        _("2. Search https://github.com/moneymanagerex/moneymanagerex/issues?q=is:issue for similar problem - update existing issue instead of creating new one."),
        _("3. Put some descriptive name for your issue in the Title field above."),
        _("4. Replace this text (marked with >) with detailed description of your problem."),
        _("Read https://www.chiark.greenend.org.uk/~sgtatham/bugs.html for useful tips."),
        _("5. Include steps to reproduce your issue, attach screenshots where appropriate."),
        _("6. Please do not remove information attached below this text.")
    };
    std::vector<std::pair<wxString, wxString>> fixes = {
        { "\n\n", "<br>" }, { "\n", " " }, { "  ", " " },
        { "^" + _("Version: "), "\n<hr><small><b>Version</b>: " },
        { _("Database version: "), L"\u2022 db " },
        { _("Git commit: "), L"\u2022 git " },
        { _("Git branch: "), "" },
        { _("MMEX is using the following support products:") + L" \u2022", "<b>Libs</b>:" },
        { "<br>" + _("Build on"), "<br><b>Build</b>:" },
        { " " + _("with:"), "" },
        { _("Running on:") + L" \u2022", "<b>OS</b>:" },
        { "(.)$", "\\1</small>" }
    };
    wxRegEx re;
    wxString diag = mmex::getProgramDescription();
    for (const auto& kv: fixes)
        if (re.Compile(kv.first, wxRE_EXTENDED)) re.Replace(&diag, kv.second);
    wxString api = "/new?body=";
    for (const auto& text: texts)
        api << "> " << text << "\n";
    api << diag;
    wxURI req = mmex::weblink::BugReport + api;
    wxLaunchDefaultBrowser(req.BuildURI());
}

//----------------------------------------------------------------------------

void mmGUIFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    mmAboutDialog(this,0).ShowModal();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnPrintPage(wxCommandEvent& WXUNUSED(event))
{
    panelCurrent_->PrintPage();
}

//----------------------------------------------------------------------------

void mmGUIFrame::showBeginAppDialog(bool fromScratch)
{
    mmAppStartDialog dlg(this);
    if (fromScratch) dlg.SetCloseButtonToExit();

    int end_mod = dlg.ShowModal();
    if (end_mod == wxID_EXIT)
    {
        Close();
    }
    else if (end_mod == wxID_FILE1)
    {
        wxFileName fname(Model_Setting::instance().getLastDbPath());
        if (fname.IsOk()) SetDatabaseFile(fname.GetFullPath());
    }
    else if (end_mod == wxID_OPEN)
    {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_OPEN);
        AddPendingEvent(evt);
    }
    else if (end_mod == wxID_NEW)
    {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_NEW);
        AddPendingEvent(evt);
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

void mmGUIFrame::createBillsDeposits()
{
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String("Bills & Deposits Panel");

    json_writer.Key("start");
    json_writer.String(wxDateTime::Now().FormatISOCombined().c_str());

    if (panelCurrent_->GetId() == mmID_BILLS)
    {
        billsDepositsPanel_->RefreshList();
    }
    else
    {
        wxSizer *sizer = cleanupHomePanel();
        billsDepositsPanel_ = new mmBillsDepositsPanel(homePanel_, mmID_BILLS
            , wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
        panelCurrent_ = billsDepositsPanel_;

        sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);

        homePanel_->Layout();
        menuPrintingEnable(true);
    }

    json_writer.Key("end");
    json_writer.String(wxDateTime::Now().FormatISOCombined().c_str());
    json_writer.EndObject();

    Model_Usage::instance().AppendToUsage(json_buffer.GetString());
}
//----------------------------------------------------------------------------

void mmGUIFrame::createCheckingAccountPage(int accountID)
{
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String("Checking Panel");

    json_writer.Key("start");
    json_writer.String(wxDateTime::Now().FormatISOCombined().c_str());

    if (panelCurrent_->GetId() == mmID_CHECKING)
    {
        checkingAccountPage_->DisplayAccountDetails(accountID);
    }
    else
    {
        windowsFreezeThaw(homePanel_);
        wxSizer *sizer = cleanupHomePanel();
        checkingAccountPage_ = new mmCheckingPanel(homePanel_
            , this, accountID, mmID_CHECKING);
        panelCurrent_ = checkingAccountPage_;
        sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
        homePanel_->Layout();
        windowsFreezeThaw(homePanel_);
    }

    json_writer.Key("end");
    json_writer.String(wxDateTime::Now().FormatISOCombined().c_str());
    json_writer.EndObject();

    Model_Usage::instance().AppendToUsage(json_buffer.GetString());
    
    menuPrintingEnable(true);
    if (gotoTransID_ > 0)
    {
        checkingAccountPage_->SetSelectedTransaction(gotoTransID_);
    }
}

void mmGUIFrame::createStocksAccountPage(int accountID)
{
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String("Stock Panel");
    
    json_writer.Key("start");
    json_writer.String(wxDateTime::Now().FormatISOCombined().c_str());

    //TODO: Refresh Panel
    {
        //updateNavTreeControl();
        windowsFreezeThaw(homePanel_);
        wxSizer *sizer = cleanupHomePanel();
        panelCurrent_ = new mmStocksPanel(accountID, this, homePanel_, mmID_STOCKS);
        sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
        homePanel_->Layout();
        windowsFreezeThaw(homePanel_);
    }

    json_writer.Key("end");
    json_writer.String(wxDateTime::Now().FormatISOCombined().c_str());
    json_writer.EndObject();
    
    Model_Usage::instance().AppendToUsage(json_buffer.GetString());
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
}

void mmGUIFrame::OnGotoStocksAccount(wxCommandEvent& WXUNUSED(event))
{
    bool proper_type = false;
    Model_Account::Data *acc = Model_Account::instance().get(gotoAccountID_);
    if (acc)
        proper_type = Model_Account::type(acc) == Model_Account::INVESTMENT;
    if (proper_type)
        createStocksAccountPage(gotoAccountID_);
}

void mmGUIFrame::OnAssets(wxCommandEvent& WXUNUSED(event))
{
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String("Asset Panel");
    
    json_writer.Key("start");
    json_writer.String(wxDateTime::Now().FormatISOCombined().c_str());

    windowsFreezeThaw(homePanel_);
    wxSizer *sizer = cleanupHomePanel();
    panelCurrent_ = new mmAssetsPanel(this, homePanel_, mmID_ASSETS);
    sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
    homePanel_->Layout();
    windowsFreezeThaw(homePanel_);
    menuPrintingEnable(true);

    json_writer.Key("end");
    json_writer.String(wxDateTime::Now().FormatISOCombined().c_str());
    json_writer.EndObject();

    Model_Usage::instance().AppendToUsage(json_buffer.GetString());
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnCurrency(wxCommandEvent& WXUNUSED(event))
{
    mmMainCurrencyDialog(this, false, false).ShowModal();
    refreshPanelData();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnEditAccount(wxCommandEvent& WXUNUSED(event))
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
        if (dlg.ShowModal() == wxID_OK)
        {
            updateNavTreeControl();
            createHomePage();
        }
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnDeleteAccount(wxCommandEvent& WXUNUSED(event))
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
            _("Are you sure you want to delete\n %s account: %s ?")
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
    updateNavTreeControl();
    createHomePage();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnReallocateAccount(wxCommandEvent& WXUNUSED(event))
{
    mmSingleChoiceDialog account_choice(this
        , _("Select the account to reallocate"), _("Account Reallocation")
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
    wxArrayString t;
    for (const auto entry : types)
        t.Add(wxGetTranslation(entry));

    mmSingleChoiceDialog type_choice(this
        , wxString::Format(_("Account: %s - Select new type."), account->ACCOUNTNAME)
        , _("Account Reallocation"), t);

    if (type_choice.ShowModal() == wxID_OK)
    {
        int sel = type_choice.GetSelection();
        account->ACCOUNTTYPE = types[sel];
        Model_Account::instance().save(account);

        updateNavTreeControl();
        createHomePage();
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

void mmGUIFrame::OnHideShareAccounts(wxCommandEvent& WXUNUSED(event))
{
    m_hide_share_accounts = !m_hide_share_accounts;
    updateNavTreeControl();
}

void mmGUIFrame::RefreshNavigationTree()
{
    updateNavTreeControl();
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

void mmGUIFrame::OnViewBudgetSetupSummary(wxCommandEvent& WXUNUSED(event))
{
    Option::instance().BudgetSetupWithoutSummaries(!Option::instance().BudgetSetupWithoutSummaries());
    refreshPanelData();
}

void mmGUIFrame::OnViewBudgetCategorySummary(wxCommandEvent& WXUNUSED(event))
{
    Option::instance().BudgetReportWithSummaries(!Option::instance().BudgetReportWithSummaries());
    refreshPanelData();
}

void mmGUIFrame::OnViewIgnoreFutureTransactions(wxCommandEvent& WXUNUSED(event))
{
    Option::instance().IgnoreFutureTransactions(!Option::instance().IgnoreFutureTransactions());
    updateNavTreeControl();
    createHomePage();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnCategoryRelocation(wxCommandEvent& WXUNUSED(event))
{
    relocateCategoryDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxString msgStr;
        msgStr << _("Category Relocation Completed.") << "\n\n"
            << wxString::Format(_("Records have been updated in the database: %i"),
            dlg.updatedCategoriesCount());
        wxMessageBox(msgStr, _("Category Relocation Result"));
        refreshPanelData();
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnPayeeRelocation(wxCommandEvent& WXUNUSED(event))
{
    relocatePayeeDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxString msgStr;
        msgStr << _("Payee Relocation Completed.") << "\n\n"
            << wxString::Format(_("Records have been updated in the database: %i"),
            dlg.updatedPayeesCount())
            << "\n\n";
        wxMessageBox(msgStr, _("Payee Relocation Result"));
        refreshPanelData();
    }
}
//----------------------------------------------------------------------------

wxSizer* mmGUIFrame::cleanupHomePanel(bool new_sizer)
{
    wxASSERT(homePanel_);

    if (panelCurrent_)
    {
        delete panelCurrent_;
        panelCurrent_ = nullptr;
    }
    homePanel_->DestroyChildren();
    homePanel_->SetSizer(new_sizer ? new wxBoxSizer(wxHORIZONTAL) : nullptr);

    return homePanel_->GetSizer();
}
//----------------------------------------------------------------------------

void mmGUIFrame::SetDatabaseFile(const wxString& dbFileName, bool newDatabase, bool encrypt, const wxString& password)
{
    autoRepeatTransactionsTimer_.Stop();

    if (openFile(dbFileName, newDatabase, encrypt, password))
    {
        updateNavTreeControl();
        createHomePage();
        mmLoadColorsFromDatabase();
    }
    else if (!m_db)
        showBeginAppDialog();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnRecentFiles(wxCommandEvent& event)
{
    int fileNum = event.GetId() - m_recentFiles->GetBaseId();
    const wxString file_name = m_recentFiles->GetHistoryFile(fileNum);
    wxFileName file(file_name);
    if (m_db && file == m_filename) return; // already open
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

void mmGUIFrame::OnClearRecentFiles(wxCommandEvent& WXUNUSED(event))
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

void mmGUIFrame::OnClose(wxCloseEvent&)
{
    Destroy();
}

void mmGUIFrame::OnHideShowReport(wxCommandEvent& event)
{
    int report = event.GetId() - MENU_TREEPOPUP_HIDE_SHOW_REPORT;
    Option::instance().HideReport(report, !Option::instance().HideReport(report));
    updateNavTreeControl();
    createHomePage();
}

void mmGUIFrame::OnChangeGUILanguage(wxCommandEvent& event)
{
    wxLanguage lang = static_cast<wxLanguage>(event.GetId()-MENU_LANG-1);
    if (lang!=m_app->getGUILanguage() && m_app->setGUILanguage(lang))
        mmErrorDialogs::MessageWarning(this
            , _("The language for this application has been changed. "
                "The change will take effect the next time the application is started.")
            , _("Language change"));
}
