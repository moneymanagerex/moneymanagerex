/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2012 Stefano Giorgio
 Copyright (C) 2013 Nikolay
 Copyright (C) 2014 James Higley
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
#include "optionsdialog.h"
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
#include "webserver.h"
#include "wizard_newdb.h"
#include "wizard_newaccount.h"
#include "wizard_update.h"

#include "reports/budgetcategorysummary.h"
#include "reports/budgetingperf.h"
#include "reports/cashflow.h"
#include "reports/categexp.h"
#include "reports/categovertimeperf.h"
#include "reports/incexpenses.h"
#include "reports/htmlbuilder.h"
#include "reports/payee.h"
#include "reports/transactions.h"

#include "import_export/qif_export.h"
#include "import_export/qif_import_gui.h"
#include "import_export/univcsvdialog.h"

#include "model/Model_Account.h"
#include "model/Model_Asset.h"
#include "model/Model_Attachment.h"
#include "model/Model_Billsdeposits.h"
#include "model/Model_Budget.h"
#include "model/Model_Budgetyear.h"
#include "model/Model_Category.h"
#include "model/Model_Checking.h"
#include "model/Model_CustomField.h"
#include "model/Model_CustomFieldData.h"
#include "model/Model_CurrencyHistory.h"
#include "model/Model_Infotable.h"
#include "model/Model_Payee.h"
#include "model/Model_Report.h"
#include "model/Model_Setting.h"
#include "model/Model_Splittransaction.h"
#include "model/Model_Stock.h"
#include "model/Model_StockHistory.h"
#include "model/Model_Subcategory.h"
#include "model/Model_Usage.h"

#include "search/Search.h"

#include <wx/fs_mem.h>
#include <stack>

#include "cajun/json/elements.h"
#include "cajun/json/reader.h"
#include "cajun/json/writer.h"

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
EVT_MENU(MENU_GOOGLEPLAY, mmGUIFrame::OnGooglePlay)
EVT_MENU(MENU_REPORTISSUES, mmGUIFrame::OnReportIssues)
EVT_MENU(MENU_ANNOUNCEMENTMAILING, mmGUIFrame::OnBeNotified)
EVT_MENU(MENU_FACEBOOK, mmGUIFrame::OnFacebook)
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
EVT_MENU(MENU_TRANSACTIONREPORT, mmGUIFrame::OnTransactionReport)
EVT_MENU(wxID_VIEW_LIST, mmGUIFrame::OnGeneralReportManager)
EVT_MENU(MENU_TREEPOPUP_LAUNCHWEBSITE, mmGUIFrame::OnLaunchAccountWebsite)
EVT_MENU(MENU_TREEPOPUP_ACCOUNTATTACHMENTS, mmGUIFrame::OnAccountAttachments)
EVT_MENU(MENU_VIEW_TOOLBAR, mmGUIFrame::OnViewToolbar)
EVT_MENU(MENU_VIEW_LINKS, mmGUIFrame::OnViewLinks)
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

wxEND_EVENT_TABLE()
//----------------------------------------------------------------------------

mmGUIFrame::mmGUIFrame(mmGUIApp* app, const wxString& title
    , const wxPoint& pos
    , const wxSize& size)
    : wxFrame(0, -1, title, pos, size)
    , m_app(app)
    , m_commit_callback_hook(nullptr)
    , m_update_callback_hook(nullptr)
    , gotoAccountID_(-1)
    , gotoTransID_(-1)
    , checkingAccountPage_(nullptr)
    , budgetingPage_(nullptr)
    , billsDepositsPanel_(nullptr)
    , homePage_(nullptr)
    , homePanel_(nullptr)
    , activeReport_(false)
    , panelCurrent_(nullptr)
    , navTreeCtrl_(nullptr)
    , menuBar_(nullptr)
    , toolBar_(nullptr)
    , selectedItemData_(nullptr)
    , helpFileIndex_(-1)
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
    CreateToolBar();
    createControls();

#if wxUSE_STATUSBAR
    CreateStatusBar();
#endif // wxUSE_STATUSBAR
    m_recentFiles = new mmFileHistory(); // TODO Max files
    m_recentFiles->UseMenu(m_menuRecentFiles);
    m_recentFiles->Load();

    // Load perspective
    wxString auiPerspective = Model_Setting::instance().GetStringSetting("AUIPERSPECTIVE", wxEmptyString);
    m_mgr.LoadPerspective(auiPerspective);

    // add the toolbars to the manager
    m_mgr.AddPane(toolBar_, wxAuiPaneInfo().
        Name("toolbar").Caption(_("Toolbar")).ToolbarPane().Top()
        .LeftDockable(false).RightDockable(false).MinSize(1000, -1)
        .Show(Model_Setting::instance().GetBoolSetting("SHOWTOOLBAR", true)));

    // change look and feel of wxAuiManager
    m_mgr.GetArtProvider()->SetMetric(16, 0);
    m_mgr.GetArtProvider()->SetMetric(3, 1);

    // "commit" all changes made to wxAuiManager
    m_mgr.Update();

    // Show license agreement at first open
    if (Model_Setting::instance().GetStringSetting(INIDB_SEND_USAGE_STATS, "") == "")
    {
        mmAboutDialog(this, 5).ShowModal();
        Model_Setting::instance().Set(INIDB_SEND_USAGE_STATS, "TRUE");
    }

    //Check for new version at startup
    if (Model_Setting::instance().GetBoolSetting("UPDATECHECK", true))
        mmUpdate::checkUpdates(true,this);

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
            updateNavTreeControl();
            //setHomePageActive(false);
            createHomePage();
            mmLoadColorsFromDatabase();
        }
        else
        {
            resetNavTreeControl();
            cleanupHomePanel();
            showBeginAppDialog(true);
        }
    }

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
        Model_Usage::instance().append_cache_usage(model->cache_to_json());
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::cleanup()
{
    autoRepeatTransactionsTimer_.Stop();
    delete m_recentFiles;
    if (!m_filename.IsEmpty()) // Exiting before file is opened
        saveSettings();

    wxTreeItemId rootitem = navTreeCtrl_->GetRootItem();
    cleanupNavTreeControl(rootitem);
    m_mgr.UnInit();

    /* Delete the GUI */
    cleanupHomePanel(false);
    ShutdownDatabase();
    /// Update the database according to user requirements
    if (mmOptions::instance().databaseUpdated_ && Model_Setting::instance().GetBoolSetting("BACKUPDB_UPDATE", false))
        dbUpgrade::BackupDB(m_filename, dbUpgrade::BACKUPTYPE::CLOSE, Model_Setting::instance().GetIntSetting("MAX_BACKUP_FILES", 4));
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
    wxTreeItemId root = navTreeCtrl_->GetRootItem();
    cleanupNavTreeControl(root);
    navTreeCtrl_->DeleteAllItems();
}

void mmGUIFrame::cleanupNavTreeControl(wxTreeItemId& item)
{
    while (item.IsOk())
    {
        if (navTreeCtrl_->ItemHasChildren(item))
        {
            wxTreeItemIdValue cookie;
            wxTreeItemId childitem = navTreeCtrl_->GetFirstChild(item, cookie);
            cleanupNavTreeControl(childitem);
        }
        mmTreeItemData* iData = dynamic_cast<mmTreeItemData*>(navTreeCtrl_->GetItemData(item));
        navTreeCtrl_->SetItemData(item, 0);
        if (iData)
            delete iData;
        item = navTreeCtrl_->GetNextSibling(item);
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
    wxTreeItemId navTreeID = navTreeCtrl_->GetFirstChild(itemID, treeDummyValue);

    while (navTreeID.IsOk())
    {
        if (accountName == navTreeCtrl_->GetItemText(navTreeID))
            break;
        else
            navTreeID = navTreeCtrl_->GetNextChild(itemID, treeDummyValue);
    }
    return navTreeID;
}

//----------------------------------------------------------------------------
bool mmGUIFrame::setAccountInSection(const wxString& sectionName, const wxString& accountName)
{
    navTreeCtrl_->SetEvtHandlerEnabled(false);
    bool accountNotFound = true;
    wxTreeItemId rootItem = getTreeItemfor(navTreeCtrl_->GetRootItem(), sectionName);
    if (rootItem.IsOk() && navTreeCtrl_->ItemHasChildren(rootItem))
    {
        navTreeCtrl_->ExpandAllChildren(rootItem);
        wxTreeItemId accountItem = getTreeItemfor(rootItem, accountName);
        if (accountItem.IsOk())
        {
            // Set the NavTreeCtrl and prevent any event code being executed for now.
            navTreeCtrl_->SelectItem(accountItem);
            //processPendingEvents();
            accountNotFound = false;
        }
    }
    navTreeCtrl_->SetEvtHandlerEnabled(true);
    return accountNotFound;
}

//----------------------------------------------------------------------------
bool mmGUIFrame::setNavTreeSection(const wxString &sectionName)
{
    navTreeCtrl_->SetEvtHandlerEnabled(false);
    bool accountNotFound = true;
    wxTreeItemId rootItem = getTreeItemfor(navTreeCtrl_->GetRootItem(), sectionName);
    if (rootItem.IsOk())
    {
        // Set the NavTreeCtrl and prevent any event code being executed for now.
        navTreeCtrl_->SelectItem(rootItem);
        //processPendingEvents();
        accountNotFound = false;
    }
    navTreeCtrl_->SetEvtHandlerEnabled(true);
    return accountNotFound;
}

//----------------------------------------------------------------------------
void mmGUIFrame::setAccountNavTreeSection(const wxString& accountName)
{
    if (setAccountInSection(_("Bank Accounts"), accountName)) {
        if (setAccountInSection(_("Term Accounts"), accountName))
            setAccountInSection(_("Stocks"), accountName);
    }
}

//----------------------------------------------------------------------------
void mmGUIFrame::OnAutoRepeatTransactionsTimer(wxTimerEvent& /*event*/)
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

    Model_Billsdeposits& bills = Model_Billsdeposits::instance();
    for (const auto& q1 : bills.all())
    {
        bills.decode_fields(q1);
        const wxDateTime payment_date = bills.TRANSDATE(q1);
        if (bills.autoExecuteManual() && bills.requireExecution())
        {
            if (bills.allowExecution())
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
    menuBar_->FindItem(MENU_ACCTDELETE)->Enable(enable);

    menuBar_->FindItem(MENU_ORGCATEGS)->Enable(enable);
    menuBar_->FindItem(MENU_ORGPAYEE)->Enable(enable);
    menuBar_->FindItem(MENU_CATEGORY_RELOCATION)->Enable(enable);
    menuBar_->FindItem(MENU_PAYEE_RELOCATION)->Enable(enable);
    menuBar_->FindItem(wxID_VIEW_LIST)->Enable(enable);
    menuBar_->FindItem(MENU_CONVERT_ENC_DB)->Enable(enable);

    menuBar_->FindItem(MENU_IMPORT)->Enable(enable);
    menuBar_->FindItem(wxID_PRINT)->Enable(enable);
    menuBar_->FindItem(wxID_PREFERENCES)->Enable(enable);
    menuBar_->FindItem(MENU_BILLSDEPOSITS)->Enable(enable);
    menuBar_->FindItem(MENU_CURRENCY)->Enable(enable);
    menuBar_->FindItem(MENU_ASSETS)->Enable(enable);
    menuBar_->FindItem(MENU_BUDGETSETUPDIALOG)->Enable(enable);
    menuBar_->FindItem(MENU_TRANSACTIONREPORT)->Enable(enable);

    toolBar_->EnableTool(MENU_NEWACCT, enable);
    toolBar_->EnableTool(MENU_ACCTLIST, enable);
    toolBar_->EnableTool(MENU_ORGPAYEE, enable);
    toolBar_->EnableTool(MENU_ORGCATEGS, enable);
    toolBar_->EnableTool(MENU_CURRENCY, enable);
    toolBar_->EnableTool(wxID_VIEW_LIST, enable);
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
    navTreeCtrl_ = new wxTreeCtrl( this, ID_NAVTREECTRL,
        wxDefaultPosition, wxSize(100, 100));
#else
    navTreeCtrl_ = new wxTreeCtrl(this, ID_NAVTREECTRL,
        wxDefaultPosition, wxSize(100, 100), wxTR_SINGLE | wxTR_HAS_BUTTONS | wxTR_ROW_LINES);
#endif

    navTreeCtrl_->AssignImageList(navtree_images_list());
    navTreeCtrl_->Connect(ID_NAVTREECTRL, wxEVT_TREE_SEL_CHANGED, wxTreeEventHandler(mmGUIFrame::OnSelChanged), nullptr, this);

    homePanel_ = new wxPanel(this, wxID_ANY,
        wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxTR_SINGLE | wxNO_BORDER);

    m_mgr.AddPane(navTreeCtrl_, wxAuiPaneInfo().
        Name("Navigation").Caption(_("Navigation")).
        BestSize(wxSize(200, 100)).MinSize(wxSize(100, 100)).
        Left());

    m_mgr.AddPane(homePanel_, wxAuiPaneInfo().
        Name("Home").Caption("Home").
        CenterPane().PaneBorder(false));
}
//----------------------------------------------------------------------------

void mmGUIFrame::updateNavTreeControl()
{
    windowsFreezeThaw(navTreeCtrl_);
    navTreeCtrl_->SetEvtHandlerEnabled(false);
    wxTreeItemId root = navTreeCtrl_->GetRootItem();
    cleanupNavTreeControl(root);
    navTreeCtrl_->DeleteAllItems();

    root = navTreeCtrl_->AddRoot(_("Home Page"), img::HOUSE_PNG, img::HOUSE_PNG);
    navTreeCtrl_->SetItemData(root, new mmTreeItemData("Home Page"));
    navTreeCtrl_->SetItemBold(root, true);
    navTreeCtrl_->SetFocus();

    wxTreeItemId accounts = navTreeCtrl_->AppendItem(root, _("Bank Accounts"), img::SAVINGS_ACC_NORMAL_PNG, img::SAVINGS_ACC_NORMAL_PNG);
    navTreeCtrl_->SetItemData(accounts, new mmTreeItemData("Bank Accounts"));
    navTreeCtrl_->SetItemBold(accounts, true);
    wxTreeItemId cardAccounts = navTreeCtrl_->AppendItem(root, _("Credit Card Accounts"), img::CARD_ACC_PNG, img::CARD_ACC_PNG);
    navTreeCtrl_->SetItemData(cardAccounts, new mmTreeItemData("Credit Card Accounts"));
    navTreeCtrl_->SetItemBold(cardAccounts, true);

    wxTreeItemId cashAccounts = navTreeCtrl_->AppendItem(root, _("Cash Accounts"), img::CASH_ACC_NORMAL_PNG, img::CASH_ACC_NORMAL_PNG);
    navTreeCtrl_->SetItemData(cashAccounts, new mmTreeItemData("Cash Accounts"));
    navTreeCtrl_->SetItemBold(cashAccounts, true);
    wxTreeItemId loanAccounts = navTreeCtrl_->AppendItem(root, _("Loan Accounts"), img::LOAN_ACC_NORMAL_PNG, img::LOAN_ACC_NORMAL_PNG);
    navTreeCtrl_->SetItemData(loanAccounts, new mmTreeItemData("Loan Accounts"));
    navTreeCtrl_->SetItemBold(loanAccounts, true);

    wxTreeItemId termAccount = navTreeCtrl_->AppendItem(root, _("Term Accounts"), img::TERMACCOUNT_PNG, img::TERMACCOUNT_PNG);
    navTreeCtrl_->SetItemData(termAccount, new mmTreeItemData("Term Accounts"));
    navTreeCtrl_->SetItemBold(termAccount, true);
    wxTreeItemId stocks = navTreeCtrl_->AppendItem(root, _("Stocks"), img::STOCK_ACC_PNG, img::STOCK_ACC_PNG);
    navTreeCtrl_->SetItemData(stocks, new mmTreeItemData("Stocks"));
    navTreeCtrl_->SetItemBold(stocks, true);

    wxTreeItemId assets = navTreeCtrl_->AppendItem(root, _("Assets"), img::ASSET_PNG, img::ASSET_PNG);
    navTreeCtrl_->SetItemData(assets, new mmTreeItemData("Assets"));
    navTreeCtrl_->SetItemBold(assets, true);

    wxTreeItemId bills = navTreeCtrl_->AppendItem(root, _("Recurring Transactions"), img::SCHEDULE_PNG, img::SCHEDULE_PNG);
    navTreeCtrl_->SetItemData(bills, new mmTreeItemData("Bills & Deposits"));
    navTreeCtrl_->SetItemBold(bills, true);

    wxTreeItemId budgeting = navTreeCtrl_->AppendItem(root, _("Budget Setup"), img::CALENDAR_PNG, img::CALENDAR_PNG);
    navTreeCtrl_->SetItemData(budgeting, new mmTreeItemData("Budgeting"));
    navTreeCtrl_->SetItemBold(budgeting, true);

    wxTreeItemId reports = navTreeCtrl_->AppendItem(root, _("Reports"), img::PIECHART_PNG, img::PIECHART_PNG);
    navTreeCtrl_->SetItemBold(reports, true);
    navTreeCtrl_->SetItemData(reports, new mmTreeItemData("Reports"));

    this->updateReportNavigation(reports, budgeting);

    ///////////////////////////////////////////////////////////////////

    wxTreeItemId help = navTreeCtrl_->AppendItem(root, _("Help"), img::HELP_PNG, img::HELP_PNG);
    navTreeCtrl_->SetItemData(help, new mmTreeItemData("Help"));
    navTreeCtrl_->SetItemBold(help, true);

    if (m_db)
    {
        /* Start Populating the dynamic data */
        const wxString vAccts = Model_Setting::instance().ViewAccounts();
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

            int selectedImage = mmIniOptions::instance().account_image_id(account.ACCOUNTID);

            wxTreeItemId tacct;

            switch (Model_Account::type(account))
            {
            case Model_Account::INVESTMENT:
                tacct = navTreeCtrl_->AppendItem(stocks, account.ACCOUNTNAME, selectedImage, selectedImage);
                break;
            case Model_Account::TERM:
                tacct = navTreeCtrl_->AppendItem(termAccount, account.ACCOUNTNAME, selectedImage, selectedImage);
                break;
            case Model_Account::CREDIT_CARD:
                tacct = navTreeCtrl_->AppendItem(cardAccounts, account.ACCOUNTNAME, selectedImage, selectedImage);
                break;
            case Model_Account::CASH:
                tacct = navTreeCtrl_->AppendItem(cashAccounts, account.ACCOUNTNAME, selectedImage, selectedImage);
                break;
            case Model_Account::LOAN:
                tacct = navTreeCtrl_->AppendItem(loanAccounts, account.ACCOUNTNAME, selectedImage, selectedImage);
                break;
            default: 
                tacct = navTreeCtrl_->AppendItem(accounts, account.ACCOUNTNAME, selectedImage, selectedImage);
                break;
            }

            navTreeCtrl_->SetItemData(tacct, new mmTreeItemData(account.ACCOUNTID, false));
        }

        loadNavTreeItemsStatus();
        if (!navTreeCtrl_->ItemHasChildren(accounts)) navTreeCtrl_->Delete(accounts);
        if (!navTreeCtrl_->ItemHasChildren(cardAccounts)) navTreeCtrl_->Delete(cardAccounts);
        if (!navTreeCtrl_->ItemHasChildren(termAccount)) navTreeCtrl_->Delete(termAccount);
        if (!navTreeCtrl_->ItemHasChildren(stocks)) navTreeCtrl_->Delete(stocks);
        if (!navTreeCtrl_->ItemHasChildren(cashAccounts)) navTreeCtrl_->Delete(cashAccounts);
        if (!navTreeCtrl_->ItemHasChildren(loanAccounts)) navTreeCtrl_->Delete(loanAccounts);
    }
    windowsFreezeThaw(navTreeCtrl_);
    navTreeCtrl_->SelectItem(root);
    navTreeCtrl_->EnsureVisible(root);
    navTreeCtrl_->Refresh();
    navTreeCtrl_->Update();
    navTreeCtrl_->SetEvtHandlerEnabled(true);
}


void mmGUIFrame::loadNavTreeItemsStatus()
{
    /* Load Nav Tree Control */
    SetEvtHandlerEnabled(false);
    wxTreeItemId root = navTreeCtrl_->GetRootItem();
    navTreeCtrl_->Expand(root);

    wxString str = Model_Infotable::instance().GetStringInfo("NAV_TREE_STATUS", "");
    if (!(str.StartsWith("{") && str.EndsWith("}"))) str = "{}";
    wxLogDebug("%s", str);
    std::wstringstream ss;
    ss << str.ToStdWstring();
    json::Object o;
    json::Reader::Read(o, ss);

    std::stack<wxTreeItemId> items;
    if (navTreeCtrl_->GetRootItem().IsOk())
        items.push(navTreeCtrl_->GetRootItem());

    while (!items.empty())
    {
        wxTreeItemId next = items.top();
        items.pop();

        wxTreeItemIdValue cookie;
        wxTreeItemId nextChild = navTreeCtrl_->GetFirstChild(next, cookie);
        while (nextChild.IsOk())
        {
            if (navTreeCtrl_->HasChildren(nextChild)) items.push(nextChild);
            nextChild = navTreeCtrl_->GetNextSibling(nextChild);
        }

        mmTreeItemData* iData =
            dynamic_cast<mmTreeItemData*>(navTreeCtrl_->GetItemData(next));
        if (iData && json::Boolean(o[iData->getString().ToStdWstring()]))
            navTreeCtrl_->Expand(next);
    };

    SetEvtHandlerEnabled(true);
}

void mmGUIFrame::OnTreeItemExpanded(wxTreeEvent& event)
{
    mmTreeItemData* iData =
        dynamic_cast<mmTreeItemData*>(navTreeCtrl_->GetItemData(event.GetItem()));
    if (!iData) return;
    navTreeStateToJson();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnTreeItemCollapsed(wxTreeEvent& event)
{
    mmTreeItemData* iData =
        dynamic_cast<mmTreeItemData*>(navTreeCtrl_->GetItemData(event.GetItem()));
    if (!iData) return;
    navTreeStateToJson();
}

void mmGUIFrame::navTreeStateToJson()
{
    json::Object o;
    o.Clear();

    std::stack<wxTreeItemId> items;
    if (navTreeCtrl_->GetRootItem().IsOk())
        items.push(navTreeCtrl_->GetRootItem());

    while (!items.empty())
    {
        wxTreeItemId next = items.top();
        items.pop();

        wxTreeItemIdValue cookie;
        wxTreeItemId nextChild = navTreeCtrl_->GetFirstChild(next, cookie);
        while (nextChild.IsOk())
        {
            if (navTreeCtrl_->HasChildren(nextChild)) items.push(nextChild);
            nextChild = navTreeCtrl_->GetNextSibling(nextChild);
        }

        mmTreeItemData* iData =
            dynamic_cast<mmTreeItemData*>(navTreeCtrl_->GetItemData(next));
        if (iData && json::Boolean(navTreeCtrl_->IsExpanded(next)))
            o[iData->getString().ToStdWstring()] = json::Boolean(navTreeCtrl_->IsExpanded(next));
    };
    std::wstringstream ss;
    json::Writer::Write(o, ss);
    wxLogDebug("%s", ss.str());
    wxLogDebug("==========================================");
    Model_Infotable::instance().Set("NAV_TREE_STATUS", ss.str());

}
//----------------------------------------------------------------------------

void mmGUIFrame::OnSelChanged(wxTreeEvent& event)
{
    menuPrintingEnable(false);
    wxTreeItemId selectedItem = event.GetItem();
    if (!selectedItem) return;

    mmTreeItemData* iData = dynamic_cast<mmTreeItemData*>(navTreeCtrl_->GetItemData(selectedItem));
    selectedItemData_ = iData;
    if (!iData) return;

    activeReport_ = false;
    if (!iData->isStringData())
    {
        if (iData->isBudgetingNode())
        {
            int year = iData->getData();

            wxTreeItemId idparent = navTreeCtrl_->GetItemParent(selectedItem);
            mmTreeItemData* iParentData = dynamic_cast<mmTreeItemData*>(navTreeCtrl_->GetItemData(idparent));
            if (iParentData->getString() == "item@Budget Performance") //FIXME: this is report
            {
                mmPrintableBase* rs = new mmReportBudgetingPerformance(year);
                createReportsPage(rs, true);
            }
            else if (iParentData->getString() == "item@Budget Setup Performance") //FIXME: this is report
            {
                mmPrintableBase* rs = new mmReportBudgetCategorySummary(year);
                createReportsPage(rs, true);
            }
            else
            {
                createBudgetingPage(year);
            }
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
                navTreeCtrl_->SetFocus();
                menuPrintingEnable(true);
            }
            else
            {
                /* cannot find accountid */
                wxASSERT(true);
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
//----------------------------------------------------------------------------

void mmGUIFrame::OnPopupDeleteAccount(wxCommandEvent& /*event*/)
{
    if (selectedItemData_)
    {
        int data = selectedItemData_->getData();
        Model_Account::Data* account = Model_Account::instance().get(data);
        if (account)
        {
            wxMessageDialog msgDlg(this
                , _("Do you really want to delete the account?")
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
    navTreeCtrl_->SelectItem(selectedItem);
}
//----------------------------------------------------------------------------

void mmGUIFrame::showTreePopupMenu(const wxTreeItemId& id, const wxPoint& pt)
{
    mmTreeItemData* iData = dynamic_cast<mmTreeItemData*>(navTreeCtrl_->GetItemData(id));
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
            PopupMenu(&menu, pt);
        }
        else if (iData->getString() == "item@Bank Accounts" ||
            iData->getString() == "item@Term Accounts" ||
            iData->getString() == "item@Credit Card Accounts" ||
            iData->getString() == "item@Stocks")
        {
            // Create for Bank Credit Card Term & Stock Accounts
            wxMenu menu;
            menu.Append(MENU_TREEPOPUP_ACCOUNT_NEW, _("New &Account"));
            menu.Append(MENU_TREEPOPUP_ACCOUNT_DELETE, _("&Delete Account"));
            menu.Append(MENU_TREEPOPUP_ACCOUNT_EDIT, _("&Edit Account"));
            menu.Append(MENU_TREEPOPUP_ACCOUNT_LIST, _("Account &List (Home)"));
            menu.AppendSeparator();

            // Create only for Bank & Credit Card Accounts
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
    json::Object o;
    o[L"module"] = json::String(L"Budget Panel");
    o[L"start"] = json::String(wxDateTime::Now().FormatISOCombined().ToStdWstring());
    navTreeCtrl_->SetEvtHandlerEnabled(false);
    if (panelCurrent_->GetId() == mmID_BUDGET)
    {
        budgetingPage_->DisplayBudgetingDetails(budgetYearID);
    }
    else
    {
        windowsFreezeThaw(homePanel_);
        wxSizer *sizer = cleanupHomePanel();

        budgetingPage_ = new mmBudgetingPanel(budgetYearID
            , homePanel_, this, mmID_BUDGET, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
        panelCurrent_ = budgetingPage_;

        sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
        homePanel_->Layout();
        windowsFreezeThaw(homePanel_);
    }
    o[L"end"] = json::String(wxDateTime::Now().FormatISOCombined().ToStdWstring());
    Model_Usage::instance().append(o);
    menuPrintingEnable(true);
    navTreeCtrl_->SetEvtHandlerEnabled(true);
}
//----------------------------------------------------------------------------

void mmGUIFrame::createHomePage()
{
    json::Object o;
    o[L"module"] = json::String(L"Home Page");
    o[L"start"] = json::String(wxDateTime::Now().FormatISOCombined().ToStdWstring());

    navTreeCtrl_->SetEvtHandlerEnabled(false);
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
    navTreeCtrl_->SelectItem(navTreeCtrl_->GetRootItem());
    navTreeCtrl_->SetEvtHandlerEnabled(true);

    o[L"end"] = json::String(wxDateTime::Now().FormatISOCombined().ToStdWstring());
    Model_Usage::instance().append(o);
}
//----------------------------------------------------------------------------

void mmGUIFrame::createReportsPage(mmPrintableBase* rs, bool cleanup)
{
    if (!rs) return;
    navTreeCtrl_->SetEvtHandlerEnabled(false);

    windowsFreezeThaw(homePanel_);
    wxSizer *sizer = cleanupHomePanel();
    panelCurrent_ = new mmReportsPanel(rs
        , cleanup, homePanel_, this, mmID_REPORTS
        , wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);

    sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
    homePanel_->Layout();
    windowsFreezeThaw(homePanel_);

    menuPrintingEnable(true);
    navTreeCtrl_->SetEvtHandlerEnabled(true);
}
//----------------------------------------------------------------------------

void mmGUIFrame::createHelpPage()
{
    navTreeCtrl_->SetEvtHandlerEnabled(false);
    windowsFreezeThaw(homePanel_);
    wxSizer *sizer = cleanupHomePanel();
    panelCurrent_ = new mmHelpPanel(homePanel_, this, wxID_HELP
        , wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
    sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
    homePanel_->Layout();
    windowsFreezeThaw(homePanel_);
    menuPrintingEnable(true);
    navTreeCtrl_->SetEvtHandlerEnabled(true);
}
//----------------------------------------------------------------------------

void mmGUIFrame::createMenu()
{
    int vFontSize = Model_Setting::instance().GetHtmlScaleFactor();
    int x = 16;
    if (vFontSize >= 300) x = 48;
    else if (vFontSize >= 200) x = 32;

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

    wxMenuItem* menuItemQuit = new wxMenuItem(menu_file, wxID_EXIT,
        _("E&xit\tAlt-X"), _("Quit this program"));
    menuItemQuit->SetBitmap(mmBitmap(png::EXIT));
    menu_file->Append(menuItemQuit);

    // Create the required menu items
    wxMenu *menuView = new wxMenu;
    wxMenuItem* menuItemToolbar = new wxMenuItem(menuView, MENU_VIEW_TOOLBAR,
        _("&Toolbar"), _("Show/Hide the toolbar"), wxITEM_CHECK);
    wxMenuItem* menuItemLinks = new wxMenuItem(menuView, MENU_VIEW_LINKS,
        _("&Navigation"), _("Show/Hide the Navigation tree control"), wxITEM_CHECK);;
    wxMenuItem* menuItemBudgetFinancialYears = new wxMenuItem(menuView, MENU_VIEW_BUDGET_FINANCIAL_YEARS,
        _("Budgets: As &Financial Years"), _("Display Budgets in Financial Year Format"), wxITEM_CHECK);
    wxMenuItem* menuItemBudgetTransferTotal = new wxMenuItem(menuView, MENU_VIEW_BUDGET_TRANSFER_TOTAL,
        _("Budgets: &Include Transfers in Totals"), _("Include the transfer transactions in the Budget Totals"), wxITEM_CHECK);
    wxMenuItem* menuItemBudgetSetupWithoutSummary = new wxMenuItem(menuView, MENU_VIEW_BUDGET_SETUP_SUMMARY,
        _("Budget Setup: &Without Summaries"), _("Display the Budget Setup without category summaries"), wxITEM_CHECK);
    wxMenuItem* menuItemBudgetCategorySummary = new wxMenuItem(menuView, MENU_VIEW_BUDGET_CATEGORY_SUMMARY,
        _("Budget Summary: Include &Categories"), _("Include the categories in the Budget Category Summary"), wxITEM_CHECK);
    wxMenuItem* menuItemIgnoreFutureTransactions = new wxMenuItem(menuView, MENU_VIEW_IGNORE_FUTURE_TRANSACTIONS,
        _("Ignore F&uture Transactions"), _("Ignore Future transactions"), wxITEM_CHECK);
    //Add the menu items to the menu bar
    menuView->Append(menuItemToolbar);
    menuView->Append(menuItemLinks);

    menuView->Append(menuItemBudgetFinancialYears);
    menuView->Append(menuItemBudgetTransferTotal);
    menuView->AppendSeparator();
    menuView->Append(menuItemBudgetSetupWithoutSummary);
    menuView->Append(menuItemBudgetCategorySummary);
    menuView->AppendSeparator();
    menuView->Append(menuItemIgnoreFutureTransactions);
#if (wxMAJOR_VERSION >= 3 && wxMINOR_VERSION >= 0)
    wxMenuItem* menuItemToggleFullscreen = new wxMenuItem(menuView, MENU_VIEW_TOGGLE_FULLSCREEN
        , _("Toggle Fullscreen\tF11"), _("Toggle Fullscreen"), wxITEM_CHECK);
    menuView->AppendSeparator();
    menuView->Append(menuItemToggleFullscreen);
#endif
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

    wxMenu *menuRelocation = new wxMenu;
    wxMenuItem* menuItemCategoryRelocation = new wxMenuItem(menuRelocation
        , MENU_CATEGORY_RELOCATION, _("&Categories...")
        , _("Reassign all categories to another category"));
    menuItemCategoryRelocation->SetBitmap(mmBitmap(png::CATEGORY_RELOCATION));
    wxMenuItem* menuItemPayeeRelocation = new wxMenuItem(menuRelocation
        , MENU_PAYEE_RELOCATION, _("&Payees...")
        , _("Reassign all payees to another payee"));
    menuItemPayeeRelocation->SetBitmap(mmBitmap(png::PAYEE_RELOCATION));
    menuRelocation->Append(menuItemCategoryRelocation);
    menuRelocation->Append(menuItemPayeeRelocation);
    menuTools->AppendSubMenu(menuRelocation, _("Relocation of...")
        , _("Relocate Categories && Payees"));

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

    menuTools->AppendSeparator();

    wxMenuItem* menuItemTransactions = new wxMenuItem(menuTools, MENU_TRANSACTIONREPORT
        , _("&Transaction Report Filter..."), _("Transaction Report Filter"));
    menuItemTransactions->SetBitmap(mmBitmap(png::FILTER));
    menuTools->Append(menuItemTransactions);

    menuTools->AppendSeparator();

    wxMenuItem* menuItemGRM = new wxMenuItem(menuTools, wxID_VIEW_LIST
        , _("&General Report Manager..."), _("General Report Manager"));
    menuItemGRM->SetBitmap(mmBitmap(png::GRM));
    menuTools->Append(menuItemGRM);

    menuTools->AppendSeparator();

    wxMenuItem* menuItemOptions = new wxMenuItem(menuTools, wxID_PREFERENCES
        , _("&Options..."), _("Show the Options Dialog"));
    menuItemOptions->SetBitmap(mmBitmap(png::OPTIONS));
    menuTools->Append(menuItemOptions);

    menuTools->AppendSeparator();

    wxMenu *menuDatabase = new wxMenu;
    wxMenuItem* menuItemConvertDB = new wxMenuItem(menuTools, MENU_CONVERT_ENC_DB
        , _("Convert Encrypted &DB")
        , _("Convert Encrypted DB to Non-Encrypted DB"));
    menuItemConvertDB->SetBitmap(mmBitmap(png::ENCRYPT_DB));
    wxMenuItem* menuItemChangeEncryptPassword = new wxMenuItem(menuTools, MENU_CHANGE_ENCRYPT_PASSWORD
        , _("Change Encrypted &Password")
        , _("Change the password of an encrypted database"));
    menuItemChangeEncryptPassword->SetBitmap(mmBitmap(png::ENCRYPT_DB_EDIT));
    wxMenuItem* menuItemVacuumDB = new wxMenuItem(menuTools, MENU_DB_VACUUM
        , _("Optimize &Database")
        , _("Optimize database space and performance"));
    menuItemVacuumDB->SetBitmap(mmBitmap(png::EMPTY));
    wxMenuItem* menuItemCheckDB = new wxMenuItem(menuTools, MENU_DB_DEBUG
        , _("Database Debug")
        , _("Generate database report or fix errors"));
    menuItemCheckDB->SetBitmap(mmBitmap(png::EMPTY));
    menuDatabase->Append(menuItemConvertDB);
    menuDatabase->Append(menuItemChangeEncryptPassword);
    menuDatabase->Append(menuItemVacuumDB);
    menuDatabase->Append(menuItemCheckDB);
    menuTools->AppendSubMenu(menuDatabase, _("Database")
        , _("Database management"));
    menuItemChangeEncryptPassword->Enable(false);

    // Help Menu
    wxMenu *menuHelp = new wxMenu;

    wxMenuItem* menuItemHelp = new wxMenuItem(menuTools, wxID_HELP,
        _("&Help\tF1"), _("Show the Help file"));
    menuItemHelp->SetBitmap(mmBitmap(png::HELP));
    menuHelp->Append(menuItemHelp);

    wxMenuItem* menuItemAppStart = new wxMenuItem(menuTools, MENU_SHOW_APPSTART
        , _("&Show App Start Dialog"), _("App Start Dialog"));
    menuItemAppStart->SetBitmap(mmBitmap(png::APPSTART));
    menuHelp->Append(menuItemAppStart);

    menuHelp->AppendSeparator();

    wxMenuItem* menuItemCheck = new wxMenuItem(menuTools, MENU_CHECKUPDATE
        , _("Check for &Updates"), _("Check For Updates"));
    menuItemCheck->SetBitmap(mmBitmap(png::UPDATE));
    menuHelp->Append(menuItemCheck);

    wxMenuItem* menuGooglePlay = new wxMenuItem(menuTools, MENU_GOOGLEPLAY
        , _("Get Android Version")
        , _("Run this program in your Android smart phone or tablet"));
    menuGooglePlay->SetBitmap(mmBitmap(png::GOOGLE_PLAY));
    menuHelp->Append(menuGooglePlay);

    wxMenuItem* menuItemReportIssues = new wxMenuItem(menuTools, MENU_REPORTISSUES
        , _("Visit MMEX Forum")
        , _("Visit the MMEX forum. See existing user comments, or report new issues with the software."));
    menuItemReportIssues->SetBitmap(mmBitmap(png::FORUM));
    menuHelp->Append(menuItemReportIssues);

    wxMenuItem* menuItemNotify = new wxMenuItem(menuTools, MENU_ANNOUNCEMENTMAILING
        , _("Register/View Release &Notifications")
        , _("Sign up to Notification Mailing List or View existing announcements."));
    menuItemNotify->SetBitmap(mmBitmap(png::NEWS));
    menuHelp->Append(menuItemNotify);

    wxMenuItem* menuItemFacebook = new wxMenuItem(menuTools, MENU_FACEBOOK
        , _("Visit us on Facebook"), _("Visit us on Facebook"));
    menuItemFacebook->SetBitmap(mmBitmap(png::FACEBOOK));
    menuHelp->Append(menuItemFacebook);

    wxMenuItem* menuItemAbout = new wxMenuItem(menuTools, wxID_ABOUT
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

    menuBar_->Check(MENU_VIEW_BUDGET_FINANCIAL_YEARS, mmIniOptions::instance().budgetFinancialYears_);
    menuBar_->Check(MENU_VIEW_BUDGET_TRANSFER_TOTAL, mmIniOptions::instance().budgetIncludeTransfers_);
    menuBar_->Check(MENU_VIEW_BUDGET_SETUP_SUMMARY, mmIniOptions::instance().budgetSetupWithoutSummaries_);
    menuBar_->Check(MENU_VIEW_BUDGET_CATEGORY_SUMMARY, mmIniOptions::instance().budgetReportWithSummaries_);
    menuBar_->Check(MENU_VIEW_IGNORE_FUTURE_TRANSACTIONS, mmIniOptions::instance().ignoreFutureTransactions_);
}
//----------------------------------------------------------------------------

void mmGUIFrame::CreateToolBar()
{
    long style = wxTB_FLAT | wxTB_NODIVIDER;

    toolBar_ = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, style, "ToolBar");

    toolBar_->AddTool(MENU_NEW, _("New"), mmBitmap(png::NEW_DB), _("New Database"));
    toolBar_->AddTool(MENU_OPEN, _("Open"), mmBitmap(png::OPEN), _("Open Database"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(MENU_NEWACCT, _("New Account"), mmBitmap(png::NEW_ACC), _("New Account"));
    toolBar_->AddTool(MENU_ACCTLIST, _("Account List"), mmBitmap(png::HOME), _("Show Account List"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(MENU_ORGCATEGS, _("Organize Categories"), mmBitmap(png::CATEGORY), _("Show Organize Categories Dialog"));
    toolBar_->AddTool(MENU_ORGPAYEE, _("Organize Payees"), mmBitmap(png::PAYEE), _("Show Organize Payees Dialog"));
    toolBar_->AddTool(MENU_CURRENCY, _("Organize Currency"), mmBitmap(png::CURR), _("Show Organize Currency Dialog"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(MENU_TRANSACTIONREPORT, _("Transaction Report Filter"), mmBitmap(png::FILTER), _("Transaction Report Filter"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(wxID_VIEW_LIST, _("General Report Manager"), mmBitmap(png::GRM), _("General Report Manager"));
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
}

bool mmGUIFrame::createDataStore(const wxString& fileName, const wxString& pwd, bool openingNew)
{
    if (m_db)
    {
        ShutdownDatabase();
        /// Backup the database according to user requirements
        if (mmOptions::instance().databaseUpdated_ &&
            Model_Setting::instance().GetBoolSetting("BACKUPDB_UPDATE", false))
        {
            dbUpgrade::BackupDB(m_filename, dbUpgrade::BACKUPTYPE::CLOSE, Model_Setting::instance().GetIntSetting("MAX_BACKUP_FILES", 4));
            mmOptions::instance().databaseUpdated_ = false;
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

    const wxString& dialogErrorMessageHeading = _("Opening MMEX Database - Error");

    // Existing Database
    if (!openingNew
        && !fileName.IsEmpty()
        && wxFileName::FileExists(fileName)
        && passwordCheckPassed)
    {
        /* Do a backup before opening */
        if (Model_Setting::instance().GetBoolSetting("BACKUPDB", false))
        {
            dbUpgrade::BackupDB(m_filename, dbUpgrade::BACKUPTYPE::START, Model_Setting::instance().GetIntSetting("MAX_BACKUP_FILES", 4));
        }

        m_db = mmDBWrapper::Open(fileName, password);
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

        // ** OBSOLETE **
        // Mantained only for really old compatibility reason and replaced by dbupgrade.cpp
        if (!Model_Infotable::instance().checkDBVersion())
        {
            wxString note = mmex::getProgramName() + " - " + _("No File opened");
            this->SetTitle(note);
            wxMessageBox(_("Sorry. The Database version is too old or Database password is incorrect")
                , dialogErrorMessageHeading
                , wxOK | wxICON_EXCLAMATION);

            ShutdownDatabase();
            return false;
        }

        m_password = password;
    }
    else if (openingNew) // New Database
    {
        // Remove file so we can replace it instead of opening it
        if (wxFileName::FileExists(fileName))
            wxRemoveFile(fileName);

        m_db = mmDBWrapper::Open(fileName, password);
        m_commit_callback_hook = new CommitCallbackHook();
        m_db->SetCommitHook(m_commit_callback_hook);
        m_update_callback_hook = new UpdateCallbackHook();
        m_db->SetUpdateHook(m_update_callback_hook);

        m_password = password;
        dbUpgrade::InitializeVersion(m_db.get());
        InitializeModelTables();

        SetDataBaseParameters(fileName);

        mmNewDatabaseWizard* wizard = new mmNewDatabaseWizard(this);
        wizard->CenterOnParent();
        wizard->RunIt(true);
        wxButton* next = (wxButton*) wizard->FindWindow(wxID_FORWARD); //FIXME: 
        if (next) next->SetLabel(_("&Next ->"));

        /* Jump to new account creation screen */
        wxCommandEvent evt;
        OnNewAccount(evt);
        return true;
    }
    else // open of existing database failed
    {
        wxString note = mmex::getProgramName() + " - " + _("No File opened");
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
    wxString title = mmex::getProgramName() + " : " + fileName;
    if (mmex::isPortableMode())
        title << " [" << _("portable mode") << ']';

    SetTitle(title);

    if (m_db)
    {
        m_filename = fileName;
        /* Set InfoTable Options into memory */
        mmOptions::instance().LoadInfotableOptions();
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
        autoRepeatTransactionsTimer_.Start(REPEAT_TRANS_DELAY_TIME, wxTIMER_ONE_SHOT);

        if (m_db->IsEncrypted())
        {
            menuBar_->FindItem(MENU_CHANGE_ENCRYPT_PASSWORD)->Enable(true);
        }
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
        "MMB Files(*.mmb)|*.mmb",
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
        , "MMB Files(*.mmb)|*.mmb|Encrypted MMB files (*.emb)|*.emb"
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

void mmGUIFrame::OnConvertEncryptedDB(wxCommandEvent& /*event*/)
{
    wxString encFileName = wxFileSelector(_("Choose Encrypted database file to open")
        , wxEmptyString, wxEmptyString, wxEmptyString
        , "Encrypted MMB files (*.emb)|*.emb"
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
        , "MMB Files(*.mmb)|*.mmb"
        , wxFD_SAVE | wxFD_OVERWRITE_PROMPT
        );

    if (dlg.ShowModal() != wxID_OK)
        return;

    wxString fileName = dlg.GetPath();

    if (!dlg.GetPath().EndsWith(".mmb"))
        fileName += ".mmb";

    wxCopyFile(encFileName, fileName);

    wxSQLite3Database db;
    db.Open(fileName, password);
    db.ReKey(wxEmptyString);
    db.Close();

    mmErrorDialogs::MessageError(this, _("Converted DB!"), _("MMEX message"));
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnChangeEncryptPassword(wxCommandEvent& /*event*/)
{
    wxString password_change_heading = _("MMEX: Encryption Password Change");
    wxString password_message = wxString::Format(_("New password for database\n\n%s"), m_filename);

    wxString new_password = wxGetPasswordFromUser(password_message, password_change_heading);
    if (new_password.IsEmpty())
    {
        wxMessageBox(_("New password must not be empty."), password_change_heading, wxOK | wxICON_WARNING);
    }
    else
    {
        wxString confirm_password = wxGetPasswordFromUser(_("Please confirm new password"), password_change_heading);
        if (!confirm_password.IsEmpty() && (new_password == confirm_password))
        {
            m_db->ReKey(confirm_password);
            wxMessageBox(_("Password change completed."), password_change_heading);
        }
        else
        {
            wxMessageBox(_("Confirm password failed."), password_change_heading);
        }
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnVacuumDB(wxCommandEvent& /*event*/)
{
    wxMessageDialog msgDlg(this
        , wxString::Format("%s\n\n%s",_("Make sure you have a backup of DB before optimize it"),_("Do you want to proceed?"))
        , _("DB Optimization"), wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
    if (msgDlg.ShowModal() == wxID_YES)
    {
        const wxString SizeBefore = wxFileName(m_filename).GetHumanReadableSize();
        m_db->Vacuum();
        const wxString SizeAfter = wxFileName(m_filename).GetHumanReadableSize();
        wxMessageBox(wxString::Format("%s\n\n%s: %s\n%s: %s\n", _("Database Optimization Completed!"), _("Size before"), SizeBefore, _("Size after"), SizeAfter),
            _("DB Optimization"));
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnDebugDB(wxCommandEvent& /*event*/)
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
        "MMB Files(*.mmb)|*.mmb|Encrypted MMB files (*.emb)|*.emb",
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
        dbx.Open(newFileName.GetFullPath(), m_password);
        dbx.ReKey(new_password); // empty password resets encryption
        dbx.Close();
    }

    m_password.clear();
    if (openFile(newFileName.GetFullPath(), false, new_password))
    {
        updateNavTreeControl();
        createHomePage();
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnExportToCSV(wxCommandEvent& /*event*/)
{
    mmUnivCSVDialog(this, mmUnivCSVDialog::DIALOG_TYPE_EXPORT_CSV).ShowModal();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnExportToXML(wxCommandEvent& /*event*/)
{
    mmUnivCSVDialog(this, mmUnivCSVDialog::DIALOG_TYPE_EXPORT_XML).ShowModal();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnExportToQIF(wxCommandEvent& /*event*/)
{
    mmQIFExportDialog dlg(this);
    dlg.ShowModal();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnImportQIF(wxCommandEvent& /*event*/)
{

    mmQIFImportDialog dlg(this);
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

void mmGUIFrame::OnImportUniversalCSV(wxCommandEvent& /*event*/)
{
    if (Model_Account::instance().all().empty())
    {
        wxMessageBox(_("No account available to import"), _("Universal CSV Import"), wxOK | wxICON_WARNING);
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

void mmGUIFrame::OnImportXML(wxCommandEvent& /*event*/)
{
    if (Model_Account::instance().all().empty())
    {
        wxMessageBox(_("No account available to import"), _("Universal CSV Import"), wxOK | wxICON_WARNING);
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

void mmGUIFrame::OnImportWebApp(wxCommandEvent& /*event*/)
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
        updateNavTreeControl();
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

void mmGUIFrame::OnOrgCategories(wxCommandEvent& /*event*/)
{
    mmCategDialog dlg(this, false);
    dlg.ShowModal();
    if (dlg.getRefreshRequested())
    {
        refreshPanelData();
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnOrgPayees(wxCommandEvent& /*event*/)
{
    mmPayeeDialog dlg(this,false);
    dlg.ShowModal();
    if (dlg.getRefreshRequested())
    {
        refreshPanelData();
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnNewTransaction(wxCommandEvent& /*event*/)
{
    if (m_db)
    {
        if (Model_Account::instance().all_checking_account_names().empty()) return;
        mmTransDialog dlg(this, gotoAccountID_, 0);

        if (dlg.ShowModal() == wxID_OK)
        {
            gotoAccountID_ = dlg.getAccountID();
            gotoTransID_ = dlg.getTransactionID();
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

void mmGUIFrame::OnBudgetSetupDialog(wxCommandEvent& /*event*/)
{
    if (m_db)
    {
        mmBudgetYearDialog(this).ShowModal();
        updateNavTreeControl();
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnTransactionReport(wxCommandEvent& /*event*/)
{
    if (!m_db) return;
    if (Model_Account::instance().all().empty()) return;

    mmFilterTransactionsDialog* dlg = new mmFilterTransactionsDialog(this);
    if (dlg->ShowModal() == wxID_OK)
    {
        mmReportTransactions* rs = new mmReportTransactions(dlg->getAccountID(), dlg);
        createReportsPage(rs, true);
        setNavTreeSection(_("Reports"));
    }
}

void mmGUIFrame::OnGeneralReportManager(wxCommandEvent& /*event*/)
{
    if (!m_db) return;

    mmGeneralReportManager dlg(this, m_db.get());
    dlg.ShowModal();
    updateNavTreeControl();
    createHomePage(); //FIXME: refreshPanelData() crash if GRM opened then closed;
}

//----------------------------------------------------------------------------

void mmGUIFrame::OnOptions(wxCommandEvent& /*event*/)
{
    if (!m_db.get()) return;

    mmOptionsDialog systemOptions(this, this->m_app);
    if (systemOptions.ShowModal() == wxID_OK)
    {
        //set the View Menu Option items the same as the options saved.
        menuBar_->FindItem(MENU_VIEW_BUDGET_FINANCIAL_YEARS)->Check(mmIniOptions::instance().budgetFinancialYears_);
        menuBar_->FindItem(MENU_VIEW_BUDGET_TRANSFER_TOTAL)->Check(mmIniOptions::instance().budgetIncludeTransfers_);
        menuBar_->FindItem(MENU_VIEW_BUDGET_SETUP_SUMMARY)->Check(mmIniOptions::instance().budgetSetupWithoutSummaries_);
        menuBar_->FindItem(MENU_VIEW_BUDGET_CATEGORY_SUMMARY)->Check(mmIniOptions::instance().budgetReportWithSummaries_);
        menuBar_->FindItem(MENU_VIEW_IGNORE_FUTURE_TRANSACTIONS)->Check(mmIniOptions::instance().ignoreFutureTransactions_);
        menuBar_->Refresh();
        menuBar_->Update();

        updateNavTreeControl();
        createHomePage();

        const wxString& sysMsg = _("MMEX Options have been updated.") + "\n\n";
        wxMessageBox(sysMsg, _("MMEX Options"), wxOK | wxICON_INFORMATION);
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnHelp(wxCommandEvent& /*event*/)
{
    helpFileIndex_ = mmex::HTML_INDEX;
    createHelpPage();
    setNavTreeSection(_("Help"));
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnCheckUpdate(wxCommandEvent& /*event*/)
{
    mmUpdate::checkUpdates(false, this);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnGooglePlay(wxCommandEvent& /*event*/)
{
    wxLaunchDefaultBrowser(mmex::weblink::GooglePlay);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnReportIssues(wxCommandEvent& /*event*/)
{
    wxLaunchDefaultBrowser(mmex::weblink::Forum);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnBeNotified(wxCommandEvent& /*event*/)
{
    Model_Setting::instance().Set(INIDB_NEWS_LAST_READ_DATE, wxDate::Today().FormatISODate());
    wxLaunchDefaultBrowser(mmex::weblink::News);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnFacebook(wxCommandEvent& /*event*/)
{
    wxLaunchDefaultBrowser(mmex::weblink::Facebook);
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
    wxString fileName = wxFileSelector(_("Choose HTML file to Export"),
        wxEmptyString, wxEmptyString, wxEmptyString, "*.html", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
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
    json::Object o;
    o[L"module"] = json::String(L"Bills & Deposits Panel");
    o[L"start"] = json::String(wxDateTime::Now().FormatISOCombined().ToStdWstring());
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
    o[L"end"] = json::String(wxDateTime::Now().FormatISOCombined().ToStdWstring());
    Model_Usage::instance().append(o);
}
//----------------------------------------------------------------------------

void mmGUIFrame::createCheckingAccountPage(int accountID)
{
    json::Object o;
    o[L"module"] = json::String(L"Checking Panel");
    o[L"start"] = json::String(wxDateTime::Now().FormatISOCombined().ToStdWstring());
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
    o[L"end"] = json::String(wxDateTime::Now().FormatISOCombined().ToStdWstring());
    Model_Usage::instance().append(o);
    
    menuPrintingEnable(true);
    if (gotoTransID_ > 0)
    {
        checkingAccountPage_->SetSelectedTransaction(gotoTransID_);
    }
}

void mmGUIFrame::createStocksAccountPage(int accountID)
{
    json::Object o;
    o[L"module"] = json::String(L"Stock Panel");
    o[L"start"] = json::String(wxDateTime::Now().FormatISOCombined().ToStdWstring());

    //TODO: Refresh Panel
    {
        windowsFreezeThaw(homePanel_);
        wxSizer *sizer = cleanupHomePanel();
        panelCurrent_ = new mmStocksPanel(accountID, homePanel_, mmID_STOCKS);
        sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
        homePanel_->Layout();
        windowsFreezeThaw(homePanel_);
    }

    o[L"end"] = json::String(wxDateTime::Now().FormatISOCombined().ToStdWstring());
    Model_Usage::instance().append(o);
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

void mmGUIFrame::OnAssets(wxCommandEvent& /*event*/)
{
    json::Object o;
    o[L"module"] = json::String(L"Asset Panel");
    o[L"start"] = json::String(wxDateTime::Now().FormatISOCombined().ToStdWstring());
    windowsFreezeThaw(homePanel_);
    wxSizer *sizer = cleanupHomePanel();
    panelCurrent_ = new mmAssetsPanel(homePanel_, mmID_ASSETS);
    sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
    homePanel_->Layout();
    windowsFreezeThaw(homePanel_);
    menuPrintingEnable(true);
    o[L"end"] = json::String(wxDateTime::Now().FormatISOCombined().ToStdWstring());
    Model_Usage::instance().append(o);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnCurrency(wxCommandEvent& /*event*/)
{
    mmMainCurrencyDialog(this, false, false).ShowModal();
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
        if (dlg.ShowModal() == wxID_OK)
        {
            updateNavTreeControl();
            createHomePage();
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

void mmGUIFrame::OnViewBankAccounts(wxCommandEvent &event)
{
    updateNavTreeControl();
}

void mmGUIFrame::OnViewTermAccounts(wxCommandEvent &event)
{
    updateNavTreeControl();
}

void mmGUIFrame::OnViewStockAccounts(wxCommandEvent &event)
{
    updateNavTreeControl();
}

void mmGUIFrame::OnViewBudgetFinancialYears(wxCommandEvent &event)
{
    mmIniOptions::instance().budgetFinancialYears_ = !mmIniOptions::instance().budgetFinancialYears_;
    refreshPanelData();
}

void mmGUIFrame::OnViewBudgetTransferTotal(wxCommandEvent &event)
{
    mmIniOptions::instance().budgetIncludeTransfers_ = !mmIniOptions::instance().budgetIncludeTransfers_;
    refreshPanelData();
}

void mmGUIFrame::OnViewBudgetSetupSummary(wxCommandEvent &event)
{
    mmIniOptions::instance().budgetSetupWithoutSummaries_ = !mmIniOptions::instance().budgetSetupWithoutSummaries_;
    refreshPanelData();
}

void mmGUIFrame::OnViewBudgetCategorySummary(wxCommandEvent &event)
{
    mmIniOptions::instance().budgetReportWithSummaries_ = !mmIniOptions::instance().budgetReportWithSummaries_;
    refreshPanelData();
}

void mmGUIFrame::OnViewIgnoreFutureTransactions(wxCommandEvent &event)
{
    mmIniOptions::instance().ignoreFutureTransactions_ = !mmIniOptions::instance().ignoreFutureTransactions_;
    updateNavTreeControl();
    createHomePage();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnCategoryRelocation(wxCommandEvent& /*event*/)
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

void mmGUIFrame::OnPayeeRelocation(wxCommandEvent& /*event*/)
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

void mmGUIFrame::SetDatabaseFile(const wxString& dbFileName, bool newDatabase)
{
    autoRepeatTransactionsTimer_.Stop();

    if (openFile(dbFileName, newDatabase))
    {
        updateNavTreeControl();
        createHomePage();
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

void mmGUIFrame::OnClose(wxCloseEvent&)
{
    Destroy();
}
