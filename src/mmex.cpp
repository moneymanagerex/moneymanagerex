/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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
#include <wx/cmdline.h>

#include "mmex.h"
#include "guiid.h"
#include "dbwrapper.h"
#include "aboutdialog.h"
#include "appstartdialog.h"
#include "assetspanel.h"
#include "billsdepositsdialog.h"
#include "billsdepositspanel.h"
#include "budgetingpanel.h"
#include "budgetyeardialog.h"
#include "categdialog.h"
#include "constants.h"
#include "currencydialog.h"
#include "customreportdialog.h"
#include "filtertransdialog.h"
#include "images_list.h"
#include "maincurrencydialog.h"
#include "mmcheckingpanel.h"
#include "mmhelppanel.h"
#include "mmhomepagepanel.h"
#include "mmreportspanel.h"
#include "accountdialog.h"
#include "optionsdialog.h"
#include "paths.h"
#include "payeedialog.h"
#include "platfdep.h"
#include "relocatecategorydialog.h"
#include "relocatepayeedialog.h"
#include "recentfiles.h"
#include "stockspanel.h"
#include "transdialog.h"
#include "reports/budgetcategorysummary.h"
#include "reports/budgetingperf.h"
#include "reports/cashflow.h"
#include "reports/categexp.h"
#include "reports/categovertimeperf.h"
#include "reports/incexpenses.h"
#include "reports/htmlbuilder.h"
#include "reports/payee.h"
#include "reports/summary.h"
#include "reports/summaryassets.h"
#include "reports/summarystocks.h"
#include "reports/transactions.h"
#include "import_export/qif_export.h"
#include "import_export/qif_import.h"
#include "import_export/univcsvdialog.h"
#include "model/Model_Asset.h"
#include "model/Model_Stock.h"
#include "model/Model_Infotable.h"
#include "model/Model_Setting.h"
#include "model/Model_Budgetyear.h"
#include "model/Model_Account.h"
#include "model/Model_Payee.h"
#include "model/Model_Checking.h"
#include "model/Model_Category.h"
#include "model/Model_Subcategory.h"
#include "model/Model_Billsdeposits.h"
#include "model/Model_Splittransaction.h"
#include "model/Model_Budget.h"
#include "model/Model_Report.h"
#include "wizard_newdb.h"
#include "wizard_newaccount.h"

#include "../resources/house.xpm"
#include "../resources/money_dollar.xpm"
#include "../resources/car.xpm"

//----------------------------------------------------------------------------

int REPEAT_TRANS_DELAY_TIME = 7000; // 7 seconds
//----------------------------------------------------------------------------
IMPLEMENT_APP(mmGUIApp)
//----------------------------------------------------------------------------

static const wxCmdLineEntryDesc g_cmdLineDesc [] =
{
    { wxCMD_LINE_PARAM,  NULL, NULL, _("database file"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
    { wxCMD_LINE_NONE }
};

//----------------------------------------------------------------------------

mmGUIApp::mmGUIApp(): m_frame(0), m_setting_db(0), m_optParam("")
{
#if wxUSE_ON_FATAL_EXCEPTION
    // catch fatal exceptions
    wxHandleFatalExceptions(true);
#endif
}

wxLocale& mmGUIApp::getLocale()
{
    return this->m_locale;
}

void mmGUIApp::OnInitCmdLine(wxCmdLineParser& parser)
{
    parser.SetDesc (g_cmdLineDesc);
}

bool mmGUIApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
    if(parser.GetParamCount() > 0)
        m_optParam = parser.GetParam(0);
    return true;
}

//----------------------------------------------------------------------------
/*
    See also: wxStackWalker, wxDebugReportUpload.
*/
void mmGUIApp::reportFatalException(wxDebugReport::Context ctx)
{
    wxDebugReportCompress rep;

    if (!rep.IsOk())
    {
        wxSafeShowMessage(mmex::getProgramName(), _("Fatal error occured.\nApplication will be terminated."));
        return;
    }

    rep.AddAll(ctx);

    wxDebugReportPreviewStd preview;

    if (preview.Show(rep) && rep.Process()) {
        rep.Reset();
    }
}
/*
    This method allows catching the exceptions thrown by any event handler.
*/
void mmGUIApp::HandleEvent(wxEvtHandler *handler, wxEventFunction func, wxEvent& event) const
{
    try
    {
        wxApp::HandleEvent(handler, func, event);
    }
    catch (const wxSQLite3Exception &e)
    {
        wxLogError(e.GetMessage());
    }
    catch (const std::exception &e)
    {
        wxLogError("%s", e.what());
    }
}
//----------------------------------------------------------------------------

void mmGUIApp::OnFatalException()
{
    reportFatalException(wxDebugReport::Context_Exception);
}
//----------------------------------------------------------------------------

bool OnInitImpl(mmGUIApp* app)
{
    app->SetAppName(mmex::GetAppName());

    /* Setting Locale causes unexpected problems, so default to English Locale */
    app->getLocale().Init(wxLANGUAGE_ENGLISH);

    /* Initialize Image Handlers */
    wxImage::AddHandler(new wxICOHandler());
    wxImage::AddHandler(new wxJPEGHandler());
    wxImage::AddHandler(new wxPNGHandler());

    app->m_setting_db = new wxSQLite3Database();
    app->m_setting_db->Open(mmex::getPathUser(mmex::SETTINGS));
    Model_Setting::instance(app->m_setting_db);

    /* Force setting MMEX language parameter if it has not been set. */
    mmSelectLanguage(0, !Model_Setting::instance().ContainsSetting(LANGUAGE_PARAMETER));

    /* Load Colors from Database */
    mmLoadColorsFromDatabase();

    /* Load MMEX Custom Settings */
    mmIniOptions::instance().loadOptions();

    /* Was App Maximized? */
    bool isMax = Model_Setting::instance().GetBoolSetting("ISMAXIMIZED", false);

    //Get System screen size
    int sys_screen_x = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
    int sys_screen_y = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);

    /* Load Dimensions of Window */
    int valx = Model_Setting::instance().GetIntSetting("ORIGINX", 50);
    int valy = Model_Setting::instance().GetIntSetting("ORIGINY", 50);
    int valw = Model_Setting::instance().GetIntSetting("SIZEW", sys_screen_x/4*3);
    int valh = Model_Setting::instance().GetIntSetting("SIZEH", sys_screen_y/4*3);

    //BUGFIX: #214 MMEX Window is "off screen"
    if (valx >= sys_screen_x ) valx = sys_screen_x - valw;
    if (valy >= sys_screen_y ) valy = sys_screen_y - valh;

    app->m_frame = new mmGUIFrame(mmex::getProgramName(), wxPoint(valx, valy), wxSize(valw, valh));
    bool ok = app->m_frame->Show();

    if (isMax) app->m_frame->Maximize(true);

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned FALSE here, the
    // application would exit immediately.
    return ok;
}

//----------------------------------------------------------------------------
bool mmGUIApp::OnInit()
{
    bool ok = false;

    try
    {
        ok = wxApp::OnInit() && OnInitImpl(this);
    }
    catch (const wxSQLite3Exception &e)
    {
        wxLogError(e.GetMessage());
    }
    catch (const std::exception &e)
    {
        wxLogError("%s", e.what());
    }

    return ok;
}

int mmGUIApp::OnExit()
{
    // Report database statistics
    Model_Account::instance().show_statistics();
    Model_Asset::instance().show_statistics();
    Model_Billsdeposits::instance().show_statistics();
    Model_Budgetsplittransaction::instance().show_statistics();
    Model_Budget::instance().show_statistics();
    Model_Budgetyear::instance().show_statistics();
    Model_Category::instance().show_statistics();
    Model_Checking::instance().show_statistics();
    Model_Currency::instance().show_statistics();
    Model_Infotable::instance().show_statistics();
    Model_Payee::instance().show_statistics();
    Model_Setting::instance().show_statistics();
    Model_Splittransaction::instance().show_statistics();
    Model_Stock::instance().show_statistics();
    Model_Subcategory::instance().show_statistics();

    if (m_setting_db) delete m_setting_db;

    return 0;
}
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(mmGUIFrame, wxFrame)
    EVT_MENU(MENU_NEW, mmGUIFrame::OnNew)
    EVT_MENU(MENU_OPEN, mmGUIFrame::OnOpen)
    EVT_MENU(MENU_SAVE_AS, mmGUIFrame::OnSaveAs)
    EVT_MENU(MENU_CONVERT_ENC_DB, mmGUIFrame::OnConvertEncryptedDB)
    EVT_MENU(MENU_EXPORT_CSV, mmGUIFrame::OnExportToCSV)
    EVT_MENU(MENU_EXPORT_QIF, mmGUIFrame::OnExportToQIF)
    EVT_MENU(MENU_IMPORT_QIF, mmGUIFrame::OnImportQIF)
    EVT_MENU(MENU_IMPORT_UNIVCSV, mmGUIFrame::OnImportUniversalCSV)
    EVT_MENU(wxID_EXIT,  mmGUIFrame::OnQuit)
    EVT_MENU(MENU_NEWACCT,  mmGUIFrame::OnNewAccount)
    EVT_MENU(MENU_ACCTLIST,  mmGUIFrame::OnAccountList)
    EVT_MENU(MENU_ACCTEDIT,  mmGUIFrame::OnEditAccount)
    EVT_MENU(MENU_ACCTDELETE,  mmGUIFrame::OnDeleteAccount)
    EVT_MENU(MENU_ORGCATEGS,  mmGUIFrame::OnOrgCategories)
    EVT_MENU(MENU_ORGPAYEE,  mmGUIFrame::OnOrgPayees)
    EVT_MENU(wxID_PREFERENCES,  mmGUIFrame::OnOptions)
    EVT_MENU(wxID_NEW,  mmGUIFrame::OnNewTransaction)
    EVT_MENU(MENU_BUDGETSETUPDIALOG, mmGUIFrame::OnBudgetSetupDialog)
    EVT_MENU(wxID_HELP,  mmGUIFrame::OnHelp)
    EVT_MENU(MENU_CHECKUPDATE,  mmGUIFrame::OnCheckUpdate)
    EVT_MENU(MENU_REPORTISSUES,  mmGUIFrame::OnReportIssues)
    EVT_MENU(MENU_ANNOUNCEMENTMAILING,  mmGUIFrame::OnBeNotified)
    EVT_MENU(MENU_FACEBOOK,  mmGUIFrame::OnFacebook)
    EVT_MENU(wxID_ABOUT, mmGUIFrame::OnAbout)
    EVT_MENU(wxID_PAGE_SETUP, mmGUIFrame::OnPrintPageSetup)
    EVT_MENU(MENU_PRINT_REPORT, mmGUIFrame::OnPrintPageReport)
    EVT_MENU(MENU_PRINT_PREVIEW_REPORT, mmGUIFrame::OnPrintPagePreview)
    EVT_MENU(MENU_SHOW_APPSTART, mmGUIFrame::OnShowAppStartDialog)
    EVT_MENU(MENU_EXPORT_HTML, mmGUIFrame::OnExportToHtml)
    EVT_MENU(MENU_BILLSDEPOSITS, mmGUIFrame::OnBillsDeposits)

    EVT_MENU(MENU_ASSETS, mmGUIFrame::OnAssets)
    EVT_MENU(MENU_CURRENCY, mmGUIFrame::OnCurrency)
    EVT_MENU(MENU_TRANSACTIONREPORT, mmGUIFrame::OnTransactionReport)
    EVT_MENU(wxID_VIEW_LIST, mmGUIFrame::OnGeneralReportManager)
    EVT_MENU(MENU_TREEPOPUP_LAUNCHWEBSITE, mmGUIFrame::OnLaunchAccountWebsite)
    EVT_MENU(MENU_VIEW_TOOLBAR, mmGUIFrame::OnViewToolbar)
    EVT_MENU(MENU_VIEW_LINKS, mmGUIFrame::OnViewLinks)
    EVT_MENU(MENU_VIEW_BANKACCOUNTS, mmGUIFrame::OnViewBankAccounts)
    EVT_MENU(MENU_VIEW_TERMACCOUNTS, mmGUIFrame::OnViewTermAccounts)
    EVT_MENU(MENU_VIEW_STOCKACCOUNTS, mmGUIFrame::OnViewStockAccounts)
    EVT_MENU(MENU_CATEGORY_RELOCATION, mmGUIFrame::OnCategoryRelocation)
    EVT_MENU(MENU_PAYEE_RELOCATION, mmGUIFrame::OnPayeeRelocation)

    // Added for easier ability to test new feature.
    // May be taken out in future after being added to Options Dialog.
    EVT_MENU(MENU_IGNORE_FUTURE_TRANSACTIONS, mmGUIFrame::OnIgnoreFutureTransactions)

    EVT_UPDATE_UI(MENU_VIEW_TOOLBAR, mmGUIFrame::OnViewToolbarUpdateUI)
    EVT_UPDATE_UI(MENU_VIEW_LINKS, mmGUIFrame::OnViewLinksUpdateUI)
    EVT_MENU(MENU_TREEPOPUP_EDIT, mmGUIFrame::OnPopupEditAccount)
    EVT_MENU(MENU_TREEPOPUP_DELETE, mmGUIFrame::OnPopupDeleteAccount)

    EVT_TREE_ITEM_MENU(wxID_ANY, mmGUIFrame::OnItemMenu)
    //EVT_TREE_ITEM_RIGHT_CLICK(ID_NAVTREECTRL, mmGUIFrame::OnItemRightClick)
    EVT_TREE_ITEM_ACTIVATED(ID_NAVTREECTRL, mmGUIFrame::OnItemRightClick)
    EVT_TREE_SEL_CHANGED(ID_NAVTREECTRL, mmGUIFrame::OnSelChanged)
    EVT_TREE_ITEM_EXPANDED(ID_NAVTREECTRL, mmGUIFrame::OnTreeItemExpanded)
    EVT_TREE_ITEM_COLLAPSED(ID_NAVTREECTRL,mmGUIFrame::OnTreeItemCollapsed)

    EVT_MENU(MENU_GOTOACCOUNT, mmGUIFrame::OnGotoAccount)
    EVT_MENU(MENU_STOCKS, mmGUIFrame::OnGotoStocksAccount)

    /* Navigation Panel */
    EVT_MENU(MENU_TREEPOPUP_ACCOUNT_NEW, mmGUIFrame::OnNewAccount)
    EVT_MENU(MENU_TREEPOPUP_ACCOUNT_DELETE, mmGUIFrame::OnDeleteAccount)
    EVT_MENU(MENU_TREEPOPUP_ACCOUNT_EDIT, mmGUIFrame::OnEditAccount)
    EVT_MENU(MENU_TREEPOPUP_ACCOUNT_LIST, mmGUIFrame::OnAccountList)
    EVT_MENU(MENU_TREEPOPUP_ACCOUNT_EXPORT2CSV, mmGUIFrame::OnExportToCSV)
    EVT_MENU(MENU_TREEPOPUP_ACCOUNT_EXPORT2QIF, mmGUIFrame::OnExportToQIF)
    //EVT_MENU(MENU_TREEPOPUP_ACCOUNT_IMPORTQIF, mmGUIFrame::OnImportQIF)
    EVT_MENU(MENU_TREEPOPUP_ACCOUNT_IMPORTUNIVCSV, mmGUIFrame::OnImportUniversalCSV)
    EVT_MENU(MENU_TREEPOPUP_ACCOUNT_VIEWALL, mmGUIFrame::OnViewAllAccounts)
    EVT_MENU(MENU_TREEPOPUP_ACCOUNT_VIEWFAVORITE, mmGUIFrame::OnViewFavoriteAccounts)
    EVT_MENU(MENU_TREEPOPUP_ACCOUNT_VIEWOPEN, mmGUIFrame::OnViewOpenAccounts)

    /*Automatic processing of repeat transactions*/
    EVT_TIMER(AUTO_REPEAT_TRANSACTIONS_TIMER_ID, mmGUIFrame::OnAutoRepeatTransactionsTimer)

    /* Recent Files */
    EVT_MENU_RANGE(wxID_FILE1, wxID_FILE5, mmGUIFrame::OnRecentFiles)
    EVT_MENU(MENU_RECENT_FILES_CLEAR, mmGUIFrame::OnClearRecentFiles)

END_EVENT_TABLE()
//----------------------------------------------------------------------------

mmGUIFrame::mmGUIFrame(const wxString& title
    , const wxPoint& pos
    , const wxSize& size)
: wxFrame(0, -1, title, pos, size)
, m_commit_callback_hook()
, gotoAccountID_(-1)
, gotoTransID_(-1)
, homePageAccountSelect_(false)
, checkingAccountPage_(0)
, activeCheckingAccountPage_(false)
, budgetingPage_(0)
, activeBudgetingPage_(false)
, autoRepeatTransactionsTimer_(this, AUTO_REPEAT_TRANSACTIONS_TIMER_ID)
, initHomePage_(false)
, activeHomePage_(false)
, refreshRequested_()
, panelCurrent_()
, homePanel_()
, navTreeCtrl_()
, menuBar_()
, toolBar_()
, selectedItemData_()
, helpFileIndex_(mmex::HTML_INDEX)
, expandedReportNavTree_(true)
, expandedBudgetingNavTree_(true)
{
    // tell wxAuiManager to manage this frame
    m_mgr.SetManagedWindow(this);
    SetIcon(mmex::getProgramIcon());
    SetMinSize(wxSize(480,275));

    /* Setup Printer */
    printer_.reset(new wxHtmlEasyPrinting(mmex::getProgramName(), this));
    printer_->SetFooter("<center>@PAGENUM@/@PAGESCNT@</center>", wxPAGE_ALL);
    restorePrinterValues();

    // decide if we need to show app start dialog
    bool from_scratch = false;
    wxFileName dbpath = wxGetApp().m_optParam;
    if(!dbpath.IsOk())
    {
        from_scratch = Model_Setting::instance().GetBoolSetting("SHOWBEGINAPP", true);
        if (from_scratch)
            dbpath  = wxGetEmptyString();
        else
            dbpath = Model_Setting::instance().getLastDbPath();
    }

    /* Create the Controls for the frame */
    createMenu();
    CreateToolBar();
    createControls();
#if wxUSE_STATUSBAR
    CreateStatusBar();
#endif // wxUSE_STATUSBAR
    recentFiles_ = new RecentDatabaseFiles(menuRecentFiles_);

    // Load perspective
    wxString auiPerspective = Model_Setting::instance().GetStringSetting("AUIPERSPECTIVE", m_perspective);
    m_mgr.LoadPerspective(auiPerspective);

    // add the toolbars to the manager
    m_mgr.AddPane(toolBar_, wxAuiPaneInfo().
        Name("toolbar").Caption(_("Toolbar")).ToolbarPane().Top()
        .LeftDockable(false).RightDockable(false).MinSize(1000,-1)
        .Show(Model_Setting::instance().GetBoolSetting("SHOWTOOLBAR", true)));

    // change look and feel of wxAuiManager
    m_mgr.GetArtProvider()->SetMetric(16, 0);
    m_mgr.GetArtProvider()->SetMetric(3, 1);

    // "commit" all changes made to wxAuiManager
    m_mgr.Update();

    if (from_scratch || !dbpath.IsOk())
    {
        menuEnableItems(false);
        showBeginAppDialog(dbpath.GetFullName().IsEmpty());
    }
    else
    {
        if (!openFile(dbpath.GetFullPath(), false))
            showBeginAppDialog(true);
    }

    const wxAcceleratorEntry entries[] =
    {
        wxAcceleratorEntry(wxACCEL_NORMAL, WXK_F9, wxID_NEW),
    };

    wxAcceleratorTable tab(sizeof(entries)/sizeof(*entries), entries);
    SetAcceleratorTable(tab);

}
//----------------------------------------------------------------------------

mmGUIFrame::~mmGUIFrame()
{
    try {
        cleanup();
    } catch (...) {
        wxASSERT(false);
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::cleanup()
{
    printer_.reset();
    delete recentFiles_;
    if(!fileName_.IsEmpty()) // Exiting before file is opened
        saveSettings();

    wxTreeItemId rootitem = navTreeCtrl_->GetRootItem();
    cleanupNavTreeControl(rootitem);
    m_mgr.UnInit();

    /* Delete the GUI */
    cleanupHomePanel(false);
    ShutdownDatabase();
    /// Update the database according to user requirements
    if (mmOptions::instance().databaseUpdated_ && Model_Setting::instance().GetBoolSetting("BACKUPDB_UPDATE", false))
    {
        BackupDatabase(fileName_, true);
    }
}

void mmGUIFrame::ShutdownDatabase()
{
    if (m_db)
    {
        m_db->SetCommitHook(NULL);
        m_db->Close();
        delete m_commit_callback_hook;
        m_db.reset();
    }
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
    while (wxGetApp().Pending())
    {
        wxGetApp().Dispatch();
    }
}

//----------------------------------------------------------------------------
// returns a wxTreeItemID for the accountName in the navtree section.
wxTreeItemId mmGUIFrame::getTreeItemfor(wxTreeItemId itemID, const wxString& accountName) const
{
    wxTreeItemIdValue treeDummyValue;
    wxTreeItemId navTreeID = navTreeCtrl_->GetFirstChild(itemID, treeDummyValue);

    bool searching = true;
    while (navTreeID.IsOk() && searching)
    {
        if ( accountName == navTreeCtrl_->GetItemText(navTreeID))
            searching = false;
        else
            navTreeID = navTreeCtrl_->GetNextChild(itemID, treeDummyValue);
    }
    return navTreeID;
}

//----------------------------------------------------------------------------
bool mmGUIFrame::setAccountInSection(const wxString& sectionName, const wxString& accountName)
{
    bool accountNotFound = true;
    wxTreeItemId rootItem = getTreeItemfor(navTreeCtrl_->GetRootItem(), sectionName );
    if (rootItem.IsOk() && navTreeCtrl_->ItemHasChildren(rootItem))
    {
        navTreeCtrl_->ExpandAllChildren(rootItem);
        wxTreeItemId accountItem = getTreeItemfor(rootItem, accountName);
        if (accountItem.IsOk())
        {
            // Set the NavTreeCtrl and prevent any event code being executed for now.
            homePageAccountSelect_ = true;
            navTreeCtrl_->SelectItem(accountItem);
            processPendingEvents();
            homePageAccountSelect_ = false;
            accountNotFound = false;
        }
    }
    return accountNotFound;
}

//----------------------------------------------------------------------------
bool mmGUIFrame::setNavTreeSection( wxString sectionName)
{
    bool accountNotFound = true;
    wxTreeItemId rootItem = getTreeItemfor(navTreeCtrl_->GetRootItem(), sectionName );
    if (rootItem.IsOk())
    {
        // Set the NavTreeCtrl and prevent any event code being executed for now.
        homePageAccountSelect_ = true;
        navTreeCtrl_->SelectItem(rootItem);
        processPendingEvents();
        homePageAccountSelect_ = false;
        accountNotFound = false;
    }
    return accountNotFound;
}

//----------------------------------------------------------------------------
void mmGUIFrame::setAccountNavTreeSection(const wxString& accountName)
{
    if ( setAccountInSection(_("Bank Accounts"), accountName))
        if (setAccountInSection(_("Term Accounts"), accountName))
            setAccountInSection(_("Stocks"), accountName);
}

//----------------------------------------------------------------------------
void mmGUIFrame::setHomePageActive(bool active)
{
    activeHomePage_ = active;
}

//----------------------------------------------------------------------------
void mmGUIFrame::OnAutoRepeatTransactionsTimer(wxTimerEvent& /*event*/)
{
    bool continueExecution = false;

    Model_Billsdeposits& bills = Model_Billsdeposits::instance();
    for (const auto& q1 : bills.all())
    {
        bills.decode_fields(q1);

        if (bills.autoExecuteManual() && bills.requireExecution())
        {
            if (bills.allowExecution())
            {
                continueExecution = true;
                mmBDDialog repeatTransactionsDlg(this, q1.BDID, false, true);
                repeatTransactionsDlg.SetDialogHeader(_(" Auto Repeat Transactions"));
                if ( repeatTransactionsDlg.ShowModal() == wxID_OK )
                {
                    if (activeHomePage_) createHomePage();
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

                tran->ACCOUNTID         = q1.ACCOUNTID;
                tran->TOACCOUNTID       = q1.TOACCOUNTID;
                tran->PAYEEID           = q1.PAYEEID;
                tran->TRANSCODE         = q1.TRANSCODE;
                tran->TRANSAMOUNT       = q1.TRANSAMOUNT;
                tran->TOTRANSAMOUNT     = q1.TOTRANSAMOUNT;
                tran->STATUS            = q1.STATUS;
                tran->TRANSACTIONNUMBER = q1.TRANSACTIONNUMBER;
                tran->NOTES             = q1.NOTES;
                tran->CATEGID           = q1.CATEGID;
                tran->SUBCATEGID        = q1.SUBCATEGID;
                tran->TRANSDATE         = q1.NEXTOCCURRENCEDATE;

                int transID = Model_Checking::instance().save(tran);

                Model_Splittransaction::Data_Set checking_splits;
                for (const auto &item : Model_Billsdeposits::splittransaction(q1))
                {
                    Model_Splittransaction::Data *split = Model_Splittransaction::instance().create();
                    split->TRANSID = transID;
                    split->CATEGID = item.CATEGID;
                    split->SUBCATEGID = item.SUBCATEGID;
                    split->SPLITTRANSAMOUNT = item.SPLITTRANSAMOUNT;
                    checking_splits.push_back(*split);
                }
                Model_Splittransaction::instance().save(checking_splits);
            }
            Model_Billsdeposits::instance().completeBDInSeries(q1.BDID);

            if (activeHomePage_)
            {
                createHomePage(); // Update home page details only if it is being displayed
            }
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
    Model_Setting::instance().Begin();
    if (! fileName_.IsEmpty())
    {
        wxFileName fname(fileName_);
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
    Model_Setting::instance().Commit();
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
    menuBar_->FindItem(MENU_CONVERT_ENC_DB)->Enable(enable);

    menuBar_->FindItem(MENU_IMPORT)->Enable(enable);
    menuBar_->FindItem(wxID_PREVIEW)->Enable(enable);
    menuBar_->FindItem(wxID_PRINT)->Enable(enable);
    menuBar_->FindItem(wxID_PREFERENCES)->Enable(enable);
    if (mmIniOptions::instance().enableRepeatingTransactions_)
        menuBar_->FindItem(MENU_BILLSDEPOSITS)->Enable(enable);
    menuBar_->FindItem(MENU_CURRENCY)->Enable(enable);
    if (mmIniOptions::instance().enableAssets_)
        menuBar_->FindItem(MENU_ASSETS)->Enable(enable);
    if (mmIniOptions::instance().enableBudget_)
        menuBar_->FindItem(MENU_BUDGETSETUPDIALOG)->Enable(enable);
    menuBar_->FindItem(MENU_TRANSACTIONREPORT)->Enable(enable);

    toolBar_->EnableTool(MENU_NEWACCT, enable);
    toolBar_->EnableTool(MENU_ACCTLIST, enable);
    toolBar_->EnableTool(MENU_ORGPAYEE, enable);
    toolBar_->EnableTool(MENU_ORGCATEGS, enable);
    toolBar_->EnableTool(MENU_CURRENCY, enable);
}
//----------------------------------------------------------------------------

void mmGUIFrame::menuPrintingEnable(bool enable)
{
    menuBar_->FindItem(wxID_PREVIEW)->Enable(enable);
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
    navTreeCtrl_ = new wxTreeCtrl( this, ID_NAVTREECTRL,
        wxDefaultPosition, wxSize(100, 100), wxTR_SINGLE | wxTR_HAS_BUTTONS | wxTR_ROW_LINES );
#endif

    navTreeCtrl_->AssignImageList(navtree_images_list());

    homePanel_ = new wxPanel( this, wxID_ANY,
        wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxTR_SINGLE | wxNO_BORDER);

    m_mgr.AddPane(navTreeCtrl_, wxAuiPaneInfo().
        Name("Navigation").Caption(_("Navigation")).
        BestSize(wxSize(200,100)).MinSize(wxSize(100,100)).
        Left());

    m_mgr.AddPane(homePanel_, wxAuiPaneInfo().
        Name("Home").Caption("Home").
        CenterPane().PaneBorder(false));
}
//----------------------------------------------------------------------------

void mmGUIFrame::updateNavTreeControl(bool expandTermAccounts)
{
    navTreeCtrl_->SetEvtHandlerEnabled(false);
    wxTreeItemId root = navTreeCtrl_->GetRootItem();
    cleanupNavTreeControl(root);
    navTreeCtrl_->DeleteAllItems();
    //navTreeCtrl_->SetBackgroundColour(mmColors::navTreeBkColor);

    root = navTreeCtrl_->AddRoot(_("Home Page"), 0, 0);
    navTreeCtrl_->SetItemData(root, new mmTreeItemData("Home Page"));
    navTreeCtrl_->SetItemBold(root, true);
    navTreeCtrl_->SetFocus();


    wxTreeItemId accounts = navTreeCtrl_->AppendItem(root, _("Bank Accounts"), 9, 9);
    navTreeCtrl_->SetItemData(accounts, new mmTreeItemData("Bank Accounts"));
    navTreeCtrl_->SetItemBold(accounts, true);

    wxTreeItemId termAccount;
    if ( Model_Account::hasActiveTermAccount() )
    {
    //  Positioning for new type of accounts: Term Accounts
        termAccount = navTreeCtrl_->AppendItem(root, _("Term Accounts"), 12, 12);
        navTreeCtrl_->SetItemData(termAccount, new mmTreeItemData("Term Accounts"));
        navTreeCtrl_->SetItemBold(termAccount, true);
    }

    wxTreeItemId stocks;
    stocks = navTreeCtrl_->AppendItem(root, _("Stocks"), 15, 15);
    navTreeCtrl_->SetItemData(stocks, new mmTreeItemData("Stocks"));
    navTreeCtrl_->SetItemBold(stocks, true);

    if (mmIniOptions::instance().enableAssets_)
    {
        wxTreeItemId assets = navTreeCtrl_->AppendItem(root, _("Assets"), 7, 7);
        navTreeCtrl_->SetItemData(assets, new mmTreeItemData("Assets"));
        navTreeCtrl_->SetItemBold(assets, true);
    }

    if (mmIniOptions::instance().enableRepeatingTransactions_)
    {
       wxTreeItemId bills = navTreeCtrl_->AppendItem(root, _("Repeating Transactions"), 2, 2);
       navTreeCtrl_->SetItemData(bills, new mmTreeItemData("Bills & Deposits"));
       navTreeCtrl_->SetItemBold(bills, true);
    }

    wxTreeItemId budgeting;
    if (mmIniOptions::instance().enableBudget_)
    {
        budgeting = navTreeCtrl_->AppendItem(root, _("Budget Setup"), 3, 3);
        navTreeCtrl_->SetItemData(budgeting, new mmTreeItemData("Budgeting"));
        navTreeCtrl_->SetItemBold(budgeting, true);
    }

    wxTreeItemId reports = navTreeCtrl_->AppendItem(root, _("Reports"), 4, 4);
    navTreeCtrl_->SetItemBold(reports, true);
    navTreeCtrl_->SetItemData(reports, new mmTreeItemData(NAVTREECTRL_REPORTS));

    /* ================================================================================================= */

    wxTreeItemId reportsSummary = navTreeCtrl_->AppendItem(reports, _("Summary of Accounts"), 4, 4);
    navTreeCtrl_->SetItemData(reportsSummary, new mmTreeItemData("Summary of Accounts"
        , new mmReportSummary()));

    wxTreeItemId reportsStocks = navTreeCtrl_->AppendItem(reportsSummary, _("Stocks"), 4, 4);
    navTreeCtrl_->SetItemData(reportsStocks, new mmTreeItemData("Summary of Stocks"
        , new mmReportSummaryStocks()));

    if (mmIniOptions::instance().enableAssets_)
    {
        wxTreeItemId reportsAssets = navTreeCtrl_->AppendItem(reportsSummary, _("Assets"), 4, 4);
        navTreeCtrl_->SetItemData(reportsAssets, new mmTreeItemData("Summary of Assets"
        , new mmReportSummaryAssets()));
    }

    /* ================================================================================================= */
    bool ignoreFuture = mmIniOptions::instance().ignoreFutureTransactions_;
    wxTreeItemId categsOverTime = navTreeCtrl_->AppendItem(reports
        , _("Where the Money Goes"), 4, 4);
    navTreeCtrl_->SetItemData(categsOverTime, new mmTreeItemData("Where the Money Goes"
        , new mmReportCategoryExpensesGoes()));

    wxTreeItemId categsOverTimeCalMonth = navTreeCtrl_->AppendItem(categsOverTime
        , _("Last Calendar Month"), 4, 4);
    navTreeCtrl_->SetItemData(categsOverTimeCalMonth
        , new mmTreeItemData("Where the Money Goes - Last Calendar Month"
        , new mmReportCategoryExpensesGoesLastMonth()));

    if (ignoreFuture)
    {
        wxTreeItemId categsOverTimeCurrentMonth = navTreeCtrl_->AppendItem(categsOverTime
            , _("Current Month to Date"), 4, 4);
        navTreeCtrl_->SetItemData(categsOverTimeCurrentMonth
            , new mmTreeItemData("Where the Money Goes - Current Month"
            , new mmReportCategoryExpensesGoesCurrentMonthToDate()));
    }
    else
    {
        wxTreeItemId categsOverTimeCurrentMonth = navTreeCtrl_->AppendItem(categsOverTime
            , _("Current Month"), 4, 4);
        navTreeCtrl_->SetItemData(categsOverTimeCurrentMonth
            , new mmTreeItemData("Where the Money Goes - Current Month"
            , new mmReportCategoryExpensesGoesCurrentMonth()));
    }

    wxTreeItemId categsOverTimeLast30 = navTreeCtrl_->AppendItem(categsOverTime
        , _("Last 30 Days"), 4, 4);
    navTreeCtrl_->SetItemData(categsOverTimeLast30
        , new mmTreeItemData("Where the Money Goes - Last 30 Days"
        , new mmReportCategoryExpensesGoesLast30Days()));

    wxTreeItemId categsOverTimeLastYear = navTreeCtrl_->AppendItem(categsOverTime, _("Last Year"), 4, 4);
    navTreeCtrl_->SetItemData(categsOverTimeLastYear
        , new mmTreeItemData("Where the Money Goes - Last Year"
        , new mmReportCategoryExpensesGoesLastYear()));

    if (ignoreFuture)
    {
        wxTreeItemId categsOverTimeCurrentYear = navTreeCtrl_->AppendItem(categsOverTime
            , _("Current Year to Date"), 4, 4);
        navTreeCtrl_->SetItemData(categsOverTimeCurrentYear
            , new mmTreeItemData("Where the Money Goes - Current Year"
            , new mmReportCategoryExpensesGoesCurrentYearToDate()));
    }
    else
    {
        wxTreeItemId categsOverTimeCurrentYear = navTreeCtrl_->AppendItem(categsOverTime
            , _("Current Year"), 4, 4);
        navTreeCtrl_->SetItemData(categsOverTimeCurrentYear
            , new mmTreeItemData("Where the Money Goes - Current Year"
            , new mmReportCategoryExpensesGoesCurrentYear()));
    }

    int day = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_DAY", 1);
    int month = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_MONTH", 7);
    if (financialYearIsDifferent())
    {
        wxTreeItemId categsOverTimeLastFinancialYear = navTreeCtrl_->AppendItem(categsOverTime
            , _("Last Financial Year"), 4, 4);
        navTreeCtrl_->SetItemData(categsOverTimeLastFinancialYear
            , new mmTreeItemData("Where the Money Goes - Last Financial Year"
            , new mmReportCategoryExpensesGoesLastFinancialYear(day, month)));

        if (ignoreFuture)
        {
            wxTreeItemId categsOverTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(categsOverTime
                , _("Current Financial Year to Date"), 4, 4);
            navTreeCtrl_->SetItemData(categsOverTimeCurrentFinancialYear
                , new mmTreeItemData("Where the Money Goes - Current Financial Year"
                , new mmReportCategoryExpensesGoesCurrentFinancialYearToDate(day, month)));
        }
        else
        {
            wxTreeItemId categsOverTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(categsOverTime
                , _("Current Financial Year"), 4, 4);
            navTreeCtrl_->SetItemData(categsOverTimeCurrentFinancialYear
                , new mmTreeItemData("Where the Money Goes - Current Financial Year"
                , new mmReportCategoryExpensesGoesCurrentFinancialYear(day, month)));
        }
    }
    ///////////////////////////////////////////////////////////

    wxTreeItemId posCategs = navTreeCtrl_->AppendItem(reports
        , _("Where the Money Comes From"), 4, 4);
    navTreeCtrl_->SetItemData(posCategs
        , new mmTreeItemData("Where the Money Comes From"
        , new mmReportCategoryExpensesComes()));

    wxTreeItemId posCategsCalMonth = navTreeCtrl_->AppendItem(posCategs
        , _("Last Calendar Month"), 4, 4);
    navTreeCtrl_->SetItemData(posCategsCalMonth
        , new mmTreeItemData("Where the Money Comes From - Last Calendar Month"
        , new mmReportCategoryExpensesComesLastMonth()));

    if (ignoreFuture)
    {
        wxTreeItemId posCategsCurrentMonth = navTreeCtrl_->AppendItem(posCategs
            , _("Current Month to Date"), 4, 4);
        navTreeCtrl_->SetItemData(posCategsCurrentMonth
            , new mmTreeItemData("Where the Money Comes From - Current Month"
            , new mmReportCategoryExpensesComesCurrentMonthToDate()));
    }
    else
    {
        wxTreeItemId posCategsCurrentMonth = navTreeCtrl_->AppendItem(posCategs
            , _("Current Month"), 4, 4);
        navTreeCtrl_->SetItemData(posCategsCurrentMonth
            , new mmTreeItemData("Where the Money Comes From - Current Month"
            , new mmReportCategoryExpensesComesCurrentMonth()));
    }

    wxTreeItemId posCategsTimeLast30 = navTreeCtrl_->AppendItem(posCategs
        , _("Last 30 Days"), 4, 4);
    navTreeCtrl_->SetItemData(posCategsTimeLast30
        , new mmTreeItemData("Where the Money Comes From - Last 30 Days"
        , new mmReportCategoryExpensesComesLast30Days()));

    wxTreeItemId posCategsTimeLastYear = navTreeCtrl_->AppendItem(posCategs
        , _("Last Year"), 4, 4);
    navTreeCtrl_->SetItemData(posCategsTimeLastYear
        , new mmTreeItemData("Where the Money Comes From - Last Year"
        , new mmReportCategoryExpensesComesLastYear()));

    if (ignoreFuture)
    {
        wxTreeItemId posCategsTimeCurrentYear = navTreeCtrl_->AppendItem(posCategs
            , _("Current Year to Date"), 4, 4);
        navTreeCtrl_->SetItemData(posCategsTimeCurrentYear
            , new mmTreeItemData("Where the Money Comes From - Current Year"
            , new mmReportCategoryExpensesComesCurrentYearToDate()));
    }
    else
    {
        wxTreeItemId posCategsTimeCurrentYear = navTreeCtrl_->AppendItem(posCategs
            , _("Current Year"), 4, 4);
        navTreeCtrl_->SetItemData(posCategsTimeCurrentYear
            , new mmTreeItemData("Where the Money Comes From - Current Year"
            , new mmReportCategoryExpensesComesCurrentYear()));
    }

    if (financialYearIsDifferent())
    {
        wxTreeItemId posCategsTimeLastFinancialYear = navTreeCtrl_->AppendItem(posCategs
            , _("Last Financial Year"), 4, 4);
        navTreeCtrl_->SetItemData(posCategsTimeLastFinancialYear
            , new mmTreeItemData("Where the Money Comes From - Last Financial Year"
            , new mmReportCategoryExpensesComesLastFinancialYear(day, month)));

        if (ignoreFuture)
        {
            wxTreeItemId posCategsTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(posCategs
                , _("Current Financial Year to Date"), 4, 4);
            navTreeCtrl_->SetItemData(posCategsTimeCurrentFinancialYear
                , new mmTreeItemData("Where the Money Comes From - Current Financial Year"
                , new mmReportCategoryExpensesComesCurrentFinancialYearToDate(day, month)));
        }
        else
        {
            wxTreeItemId posCategsTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(posCategs
                , _("Current Financial Year"), 4, 4);
            navTreeCtrl_->SetItemData(posCategsTimeCurrentFinancialYear
                , new mmTreeItemData("Where the Money Comes From - Current Financial Year"
                , new mmReportCategoryExpensesComesCurrentFinancialYear(day, month)));
        }
    }
    ///////////////////////////////////////////////////////////

    wxTreeItemId categs = navTreeCtrl_->AppendItem(reports, _("Categories"), 4, 4);
    navTreeCtrl_->SetItemData(categs
        , new mmTreeItemData("Categories - Over Time"
        , new mmReportCategoryOverTimePerformance()));

    wxTreeItemId categsCalMonth = navTreeCtrl_->AppendItem(categs
        , _("Last Calendar Month"), 4, 4);
    navTreeCtrl_->SetItemData(categsCalMonth
        , new mmTreeItemData("Categories - Last Calendar Month"
        , new mmReportCategoryExpensesCategoriesLastMonth()));

    if (ignoreFuture)
    {
        wxTreeItemId categsCurrentMonth = navTreeCtrl_->AppendItem(categs
            , _("Current Month to Date"), 4, 4);
        navTreeCtrl_->SetItemData(categsCurrentMonth
            , new mmTreeItemData("Categories - Current Month"
            , new mmReportCategoryExpensesCategoriesCurrentMonthToDate()));
    }
    else
    {
        wxTreeItemId categsCurrentMonth = navTreeCtrl_->AppendItem(categs
            , _("Current Month"), 4, 4);
        navTreeCtrl_->SetItemData(categsCurrentMonth
            , new mmTreeItemData("Categories - Current Month"
            , new mmReportCategoryExpensesCategoriesCurrentMonth()));
    }

    wxTreeItemId categsTimeLast30 = navTreeCtrl_->AppendItem(categs, _("Last 30 Days"), 4, 4);
    navTreeCtrl_->SetItemData(categsTimeLast30
        , new mmTreeItemData("Categories - Last 30 Days"
        , new mmReportCategoryExpensesCategoriesLast30Days()));

    wxTreeItemId categsTimeLastYear = navTreeCtrl_->AppendItem(categs, _("Last Year"), 4, 4);
    navTreeCtrl_->SetItemData(categsTimeLastYear
        , new mmTreeItemData("Categories - Last Year"
        , new mmReportCategoryExpensesCategoriesLastYear()));

    if (ignoreFuture)
    {
        wxTreeItemId categsTimeCurrentYear = navTreeCtrl_->AppendItem(categs
            , _("Current Year to Date"), 4, 4);
        navTreeCtrl_->SetItemData(categsTimeCurrentYear
            , new mmTreeItemData("Categories - Current Year"
            , new mmReportCategoryExpensesCategoriesCurrentYearToDate()));
    }
    else
    {
        wxTreeItemId categsTimeCurrentYear = navTreeCtrl_->AppendItem(categs
            , _("Current Year"), 4, 4);
        navTreeCtrl_->SetItemData(categsTimeCurrentYear
            , new mmTreeItemData("Categories - Current Year"
            , new mmReportCategoryExpensesCategoriesCurrentYear()));
    }

    if (financialYearIsDifferent())
    {
        wxTreeItemId categsTimeLastFinancialYear = navTreeCtrl_->AppendItem(categs
            , _("Last Financial Year"), 4, 4);
        navTreeCtrl_->SetItemData(categsTimeLastFinancialYear
            , new mmTreeItemData("Categories - Last Financial Year"
            , new mmReportCategoryExpensesCategoriesLastFinancialYear(day, month)));

        if (ignoreFuture)
        {
            wxTreeItemId categsTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(categs
                , _("Current Financial Year to Date"), 4, 4);
            navTreeCtrl_->SetItemData(categsTimeCurrentFinancialYear
                , new mmTreeItemData("Categories - Current Financial Year"
                , new mmReportCategoryExpensesCategoriesCurrentFinancialYearToDate(day, month)));
        }
        else
        {
            wxTreeItemId categsTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(categs
                , _("Current Financial Year"), 4, 4);
            navTreeCtrl_->SetItemData(categsTimeCurrentFinancialYear
                , new mmTreeItemData("Categories - Current Financial Year"
                , new mmReportCategoryExpensesCategoriesCurrentFinancialYear(day, month)));
        }
    }
    ///////////////////////////////////////////////////////////

    wxTreeItemId payeesOverTime = navTreeCtrl_->AppendItem(reports, _("Payees"), 4, 4);
    navTreeCtrl_->SetItemData(payeesOverTime
        , new mmTreeItemData("Payee Report"
        , new mmReportPayeeExpenses()));

    wxTreeItemId payeesOverTimeCalMonth = navTreeCtrl_->AppendItem(payeesOverTime
        , _("Last Calendar Month"), 4, 4);
    navTreeCtrl_->SetItemData(payeesOverTimeCalMonth
        , new mmTreeItemData("Payees - Last Calendar Month"
        , new mmReportPayeeExpensesLastMonth()));

    if (ignoreFuture)
    {
        wxTreeItemId payeesOverTimeCurrentMonth = navTreeCtrl_->AppendItem(payeesOverTime
            , _("Current Month to Date"), 4, 4);
        navTreeCtrl_->SetItemData(payeesOverTimeCurrentMonth
            , new mmTreeItemData("Payees - Current Month"
            , new mmReportPayeeExpensesCurrentMonthToDate()));
    }
    else
    {
        wxTreeItemId payeesOverTimeCurrentMonth = navTreeCtrl_->AppendItem(payeesOverTime
            , _("Current Month"), 4, 4);
        navTreeCtrl_->SetItemData(payeesOverTimeCurrentMonth
            , new mmTreeItemData("Payees - Current Month"
            , new mmReportPayeeExpensesCurrentMonth()));
    }

    wxTreeItemId payeesOverTimeLast30 = navTreeCtrl_->AppendItem(payeesOverTime
        , _("Last 30 Days"), 4, 4);
    navTreeCtrl_->SetItemData(payeesOverTimeLast30
        , new mmTreeItemData("Payees - Last 30 Days"
        , new mmReportPayeeExpensesLast30Days()));

    wxTreeItemId payeesOverTimeLastYear = navTreeCtrl_->AppendItem(payeesOverTime
        , _("Last Year"), 4, 4);
    navTreeCtrl_->SetItemData(payeesOverTimeLastYear
        , new mmTreeItemData("Payees - Last Year"
        , new mmReportPayeeExpensesLastYear()));

    if (ignoreFuture)
    {
        wxTreeItemId payeesOverTimeCurrentYear = navTreeCtrl_->AppendItem(payeesOverTime
            , _("Current Year to Date"), 4, 4);
        navTreeCtrl_->SetItemData(payeesOverTimeCurrentYear
            , new mmTreeItemData("Payees - Current Year"
            , new mmReportPayeeExpensesCurrentYearToDate()));
    }
    else
    {
        wxTreeItemId payeesOverTimeCurrentYear = navTreeCtrl_->AppendItem(payeesOverTime
            , _("Current Year"), 4, 4);
        navTreeCtrl_->SetItemData(payeesOverTimeCurrentYear
            , new mmTreeItemData("Payees - Current Year"
            , new mmReportPayeeExpensesCurrentYear()));
    }

    if (financialYearIsDifferent())
    {
        wxTreeItemId payeesOverTimeLastFinancialYear = navTreeCtrl_->AppendItem(payeesOverTime
            , _("Last Financial Year"), 4, 4);
        navTreeCtrl_->SetItemData(payeesOverTimeLastFinancialYear
            , new mmTreeItemData("Payees - Last Financial Year"
            , new mmReportPayeeExpensesLastFinancialYear(day, month)));

        if (ignoreFuture)
        {
            wxTreeItemId payeesOverTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(payeesOverTime
                , _("Current Financial Year to Date"), 4, 4);
            navTreeCtrl_->SetItemData(payeesOverTimeCurrentFinancialYear
                , new mmTreeItemData("Payees - Current Financial Year"
                , new mmReportPayeeExpensesCurrentFinancialYearToDate(day, month)));
        }
        else
        {
            wxTreeItemId payeesOverTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(payeesOverTime
                , _("Current Financial Year"), 4, 4);
            navTreeCtrl_->SetItemData(payeesOverTimeCurrentFinancialYear
                , new mmTreeItemData("Payees - Current Financial Year"
                , new mmReportPayeeExpensesCurrentFinancialYear(day, month)));
        }
    }

    ///////////////////////////////////////////////////////////////////

    wxTreeItemId incexpOverTime = navTreeCtrl_->AppendItem(reports, _("Income vs Expenses"), 4, 4);
    navTreeCtrl_->SetItemData(incexpOverTime, new mmTreeItemData("Income vs Expenses"
        , new mmReportIncomeExpensesAllTime()));

    wxTreeItemId incexpOverTimeCalMonth = navTreeCtrl_->AppendItem(incexpOverTime, _("Last Calendar Month"), 4, 4);
    navTreeCtrl_->SetItemData(incexpOverTimeCalMonth
        , new mmTreeItemData("Income vs Expenses - Last Calendar Month"
            , new mmReportIncomeExpensesLastMonth()));

    if (ignoreFuture)
    {
        wxTreeItemId incexpOverTimeCurrentMonth = navTreeCtrl_->AppendItem(incexpOverTime
            , _("Current Month to Date"), 4, 4);
        navTreeCtrl_->SetItemData(incexpOverTimeCurrentMonth
            , new mmTreeItemData("Income vs Expenses - Current Month"
                , new mmReportIncomeExpensesCurrentMonthToDate()));
    }
    else
    {
        wxTreeItemId incexpOverTimeCurrentMonth = navTreeCtrl_->AppendItem(incexpOverTime
            , _("Current Month"), 4, 4);
        navTreeCtrl_->SetItemData(incexpOverTimeCurrentMonth
            , new mmTreeItemData("Income vs Expenses - Current Month"
                    , new mmReportIncomeExpensesCurrentMonth()));
    }

    wxTreeItemId incexpOverTimeLast30 = navTreeCtrl_->AppendItem(incexpOverTime, _("Last 30 Days"), 4, 4);
    navTreeCtrl_->SetItemData(incexpOverTimeLast30
        , new mmTreeItemData("Income vs Expenses - Last 30 Days"
            , new mmReportIncomeExpensesLast30Days()));

    wxTreeItemId incexpOverTimeLastYear = navTreeCtrl_->AppendItem(incexpOverTime, _("Last Year"), 4, 4);
    navTreeCtrl_->SetItemData(incexpOverTimeLastYear
        , new mmTreeItemData("Income vs Expenses - Last Year"
        , new mmReportIncomeExpensesLastYear()));

    if (ignoreFuture)
    {
        wxTreeItemId incexpOverTimeCurrentYear = navTreeCtrl_->AppendItem(incexpOverTime, _("Current Year to Date"), 4, 4);
        navTreeCtrl_->SetItemData(incexpOverTimeCurrentYear
            , new mmTreeItemData("Income vs Expenses - Current Year"
            , new mmReportIncomeExpensesCurrentYearToDate()));
    }
    else
    {
        wxTreeItemId incexpOverTimeCurrentYear = navTreeCtrl_->AppendItem(incexpOverTime, _("Current Year"), 4, 4);
        navTreeCtrl_->SetItemData(incexpOverTimeCurrentYear
            , new mmTreeItemData("Income vs Expenses - Current Year"
            , new mmReportIncomeExpensesCurrentYear()));
    }

    if (financialYearIsDifferent())
    {
        wxTreeItemId incexpOverTimeLastFinancialYear = navTreeCtrl_->AppendItem(incexpOverTime
            , _("Last Financial Year"), 4, 4);
        navTreeCtrl_->SetItemData(incexpOverTimeLastFinancialYear
            , new mmTreeItemData("Income vs Expenses - Last Financial Year"
            , new mmReportIncomeExpensesLastFinancialYear(day, month)));

        if (ignoreFuture)
        {
            wxTreeItemId incexpOverTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(incexpOverTime
                , _("Current Financial Year to Date"), 4, 4);
            navTreeCtrl_->SetItemData(incexpOverTimeCurrentFinancialYear
                , new mmTreeItemData("Income vs Expenses - Current Financial Year to Date"
                , new mmReportIncomeExpensesCurrentFinancialYearToDate(day, month)));
        }
        else
        {
            wxTreeItemId incexpOverTimeCurrentFinancialYear = navTreeCtrl_->AppendItem(incexpOverTime
                , _("Current Financial Year"), 4, 4);
            navTreeCtrl_->SetItemData(incexpOverTimeCurrentFinancialYear
                , new mmTreeItemData("Income vs Expenses - Current Financial Year"
                , new mmReportIncomeExpensesCurrentFinancialYear(day, month)));
        }
    }

    ///////////////////////////////////////////////////////////////////

    wxTreeItemId incexpOverTimeSpecificAccounts = navTreeCtrl_->AppendItem(reports, _("Income vs Expenses - Specific Accounts"), 4, 4);
    navTreeCtrl_->SetItemData(incexpOverTimeSpecificAccounts, new mmTreeItemData("Income vs Expenses - Specific Accounts"
        , new mmReportIncomeExpensesAllTimeSpecificAccounts()));

    wxTreeItemId incexpOverTimeCalMonthSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts, _("Last Calendar Month"), 4, 4);
    navTreeCtrl_->SetItemData(incexpOverTimeCalMonthSpecificAccounts
        , new mmTreeItemData("Income vs Expenses - Last Calendar Month - Specific Accounts"
        , new mmReportIncomeExpensesLastMonthSpecificAccounts()));

    if (ignoreFuture)
    {
        wxTreeItemId incexpOverTimeCurrentMonthSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts
            , _("Current Month to Date"), 4, 4);
        navTreeCtrl_->SetItemData(incexpOverTimeCurrentMonthSpecificAccounts
            , new mmTreeItemData("Income vs Expenses - Current Month - Specific Accounts"
            , new mmReportIncomeExpensesCurrentMonthToDateSpecificAccounts()));
    }
    else
    {
        wxTreeItemId incexpOverTimeCurrentMonthSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts
            , _("Current Month"), 4, 4);
        navTreeCtrl_->SetItemData(incexpOverTimeCurrentMonthSpecificAccounts
            , new mmTreeItemData("Income vs Expenses - Current Month - Specific Accounts"
            , new mmReportIncomeExpensesCurrentMonthSpecificAccounts()));
    }

    wxTreeItemId incexpOverTimeLast30SpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts, _("Last 30 Days"), 4, 4);
    navTreeCtrl_->SetItemData(incexpOverTimeLast30SpecificAccounts
        , new mmTreeItemData("Income vs Expenses - Last 30 Days - Specific Accounts"
        , new mmReportIncomeExpensesLast30DaysSpecificAccounts()));

    wxTreeItemId incexpOverTimeLastYearSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts, _("Last Year"), 4, 4);
    navTreeCtrl_->SetItemData(incexpOverTimeLastYearSpecificAccounts
        , new mmTreeItemData("Income vs Expenses - Last Year - Specific Accounts"
        , new mmReportIncomeExpensesLastYearSpecificAccounts()));

    if (ignoreFuture)
    {
        wxTreeItemId incexpOverTimeCurrentYearSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts, _("Current Year to Date"), 4, 4);
        navTreeCtrl_->SetItemData(incexpOverTimeCurrentYearSpecificAccounts
            , new mmTreeItemData("Income vs Expenses - Current Year - Specific Accounts"
            , new mmReportIncomeExpensesCurrentYearToDateSpecificAccounts()));
    }
    else
    {
        wxTreeItemId incexpOverTimeCurrentYearSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts, _("Current Year"), 4, 4);
        navTreeCtrl_->SetItemData(incexpOverTimeCurrentYearSpecificAccounts
            , new mmTreeItemData("Income vs Expenses - Current Year - Specific Accounts"
            , new mmReportIncomeExpensesCurrentYearSpecificAccounts()));
    }

    if (financialYearIsDifferent())
    {
        wxTreeItemId incexpOverTimeLastFinancialYearSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts
            , _("Last Financial Year"), 4, 4);
        navTreeCtrl_->SetItemData(incexpOverTimeLastFinancialYearSpecificAccounts
            , new mmTreeItemData("Income vs Expenses - Last Financial Year - Specific Accounts"
            , new mmReportIncomeExpensesLastFinancialYearSpecificAccounts(day, month)));

        if (ignoreFuture)
        {
            wxTreeItemId incexpOverTimeCurrentFinancialYearSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts
                , _("Current Financial Year to Date"), 4, 4);
            navTreeCtrl_->SetItemData(incexpOverTimeCurrentFinancialYearSpecificAccounts
                , new mmTreeItemData("Income vs Expenses - Current Financial Year - Specific Accounts"
                , new mmReportIncomeExpensesCurrentFinancialYearToDateSpecificAccounts(day, month)));
        }
        else
        {
            wxTreeItemId incexpOverTimeCurrentFinancialYearSpecificAccounts = navTreeCtrl_->AppendItem(incexpOverTimeSpecificAccounts
                , _("Current Financial Year"), 4, 4);
            navTreeCtrl_->SetItemData(incexpOverTimeCurrentFinancialYearSpecificAccounts
                , new mmTreeItemData("Income vs Expenses - Current Financial Year - Specific Accounts"
                , new mmReportIncomeExpensesCurrentFinancialYearSpecificAccounts(day, month)));
        }
    }

    //////////////////////////////////////////////////////////////////

    wxTreeItemId transactionList = navTreeCtrl_->AppendItem(reports, _("Transaction Report"), 4, 4);
    navTreeCtrl_->SetItemData(transactionList, new mmTreeItemData("Transaction Report"));

    ///////////////////////////////////////////////////////////////////

    if (m_db && mmIniOptions::instance().enableBudget_)
    {
        wxTreeItemId budgetPerformance;
        wxTreeItemId budgetSetupPerformance;

        size_t i = 0;
        for (const auto& e : Model_Budgetyear::instance().all(Model_Budgetyear::COL_BUDGETYEARNAME))
        {
            if (!i)
            { // first loop only
                budgetPerformance = navTreeCtrl_->AppendItem(reports, _("Budget Performance"), 4, 4);
                navTreeCtrl_->SetItemData(budgetPerformance, new mmTreeItemData("Budget Performance"));

                budgetSetupPerformance = navTreeCtrl_->AppendItem(reports, _("Budget Category Summary"), 4, 4);
                navTreeCtrl_->SetItemData(budgetSetupPerformance, new mmTreeItemData("Budget Setup Performance"));
            }

            int id = e.BUDGETYEARID;
            const wxString& name = e.BUDGETYEARNAME;

            wxTreeItemId bYear = navTreeCtrl_->AppendItem(budgeting, name, 3, 3);
            navTreeCtrl_->SetItemData(bYear, new mmTreeItemData(id, true));

            // Only add YEARS for Budget Performance
            if (name.length() < 5)
            {
                wxTreeItemId bYearData = navTreeCtrl_->AppendItem(budgetPerformance, name, 4, 4);
                navTreeCtrl_->SetItemData(bYearData, new mmTreeItemData(id, true));
            }
            wxTreeItemId bYearSetupData = navTreeCtrl_->AppendItem(budgetSetupPerformance, name, 4, 4);
            navTreeCtrl_->SetItemData(bYearSetupData, new mmTreeItemData(id, true));
            ++ i;
        }

        //TODO: Set up as a permanent user option
        if (expandedBudgetingNavTree_)
            navTreeCtrl_->Expand(budgeting);
    }

    ///////////////////////////////////////////////////////////////////
    wxTreeItemId cashFlow = navTreeCtrl_->AppendItem(reports, _("Cash Flow"), 4, 4);
    navTreeCtrl_->SetItemData(cashFlow, new mmTreeItemData("Cash Flow", new mmReportCashFlowAllAccounts()));

    wxTreeItemId cashflowWithBankAccounts = navTreeCtrl_->AppendItem(cashFlow, _("Cash Flow - With Bank Accounts"), 4, 4);
    navTreeCtrl_->SetItemData(cashflowWithBankAccounts, new mmTreeItemData("Cash Flow - With Bank Accounts", new mmReportCashFlowBankAccounts()));

    if ( Model_Account::hasActiveTermAccount() )
    {
        wxTreeItemId cashflowWithTermAccounts = navTreeCtrl_->AppendItem(cashFlow, _("Cash Flow - With Term Accounts"), 4, 4);
        navTreeCtrl_->SetItemData(cashflowWithTermAccounts, new mmTreeItemData("Cash Flow - With Term Accounts", new mmReportCashFlowTermAccounts()));
    }

    wxTreeItemId cashflowSpecificAccounts = navTreeCtrl_->AppendItem(cashFlow, _("Cash Flow - Specific Accounts"), 4, 4);
    navTreeCtrl_->SetItemData(cashflowSpecificAccounts, new mmTreeItemData("Cash Flow - Specific Accounts", new mmReportCashFlowSpecificAccounts()));


    wxTreeItemId cashflowSpecificAccountsDaily = navTreeCtrl_->AppendItem(cashFlow, _("Daily Cash Flow - Specific Accounts"), 4, 4);
    navTreeCtrl_->SetItemData(cashflowSpecificAccountsDaily, new mmTreeItemData("Daily Cash Flow - Specific Accounts", new mmReportDailyCashFlowSpecificAccounts()));

    ///////////////////////////////////////////////////////////////////
    //TODO: Test reports
    Model_Report::Data_Set records = Model_Report::instance().all(Model_Report::COL_GROUPNAME, Model_Report::COL_REPORTNAME);
    wxTreeItemId group;
    wxString group_name;
    for (const auto& record : records)
    {
        bool no_group = record.GROUPNAME.empty();
        if (group_name != record.GROUPNAME && !no_group)
        {
            group = navTreeCtrl_->AppendItem(reports, wxGetTranslation(record.GROUPNAME), 4, 4);
            navTreeCtrl_->SetItemData(group, new mmTreeItemData(record.GROUPNAME, 0));
            group_name = record.GROUPNAME;
        }
        Model_Report::Data* r = Model_Report::instance().get(record.REPORTID);
        wxTreeItemId item = navTreeCtrl_->AppendItem(no_group ? reports : group
            , wxGetTranslation(record.REPORTNAME), 4, 4);
        navTreeCtrl_->SetItemData(item, new mmTreeItemData(r->REPORTNAME, new mmGeneralReport(r)));
    }

    ///////////////////////////////////////////////////////////////////

    wxTreeItemId help = navTreeCtrl_->AppendItem(root, _("Help"), 5, 5);
    navTreeCtrl_->SetItemData(help, new mmTreeItemData(NAVTREECTRL_HELP));
    navTreeCtrl_->SetItemBold(help, true);

     /* Start Populating the dynamic data */
    navTreeCtrl_->Expand(root);
    if (expandedReportNavTree_)
        navTreeCtrl_->Expand(reports);

    if (!m_db)
       return;

    /* Load Nav Tree Control */

    wxString vAccts = Model_Setting::instance().GetStringSetting("VIEWACCOUNTS", VIEW_ACCOUNTS_ALL_STR);
    wxASSERT(vAccts == VIEW_ACCOUNTS_ALL_STR || vAccts == VIEW_ACCOUNTS_FAVORITES_STR || vAccts == VIEW_ACCOUNTS_OPEN_STR);
    if (vAccts != VIEW_ACCOUNTS_ALL_STR && vAccts != VIEW_ACCOUNTS_FAVORITES_STR && vAccts != VIEW_ACCOUNTS_OPEN_STR)
        vAccts = VIEW_ACCOUNTS_ALL_STR;

    for (const auto& account: Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME))
    {
        // Checking/Bank Accounts
        if (Model_Account::type(account) == Model_Account::CHECKING)
        {
            if ((vAccts == "Open" && Model_Account::status(account) == Model_Account::OPEN) ||
                (vAccts == "Favorites" && Model_Account::FAVORITEACCT(account)) ||
                (vAccts == "ALL"))
            {
                int selectedImage = mmIniOptions::instance().account_image_id(account.ACCOUNTID);
                wxTreeItemId tacct = navTreeCtrl_->AppendItem(accounts, account.ACCOUNTNAME, selectedImage, selectedImage);
                navTreeCtrl_->SetItemData(tacct, new mmTreeItemData(account.ACCOUNTID, false));
            }
        }
        // Term Accounts
        else if (Model_Account::type(account) == Model_Account::TERM)
        {
            if ((vAccts == "Open" && Model_Account::status(account) == Model_Account::OPEN) ||
                (vAccts == "Favorites" && Model_Account::FAVORITEACCT(account)) ||
                (vAccts == "ALL"))
            {
                int selectedImage = mmIniOptions::instance().account_image_id(account.ACCOUNTID);
                wxTreeItemId tacct = navTreeCtrl_->AppendItem(termAccount, account.ACCOUNTNAME, selectedImage, selectedImage);
                navTreeCtrl_->SetItemData(tacct, new mmTreeItemData(account.ACCOUNTID, false));
            }
        }
        // Stock Accounts
        else
        {
            if ((vAccts == "Open" && Model_Account::status(account) == Model_Account::OPEN) ||
                (vAccts == "Favorites" && Model_Account::FAVORITEACCT(account)) ||
                (vAccts == "ALL"))
            {
                int selectedImage = mmIniOptions::instance().account_image_id(account.ACCOUNTID);
                wxTreeItemId tacct = navTreeCtrl_->AppendItem(stocks, account.ACCOUNTNAME, selectedImage, selectedImage);
                navTreeCtrl_->SetItemData(tacct, new mmTreeItemData(account.ACCOUNTID, false));
            }
        }
    }

    if (mmIniOptions::instance().expandBankTree_)
        navTreeCtrl_->Expand(accounts);

    if ( Model_Account::hasActiveTermAccount() )
    {
        menuBar_->FindItem(MENU_VIEW_TERMACCOUNTS)->Enable(true);
        if (mmIniOptions::instance().expandTermTree_ || expandTermAccounts)
            navTreeCtrl_->Expand(termAccount);
    } else
        menuBar_->FindItem(MENU_VIEW_TERMACCOUNTS)->Enable(false);

    navTreeCtrl_->SetEvtHandlerEnabled(true);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnTreeItemExpanded(wxTreeEvent& event)
{
    mmTreeItemData* iData =
        dynamic_cast<mmTreeItemData*>(navTreeCtrl_->GetItemData(event.GetItem()));
    if (!iData) return;

    if (iData->getString() == NAVTREECTRL_REPORTS)
        expandedReportNavTree_ = true;
    else if (iData->getString() == NAVTREECTRL_BUDGET)
        expandedBudgetingNavTree_ = true;
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnTreeItemCollapsed(wxTreeEvent& event)
{
    mmTreeItemData* iData =
        dynamic_cast<mmTreeItemData*>(navTreeCtrl_->GetItemData(event.GetItem()));
    if (!iData) return;

    if (iData->getString() == NAVTREECTRL_REPORTS)
        expandedReportNavTree_ = false;
    else if (iData->getString() == NAVTREECTRL_BUDGET)
        expandedBudgetingNavTree_ = false;
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnSelChanged(wxTreeEvent& event)
{
    /* Because Windows generates 2 events when selecting the navTree, and Linux
       does not, we need to be able to control when the event is actually executed.
       This ensures that only one event activates the account for all systems. */
    if (homePageAccountSelect_) return;

    menuPrintingEnable(false);
    wxTreeItemId id = event.GetItem();
    if (!id) return;

    mmTreeItemData* iData = dynamic_cast<mmTreeItemData*>(navTreeCtrl_->GetItemData(id));
    selectedItemData_ = iData;

    if (!iData) return;

    if (!iData->isStringData())
    {
        int data = iData->getData();
        if (iData->isBudgetingNode())
        {
            wxString reportWaitingMsg = _("Budget report being generated... Please wait.");
            int year = data;

            wxTreeItemId idparent = navTreeCtrl_->GetItemParent(id);
            mmTreeItemData* iParentData = dynamic_cast<mmTreeItemData*>(navTreeCtrl_->GetItemData(idparent));
            if (iParentData->getString() == "Budget Performance")
            {
                int year = data;
                mmPrintableBase* rs = new mmReportBudgetingPerformance(year);
                createReportsPage(rs, true);
            }
            else if (iParentData->getString() == "Budget Setup Performance")
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
           Model_Account::Data* account = Model_Account::instance().get(data);
           if (account)
           {
                if (Model_Account::type(account) == Model_Account::CHECKING || Model_Account::type(account) == Model_Account::TERM)
                {
                    gotoAccountID_ = data;
                    if (gotoAccountID_ != -1) createCheckingAccountPage(gotoAccountID_);
                    navTreeCtrl_->SetFocus();
                }
                else
                {
                    wxSizer *sizer = cleanupHomePanel();

                    panelCurrent_ = new mmStocksPanel(data, homePanel_, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
                    sizer->Add(panelCurrent_, 1, wxGROW|wxALL, 1);

                    homePanel_->Layout();
                }
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
        if (iData->getString() == "Home Page")
        {
            createHomePage();
            return;
        }
        else if (iData->getString() == NAVTREECTRL_HELP)
        {
            helpFileIndex_ = mmex::HTML_INDEX;
            createHelpPage();
            return;
        }
        else if (iData->getString() == NAVTREECTRL_INVESTMENT)
        {
            helpFileIndex_ = mmex::HTML_INVESTMENT;
            createHelpPage();
            return;
        }
        else if (iData->getString() == NAVTREECTRL_BUDGET)
        {
            helpFileIndex_ = mmex::HTML_BUDGET;
            createHelpPage();
            return;
        }
        else if (iData->getString() == "Assets")
        {
            wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_ASSETS);
            AddPendingEvent(evt);
            return;
        }
        else if (iData->getString() == "Bills & Deposits")
        {
            wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_BILLSDEPOSITS);
            AddPendingEvent(evt);
            return;
        }

        if (!m_db)
            return;
        //========================================================================

        wxString sData = iData->getString();
        wxString title = wxGetTranslation(sData);

        if (sData == "Transaction Report")
        {
            wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TRANSACTIONREPORT);
            AddPendingEvent(evt);           // Events will be processed in due course.
        }
        else if (sData == "Bills & Deposits")
        {
            wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_BILLSDEPOSITS);
            AddPendingEvent(evt);
        }
        else
        {
            createReportsPage(iData->get_report(), false);
        }
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
                createHomePage();
                updateNavTreeControl();
            }
        }
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
                updateNavTreeControl();
                createHomePage();
            }
        }
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnItemMenu(wxTreeEvent& event)
{
    wxTreeItemId id = event.GetItem();
    if (menuBar_->FindItem(MENU_ORGCATEGS)->IsEnabled() )
        showTreePopupMenu(id, event.GetPoint());
    else
        wxMessageBox(_("MMEX has been opened without an active database."),_("MMEX: Menu Popup Error"), wxOK|wxICON_EXCLAMATION);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnItemRightClick(wxTreeEvent& event)
{
    wxTreeItemId id = event.GetItem();
    navTreeCtrl_ ->SelectItem(id);
}
//----------------------------------------------------------------------------

void mmGUIFrame::showTreePopupMenu(wxTreeItemId id, const wxPoint& pt)
{
    mmTreeItemData* iData = dynamic_cast<mmTreeItemData*>(navTreeCtrl_->GetItemData(id));
    selectedItemData_ = iData;

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
                menu.Append(MENU_TREEPOPUP_EDIT, _("&Edit Account"));
                menu.Append(MENU_TREEPOPUP_DELETE, _("&Delete Account"));
                menu.AppendSeparator();
                menu.Append(MENU_TREEPOPUP_LAUNCHWEBSITE, _("&Launch Account Website"));
                // Enable menu item only if a website exists for the account.
                bool webStatus = !account->WEBSITE.IsEmpty();
                menu.Enable(MENU_TREEPOPUP_LAUNCHWEBSITE, webStatus);

                PopupMenu(&menu, pt);
            }
        }
    }
    else
    {
        if (iData->getString() == NAVTREECTRL_BUDGET)
        {
            wxCommandEvent e;
            OnBudgetSetupDialog(e);
        }
        else
        if (iData->getString() == "Bank Accounts" ||
            iData->getString() == "Term Accounts" ||
            iData->getString() == "Stocks")
        { // Create for Bank Term & Stock Accounts

         //wxMenu menu;
            /*Popup Menu for Bank Accounts*/
        //New Account    //
        //Delete Account //
        //Edit Account   //
        //Export >       //
            //CSV Files //
            //QIF Files //
        //Import >       //
            //CSV Files //
            //QIF Files           //
        //Accounts Visible//
            //All      //
            //Favorite //
            //Open     //

            wxMenu menu;
            menu.Append(MENU_TREEPOPUP_ACCOUNT_NEW, _("New &Account"));
            menu.Append(MENU_TREEPOPUP_ACCOUNT_DELETE, _("&Delete Account"));
            menu.Append(MENU_TREEPOPUP_ACCOUNT_EDIT, _("&Edit Account"));
            menu.Append(MENU_TREEPOPUP_ACCOUNT_LIST, _("Account &List (Home)"));
            menu.AppendSeparator();

            // Create only for Bank Accounts
            if ( (iData->getString() != "Term Accounts") && (iData->getString() != "Stocks") )
            {
                wxMenu *exportTo = new wxMenu;
                exportTo->Append(MENU_TREEPOPUP_ACCOUNT_EXPORT2CSV, _("&CSV Files..."));
                exportTo->Append(MENU_TREEPOPUP_ACCOUNT_EXPORT2QIF, _("&QIF Files..."));
                menu.AppendSubMenu(exportTo,  _("&Export"));
                wxMenu *importFrom = new wxMenu;
                importFrom->Append(MENU_TREEPOPUP_ACCOUNT_IMPORTUNIVCSV, _("&CSV Files..."));
                importFrom->Append(MENU_TREEPOPUP_ACCOUNT_IMPORTQIF, _("&QIF Files..."));
                menu.AppendSubMenu(importFrom,  _("&Import"));
                menu.AppendSeparator();
            }

            wxMenu *viewAccounts = new wxMenu;
            viewAccounts->Append(MENU_TREEPOPUP_ACCOUNT_VIEWALL, _("All"));
            viewAccounts->Append(MENU_TREEPOPUP_ACCOUNT_VIEWOPEN, _("Open"));
            viewAccounts->Append(MENU_TREEPOPUP_ACCOUNT_VIEWFAVORITE, _("Favorites"));
            menu.AppendSubMenu(viewAccounts, _("Accounts Visible"));
            PopupMenu(&menu, pt);
        }
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnViewAllAccounts(wxCommandEvent&)
{
    //Get current settings for view accounts
    wxString vAccts = Model_Setting::instance().GetStringSetting("VIEWACCOUNTS", VIEW_ACCOUNTS_ALL_STR);

    //Set view ALL & Refresh Navigation Panel
    Model_Setting::instance().Set("VIEWACCOUNTS", VIEW_ACCOUNTS_ALL_STR);
    mmGUIFrame::updateNavTreeControl();

    //Restore settings
    Model_Setting::instance().Set("VIEWACCOUNTS", vAccts);
    vAccts = Model_Setting::instance().GetStringSetting("VIEWACCOUNTS", VIEW_ACCOUNTS_ALL_STR);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnViewFavoriteAccounts(wxCommandEvent&)
{
    //Get current settings for view accounts
    wxString vAccts = Model_Setting::instance().GetStringSetting("VIEWACCOUNTS", VIEW_ACCOUNTS_ALL_STR);

    //Set view Favorites & Refresh Navigation Panel
    Model_Setting::instance().Set("VIEWACCOUNTS", VIEW_ACCOUNTS_FAVORITES_STR);
    mmGUIFrame::updateNavTreeControl();

    //Restore settings
    Model_Setting::instance().Set("VIEWACCOUNTS", vAccts);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnViewOpenAccounts(wxCommandEvent&)
{
    //Get current settings for view accounts
    wxString vAccts = Model_Setting::instance().GetStringSetting("VIEWACCOUNTS", VIEW_ACCOUNTS_ALL_STR);

    //Set view Open & Refresh Navigation Panel
    Model_Setting::instance().Set("VIEWACCOUNTS", VIEW_ACCOUNTS_OPEN_STR);
    mmGUIFrame::updateNavTreeControl();

    //Restore settings
    Model_Setting::instance().Set("VIEWACCOUNTS", vAccts);
}
//----------------------------------------------------------------------------

void mmGUIFrame::SetBudgetingPageInactive()
{
    activeBudgetingPage_ = false;
}
//----------------------------------------------------------------------------

void mmGUIFrame::createBudgetingPage(int budgetYearID)
{
    if (activeBudgetingPage_)
    {
        budgetingPage_->DisplayBudgetingDetails(budgetYearID);
    }
    else
    {
        wxSizer *sizer = cleanupHomePanel();

        budgetingPage_ = new mmBudgetingPanel(budgetYearID,
            homePanel_, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
        panelCurrent_ = budgetingPage_;
        activeBudgetingPage_ = true;

        sizer->Add(panelCurrent_, 1, wxGROW|wxALL, 1);
        homePanel_->Layout();
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::createHomePage()
{
# if defined (__WINDOWS__)
    /* On init for Windows system that function start twice. *
     * First time it should be skiped                        */
    if (initHomePage_)
#endif
    {
        wxSizer *sizer = cleanupHomePanel();
        panelCurrent_ = new mmHomePagePanel(
            homePanel_,
            wxID_STATIC,
            wxDefaultPosition,
            wxDefaultSize,
            wxNO_BORDER|wxTAB_TRAVERSAL);

        sizer->Add(panelCurrent_, 1, wxGROW|wxALL, 1);
    }
    homePanel_->Layout();
    refreshRequested_ = false;
    initHomePage_ = true;
}
//----------------------------------------------------------------------------

void mmGUIFrame::createReportsPage(mmPrintableBase* rs, bool cleanup)
{
    if (!rs) return;
    wxSizer *sizer = cleanupHomePanel();

    rs->RefreshData();
    panelCurrent_ = new mmReportsPanel(rs, cleanup, homePanel_, wxID_STATIC,
        wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL);

    sizer->Add(panelCurrent_, 1, wxGROW|wxALL, 1);

    homePanel_->Layout();
    menuPrintingEnable(true);
}
//----------------------------------------------------------------------------

void mmGUIFrame::createHelpPage()
{
    wxSizer *sizer = cleanupHomePanel();

    panelCurrent_ = new mmHelpPanel(homePanel_, wxID_STATIC
        , wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);

    sizer->Add(panelCurrent_, 1, wxGROW|wxALL, 1);

    homePanel_->Layout();
    menuPrintingEnable(true);
}
//----------------------------------------------------------------------------

void mmGUIFrame::createMenu()
{
    wxBitmap toolBarBitmaps[10];
    toolBarBitmaps[0] = wxBitmap(new_xpm);
    toolBarBitmaps[1] = wxBitmap(open_xpm);
    toolBarBitmaps[2] = wxBitmap(save_xpm);
    toolBarBitmaps[3] = wxBitmap(newacct_xpm);
    toolBarBitmaps[4] = wxBitmap(house_xpm);
    toolBarBitmaps[5] = wxBitmap(print_xpm);
    toolBarBitmaps[6] = wxBitmap(printpreview_xpm);
    toolBarBitmaps[7] = wxBitmap(printsetup_xpm);
    toolBarBitmaps[8] = wxBitmap(edit_account_xpm);
    toolBarBitmaps[9] = wxBitmap(delete_account_xpm);

    wxMenu *menu_file = new wxMenu;

    wxMenuItem* menuItemNew = new wxMenuItem(menu_file, MENU_NEW,_("&New Database\tCtrl-N"),_("New Database"));
    menuItemNew->SetBitmap(toolBarBitmaps[0]);
    wxMenuItem* menuItemOpen = new wxMenuItem(menu_file, MENU_OPEN,_("&Open Database\tCtrl-O"),_("Open Database"));
    menuItemOpen->SetBitmap(toolBarBitmaps[1]);
    wxMenuItem* menuItemSaveAs = new wxMenuItem(menu_file, MENU_SAVE_AS,_("Save Database &As"),_("Save Database As"));
    menuItemSaveAs->SetBitmap(wxBitmap(saveas_xpm));
    menu_file->Append(menuItemNew);
    menu_file->Append(menuItemOpen);
    menu_file->Append(menuItemSaveAs);
    menu_file->AppendSeparator();

    menuRecentFiles_ = new wxMenu;
    menu_file->Append(MENU_RECENT_FILES, _("&Recent Files..."), menuRecentFiles_);
    // Note: menuRecentFiles_ will be constructed by the class: RecentDatabaseFiles::setMenuFileItems()
    wxMenuItem* menuClearRecentFiles = new wxMenuItem(menu_file, MENU_RECENT_FILES_CLEAR,_("&Clear Recent Files"));
    menuClearRecentFiles->SetBitmap(wxBitmap(clearlist_xpm));
    menu_file->Append(menuClearRecentFiles);
    menu_file->AppendSeparator();

    wxMenu* exportMenu = new wxMenu;
    exportMenu->Append(MENU_EXPORT_CSV, _("&CSV Files..."), _("Export to CSV"));
    exportMenu->Append(MENU_EXPORT_QIF, _("&QIF Files..."), _("Export to QIF"));
    exportMenu->Append(MENU_EXPORT_HTML, _("&Report to HTML"), _("Export to HTML"));
    menu_file->Append(MENU_EXPORT, _("&Export"), exportMenu);

    wxMenu* importMenu = new wxMenu;
    importMenu->Append(MENU_IMPORT_UNIVCSV, _("&CSV Files...")
        , _("Import from any CSV file"));
    importMenu->Append(MENU_IMPORT_QIF, _("&QIF Files..."), _("Import from QIF"));
    menu_file->Append(MENU_IMPORT, _("&Import"), importMenu);

    menu_file->AppendSeparator();

    wxMenuItem* menuItemPrintSetup = new wxMenuItem(menu_file, wxID_PAGE_SETUP,
        _("Page Set&up..."), _("Setup page printing options"));
    menuItemPrintSetup->SetBitmap(toolBarBitmaps[7]);
    menu_file->Append(menuItemPrintSetup);

    wxMenu* printPreviewMenu = new wxMenu;
    printPreviewMenu->Append(MENU_PRINT_PREVIEW_REPORT,
        _("Current &View"), _("Preview current report"));

    menu_file->Append(wxID_PREVIEW, _("Print Pre&view..."), printPreviewMenu);

    wxMenu* printMenu = new wxMenu;
    wxMenuItem* menuItemPrintView = new wxMenuItem(printMenu, MENU_PRINT_REPORT,
        _("Current &View"), _("Print current report"));
    printMenu->Append(menuItemPrintView);

    menu_file->Append(wxID_PRINT, _("&Print..."), printMenu);

    menu_file->AppendSeparator();

    wxMenuItem* menuItemQuit = new wxMenuItem(menu_file, wxID_EXIT,
        _("E&xit\tAlt-X"), _("Quit this program"));
    menuItemQuit->SetBitmap(wxBitmap(exit_xpm));
    menu_file->Append(menuItemQuit);

    // Create the required menu items
    wxMenu *menuView = new wxMenu;
    wxMenuItem* menuItemToolbar = new wxMenuItem(menuView, MENU_VIEW_TOOLBAR,
        _("&Toolbar"), _("Show/Hide the toolbar"), wxITEM_CHECK);
    wxMenuItem* menuItemLinks = new wxMenuItem(menuView, MENU_VIEW_LINKS,
        _("&Navigation"), _("Show/Hide the Navigation tree control"), wxITEM_CHECK);
    wxMenuItem* menuItemBankAccount = new wxMenuItem(menuView, MENU_VIEW_BANKACCOUNTS,
        _("&Bank Accounts"), _("Show/Hide Bank Accounts on Summary page"), wxITEM_CHECK);
    wxMenuItem* menuItemTermAccount = new wxMenuItem(menuView, MENU_VIEW_TERMACCOUNTS,
        _("Term &Accounts"), _("Show/Hide Term Accounts on Summary page"), wxITEM_CHECK);
    wxMenuItem* menuItemStockAccount = new wxMenuItem(menuView, MENU_VIEW_STOCKACCOUNTS,
        _("&Stock Accounts"), _("Show/Hide Stock Accounts on Summary page"), wxITEM_CHECK);
    wxMenuItem* menuItemBudgetFinancialYears = new wxMenuItem(menuView, MENU_VIEW_BUDGET_FINANCIAL_YEARS,
        _("Budgets: As &Financial Years"), _("Display Budgets in Financial Year Format"), wxITEM_CHECK);
    wxMenuItem* menuItemBudgetTransferTotal = new wxMenuItem(menuView, MENU_VIEW_BUDGET_TRANSFER_TOTAL,
        _("Budgets: &Include Transfers in Totals"), _("Include the transfer transactions in the Budget Totals"), wxITEM_CHECK);
    wxMenuItem* menuItemBudgetSetupWithoutSummary = new wxMenuItem(menuView, MENU_VIEW_BUDGET_SETUP_SUMMARY,
        _("Budget Setup: &Without Summaries"), _("Display the Budget Setup without category summaries"), wxITEM_CHECK);
    wxMenuItem* menuItemBudgetCategorySummary = new wxMenuItem(menuView, MENU_VIEW_BUDGET_CATEGORY_SUMMARY,
        _("Budget Summary: Include &Categories"), _("Include the categories in the Budget Category Summary"), wxITEM_CHECK);
    wxMenuItem* menuItemIgnoreFutureTransactions = new wxMenuItem(menuView, MENU_IGNORE_FUTURE_TRANSACTIONS,
        _("Ignore F&uture Transactions"), _("Ignore Future transactions"), wxITEM_CHECK);

    //Add the menu items to the menu bar
    menuView->Append(menuItemToolbar);
    menuView->Append(menuItemLinks);
    menuView->AppendSeparator();
    menuView->Append(menuItemBankAccount);
    menuView->Append(menuItemTermAccount);
    menuView->Append(menuItemStockAccount);
    menuView->AppendSeparator();

//    wxMenu* budgetingMenu = new wxMenu;
//    budgetingMenu->Append(menuItemBudgetFinancialYears);
//    budgetingMenu->AppendSeparator();
//    budgetingMenu->Append(menuItemBudgetSetupWithoutSummary);
//    budgetingMenu->Append(menuItemBudgetCategorySummary);
//    menuView->AppendSubMenu(budgetingMenu,_("Budget..."));

    menuView->Append(menuItemBudgetFinancialYears);
    menuView->Append(menuItemBudgetTransferTotal);
    menuView->AppendSeparator();
    menuView->Append(menuItemBudgetSetupWithoutSummary);
    menuView->Append(menuItemBudgetCategorySummary);
    menuView->AppendSeparator();
    menuView->Append(menuItemIgnoreFutureTransactions);

    wxMenu *menuAccounts = new wxMenu;

    wxMenuItem* menuItemNewAcct = new wxMenuItem(menuAccounts, MENU_NEWACCT
        , _("New &Account"), _("New Account"));
    menuItemNewAcct->SetBitmap(toolBarBitmaps[3]);
    menuAccounts->Append(menuItemNewAcct);

    wxMenuItem* menuItemAcctList = new wxMenuItem(menuAccounts, MENU_ACCTLIST
        , _("Account &List"), _("Show Account List"));
    menuItemAcctList->SetBitmap(toolBarBitmaps[4]);

    wxMenuItem* menuItemAcctEdit = new wxMenuItem(menuAccounts, MENU_ACCTEDIT
        , _("&Edit Account"), _("Edit Account"));
    menuItemAcctEdit->SetBitmap(toolBarBitmaps[8]);

    wxMenuItem* menuItemAcctDelete = new wxMenuItem(menuAccounts, MENU_ACCTDELETE
        , _("&Delete Account"), _("Delete Account from database"));
    menuItemAcctDelete->SetBitmap(toolBarBitmaps[9]);
    menuAccounts->Append(menuItemAcctDelete);

    menuAccounts->Append(menuItemAcctList);
    menuAccounts->Append(menuItemAcctEdit);

    // Tools Menu
    wxMenu *menuTools = new wxMenu;

    wxMenuItem* menuItemCateg = new wxMenuItem(menuTools
        , MENU_ORGCATEGS, _("Organize &Categories..."), _("Organize Categories"));
    menuItemCateg->SetBitmap(wxBitmap(categoryedit_xpm));
    menuTools->Append(menuItemCateg);

    wxMenuItem* menuItemPayee = new wxMenuItem(menuTools
        , MENU_ORGPAYEE, _("Organize &Payees..."), _("Organize Payees"));
    menuItemPayee->SetBitmap(wxBitmap(user_edit_xpm));
    menuTools->Append(menuItemPayee);

    wxMenuItem* menuItemCurrency = new wxMenuItem(menuTools, MENU_CURRENCY
        , _("Organize Currency..."), _("Organize Currency"));
    menuItemCurrency->SetBitmap(wxBitmap(money_dollar_xpm));
    menuTools->Append(menuItemCurrency);

    wxMenu *menuRelocation = new wxMenu;
    wxMenuItem* menuItemCategoryRelocation = new wxMenuItem(menuRelocation
        , MENU_CATEGORY_RELOCATION, _("&Categories...")
        , _("Reassign all categories to another category"));
    menuItemCategoryRelocation->SetBitmap(wxBitmap(relocate_categories_xpm));
    wxMenuItem* menuItemPayeeRelocation = new wxMenuItem(menuRelocation
        , MENU_PAYEE_RELOCATION, _("&Payees...")
        , _("Reassign all payees to another payee"));
    menuItemPayeeRelocation->SetBitmap(wxBitmap(relocate_payees_xpm));
    menuRelocation->Append(menuItemCategoryRelocation);
    menuRelocation->Append(menuItemPayeeRelocation);
    menuTools->AppendSubMenu(menuRelocation, _("Relocation of...")
        , _("Relocate Categories && Payees"));

    menuTools->AppendSeparator();

    if (mmIniOptions::instance().enableBudget_)
    {
        wxMenuItem* menuItemBudgeting = new wxMenuItem(menuTools, MENU_BUDGETSETUPDIALOG
            , _("&Budget Setup"), _("Budget Setup"));
        menuItemBudgeting->SetBitmap(wxBitmap(calendar_xpm));
        menuTools->Append(menuItemBudgeting);
    }

    if (mmIniOptions::instance().enableRepeatingTransactions_)
    {
        wxMenuItem* menuItemBillsDeposits = new wxMenuItem(menuTools, MENU_BILLSDEPOSITS
            , _("&Repeating Transactions"), _("Bills && Deposits"));
        menuItemBillsDeposits->SetBitmap(wxBitmap(clock_xpm));
        menuTools->Append(menuItemBillsDeposits);
    }

    if (mmIniOptions::instance().enableAssets_)
    {
        wxMenuItem* menuItemAssets = new wxMenuItem(menuTools, MENU_ASSETS
            , _("&Assets"), _("Assets"));
        menuItemAssets->SetBitmap(wxBitmap(car_xpm));
        menuTools->Append(menuItemAssets);
    }

    menuTools->AppendSeparator();

    wxMenuItem* menuItemTransactions = new wxMenuItem(menuTools, MENU_TRANSACTIONREPORT
        , _("&Transaction Report Filter..."), _("Transaction Report Filter"));
    menuItemTransactions->SetBitmap(wxBitmap(filter_xpm));
    menuTools->Append(menuItemTransactions);

    menuTools->AppendSeparator();

    wxMenuItem* menuItemOptions = new wxMenuItem(menuTools, wxID_PREFERENCES
        , _("&Options..."), _("Show the Options Dialog"));
    menuItemOptions->SetBitmap(wxBitmap(wrench_xpm));
    menuTools->Append(menuItemOptions);

    menuTools->AppendSeparator();

    wxMenuItem* menuItemConvertDB = new wxMenuItem(menuTools, MENU_CONVERT_ENC_DB
        , _("Convert Encrypted &DB")
        , _("Convert Encrypted DB to Non-Encrypted DB"));
    menuItemConvertDB->SetBitmap(wxBitmap(encrypt_db_xpm));
    menuTools->Append(menuItemConvertDB);

    // Help Menu
    wxMenu *menuHelp = new wxMenu;

    wxMenuItem* menuItemHelp = new wxMenuItem(menuTools, wxID_HELP,
         _("&Help\tCtrl-F1"), _("Show the Help file"));
    menuItemHelp->SetBitmap(wxBitmap(help_xpm));
    menuHelp->Append(menuItemHelp);

    wxMenuItem* menuItemAppStart = new wxMenuItem(menuTools, MENU_SHOW_APPSTART
        , _("&Show App Start Dialog"), _("App Start Dialog"));
    menuItemAppStart->SetBitmap(wxBitmap(appstart_xpm));
    menuHelp->Append(menuItemAppStart);

    menuHelp->AppendSeparator();

    if (mmIniOptions::instance().enableCheckForUpdates_)
    {
        wxMenuItem* menuItemCheck = new wxMenuItem(menuTools, MENU_CHECKUPDATE
            , _("Check for &Updates"), _("Check For Updates"));
        menuItemCheck->SetBitmap(wxBitmap(checkupdate_xpm));
        menuHelp->Append(menuItemCheck);
    }

    if (mmIniOptions::instance().enableReportIssues_)
    {
        wxMenuItem* menuItemReportIssues = new wxMenuItem(menuTools, MENU_REPORTISSUES
            , _("Visit MMEX Forum")
            , _("Visit the MMEX forum. See existing user comments, or report new issues with the software."));
        menuItemReportIssues->SetBitmap(wxBitmap(issues_xpm));
        menuHelp->Append(menuItemReportIssues);
    }

    if (mmIniOptions::instance().enableBeNotifiedForNewReleases_)
    {
        wxMenuItem* menuItemNotify = new wxMenuItem(menuTools, MENU_ANNOUNCEMENTMAILING
            , _("Register/View Release &Notifications.")
            , _("Sign up to Notification Mailing List or View existing announcements."));
        menuItemNotify->SetBitmap(wxBitmap(notify_xpm));
        menuHelp->Append(menuItemNotify);
    }

    wxMenuItem* menuItemFacebook = new wxMenuItem(menuTools, MENU_FACEBOOK
        , _("Visit us on Facebook"), _("Visit us on Facebook"));
    menuItemFacebook->SetBitmap(wxBitmap(facebook_xpm));
    menuHelp->Append(menuItemFacebook);

    wxMenuItem* menuItemAbout = new wxMenuItem(menuTools, wxID_ABOUT
        , _("&About..."), _("Show about dialog"));
    menuItemAbout->SetBitmap(wxBitmap(about_xpm));
    menuHelp->Append(menuItemAbout);

    menuBar_ = new wxMenuBar;
    menuBar_->Append(menu_file, _("&File"));
    menuBar_->Append(menuAccounts, _("&Accounts"));
    menuBar_->Append(menuTools, _("&Tools"));
    menuBar_->Append(menuView, _("&View"));
    menuBar_->Append(menuHelp, _("&Help"));

    SetMenuBar(menuBar_);

    menuBar_->Check(MENU_VIEW_BANKACCOUNTS, mmIniOptions::instance().expandBankHome_);
    menuBar_->Check(MENU_VIEW_TERMACCOUNTS, mmIniOptions::instance().expandTermHome_);
    menuBar_->Check(MENU_VIEW_STOCKACCOUNTS, mmIniOptions::instance().expandStocksHome_);
    menuBar_->Check(MENU_VIEW_BUDGET_FINANCIAL_YEARS, mmIniOptions::instance().budgetFinancialYears_);
    menuBar_->Check(MENU_VIEW_BUDGET_TRANSFER_TOTAL, mmIniOptions::instance().budgetIncludeTransfers_);
    menuBar_->Check(MENU_VIEW_BUDGET_SETUP_SUMMARY, mmIniOptions::instance().budgetSetupWithoutSummaries_);
    menuBar_->Check(MENU_VIEW_BUDGET_CATEGORY_SUMMARY, mmIniOptions::instance().budgetSummaryWithoutCategories_);
    menuBar_->Check(MENU_IGNORE_FUTURE_TRANSACTIONS, mmIniOptions::instance().ignoreFutureTransactions_);
}
//----------------------------------------------------------------------------

void mmGUIFrame::CreateToolBar()
{
    long style = wxTB_FLAT | wxTB_NODIVIDER;
    toolBar_ = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, style, "ToolBar");

    toolBar_->AddTool(MENU_NEW, _("New"), wxBitmap(new_xpm), _("New Database"));
    toolBar_->AddTool(MENU_OPEN, _("Open"), wxBitmap(open_xpm), _("Open Database"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(MENU_NEWACCT, _("New Account"), wxBitmap(newacct_xpm), _("New Account"));
    toolBar_->AddTool(MENU_ACCTLIST, _("Account List"), wxBitmap(house_xpm), _("Show Account List"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(MENU_ORGCATEGS, _("Organize Categories"), wxBitmap(categoryedit_xpm), _("Show Organize Categories Dialog"));
    toolBar_->AddTool(MENU_ORGPAYEE, _("Organize Payees"), wxBitmap(user_edit_xpm), _("Show Organize Payees Dialog"));
    toolBar_->AddTool(MENU_CURRENCY, _("Organize Currency"), wxBitmap(money_dollar_xpm), _("Show Organize Currency Dialog"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(MENU_TRANSACTIONREPORT, _("Transaction Report Filter"), wxBitmap(filter_xpm), _("Transaction Report Filter"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(wxID_VIEW_LIST, _("General Report Manager"), wxBitmap(customsql_xpm), _("General Report Manager"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(wxID_PREFERENCES, _("&Options..."), wxBitmap(wrench_xpm), _("Show the Options Dialog"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(wxID_NEW, _("New"), wxBitmap(new_transaction_xpm), _("New Transaction"));
    toolBar_->AddSeparator();
    toolBar_->AddTool(wxID_ABOUT, _("&About..."), wxBitmap(about_xpm), _("Show about dialog"));
    toolBar_->AddTool(wxID_HELP, _("&Help\tCtrl-F1"), wxBitmap(help_xpm), _("Show the Help file"));

    // after adding the buttons to the toolbar, must call Realize() to reflect changes
    toolBar_->Realize();
}
//----------------------------------------------------------------------------

void mmGUIFrame::InitializeModelTables()
{
    Model_Infotable::instance(m_db.get());
    Model_Asset::instance(m_db.get());
    Model_Stock::instance(m_db.get());
    Model_Account::instance(m_db.get());
    Model_Payee::instance(m_db.get());
    Model_Checking::instance(m_db.get());
    Model_Currency::instance(m_db.get());
    Model_Budgetyear::instance(m_db.get());
    Model_Subcategory::instance(m_db.get()); // subcategory must be initialized before category
    Model_Category::instance(m_db.get());
    Model_Billsdeposits::instance(m_db.get());
    Model_Splittransaction::instance(m_db.get());
    Model_Budgetsplittransaction::instance(m_db.get());
    Model_Budget::instance(m_db.get());
    Model_Report::instance(m_db.get());
}

bool mmGUIFrame::createDataStore(const wxString& fileName, const wxString& pwd, bool openingNew)
{
    if (m_db)
    {
        ShutdownDatabase();
        /// Update the database according to user requirements
        if (mmOptions::instance().databaseUpdated_ &&
            Model_Setting::instance().GetBoolSetting("BACKUPDB_UPDATE", false))
        {
            BackupDatabase(fileName_, true);
            mmOptions::instance().databaseUpdated_ = false;
        }
    }

    wxFileName checkExt(fileName);
    wxString password;
    bool passwordCheckPassed = true;
    if (checkExt.GetExt().Lower() == "emb" && wxFileName::FileExists(fileName))
    {
        password = !pwd.empty() ? pwd : wxGetPasswordFromUser(_("Enter database's password"));
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
            BackupDatabase(fileName);
        }

        m_db = mmDBWrapper::Open(fileName, password);
        // if the database pointer has been reset, the password is possibly incorrect
        if (!m_db) return false;

        m_commit_callback_hook = new CommitCallbackHook();
        m_db->SetCommitHook(m_commit_callback_hook);
        InitializeModelTables();

        // we need to check the db whether it is the right version
        if (!Model_Infotable::instance().checkDBVersion())
        {
            wxString note = mmex::getProgramName() + _(" - No File opened ");
            this->SetTitle(note);
            wxMessageBox(_("Sorry. The Database version is too old or Database password is incorrect")
                        , dialogErrorMessageHeading
                        , wxOK|wxICON_EXCLAMATION);

            ShutdownDatabase();
            return false;
        }

        password_ = password;
    }
    else if (openingNew) // New Database
    {
        // Remove file so we can replace it instead of opening it
        if (wxFileName::FileExists(fileName))
            wxRemoveFile(fileName);

        m_db = mmDBWrapper::Open(fileName, password);
        m_commit_callback_hook = new CommitCallbackHook();
        m_db->SetCommitHook(m_commit_callback_hook);

        password_ = password;
        InitializeModelTables();

        SetDataBaseParameters(fileName);

        mmNewDatabaseWizard* wizard = new mmNewDatabaseWizard(this);
        wizard->CenterOnParent();
        wizard->RunIt(true);

        /* Jump to new account creation screen */
        wxCommandEvent evt;
        OnNewAccount(evt);
        return true;
    }
    else // open of existing database failed
    {
        wxString note = mmex::getProgramName() + _(" - No File opened ");
        this->SetTitle(note);

        wxString msgStr = _("Cannot locate previously opened database.\n");
        if (!passwordCheckPassed)
            msgStr = _("Password not entered for encrypted Database.\n");

        msgStr << fileName;
        wxMessageBox(msgStr, dialogErrorMessageHeading, wxOK|wxICON_ERROR);
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
        fileName_ = fileName;
        /* Set InfoTable Options into memory */
        mmOptions::instance().LoadInfotableOptions();
    }
    else
    {
        fileName_.Clear();
        password_.Clear();
    }
}
//----------------------------------------------------------------------------

bool mmGUIFrame::openFile(const wxString& fileName, bool openingNew, const wxString &password)
{
    if (createDataStore(fileName, password, openingNew))
    {
        menuEnableItems(true);
        menuPrintingEnable(false);
        autoRepeatTransactionsTimer_.Start(REPEAT_TRANS_DELAY_TIME, wxTIMER_ONE_SHOT);

        updateNavTreeControl();
        initHomePage_ = true;

        if (!refreshRequested_)
        {
            refreshRequested_ = true;
            /* Currency Options might have changed so refresh */
            wxCommandEvent ev(wxEVT_COMMAND_MENU_SELECTED, MENU_ACCTLIST);
            GetEventHandler()->AddPendingEvent(ev);
        }
    }
    else
    {
        return false;
    }

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

    if(dlg.ShowModal() != wxID_OK)
        return;

    wxString fileName = dlg.GetPath();

    if (!fileName.EndsWith(".mmb"))
        fileName += ".mmb";

    SetDatabaseFile(fileName, true);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnOpen(wxCommandEvent& /*event*/)
{
    autoRepeatTransactionsTimer_.Stop();
    wxString fileName = wxFileSelector( _("Choose database file to open")
                                       , wxEmptyString, wxEmptyString, wxEmptyString
                                       , "MMB Files(*.mmb)|*.mmb|Encrypted MMB files (*.emb)|*.emb"
                                       , wxFD_FILE_MUST_EXIST|wxFD_OPEN
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
    wxString encFileName = wxFileSelector( _("Choose Encrypted database file to open")
                                          , wxEmptyString, wxEmptyString, wxEmptyString
                                          , "Encrypted MMB files (*.emb)|*.emb"
                                          , wxFD_FILE_MUST_EXIST
                                          , this
                                         );

    if (encFileName.empty())
        return;

    wxString password = wxGetPasswordFromUser(_("Enter password for database"));
    if (password.empty())
        return;

    wxFileDialog dlg( this
                     , _("Choose database file to Save As")
                     , wxEmptyString
                     , wxEmptyString
                     , "MMB Files(*.mmb)|*.mmb"
                     , wxFD_SAVE | wxFD_OVERWRITE_PROMPT
                    );

    if(dlg.ShowModal() != wxID_OK)
        return;

    wxString fileName = dlg.GetPath();

    if (!dlg.GetPath().EndsWith(".mmb"))
        fileName += ".mmb";

    wxCopyFile(encFileName, fileName);

    wxSQLite3Database db;
    db.Open(fileName, password);
    db.ReKey(wxEmptyString);
    db.Close();

    mmShowErrorMessage(this, _("Converted DB!"), _("MMEX message"));
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnSaveAs(wxCommandEvent& /*event*/)
{
    wxASSERT(m_db);

    if (fileName_.empty())
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
    if (!activeHomePage_) createHomePage();   // Display Home page when not being displayed.

    bool encrypt = dlg.GetFilterIndex() != 0; // emb -> Encrypted mMB
    wxFileName newFileName(dlg.GetPath());
    wxString ext = encrypt ? "emb" : "mmb";
    if (newFileName.GetExt().Lower() != ext) newFileName.SetExt(ext);

    wxFileName oldFileName(fileName_); // opened db's file

    if (newFileName == oldFileName) // on case-sensitive FS uses case-sensitive comparison
    {
        wxMessageDialog dlg(this, _("Can't copy file to itself"), _("Save database file as"), wxOK|wxICON_WARNING);
        dlg.ShowModal();
        return;
    }

    // prepare to copy
    wxString new_password;
    bool rekey = encrypt ^ m_db->IsEncrypted();

    if (encrypt)
    {
        if (rekey)
        {
            new_password = wxGetPasswordFromUser(_("Enter password for new database"));
            if (new_password.empty())
                return;
        }
        else
        {
            new_password = password_;
        }
    }

    // copying db
    ShutdownDatabase(); // database must be closed before copying its file

    if (!wxCopyFile(oldFileName.GetFullPath(), newFileName.GetFullPath(), true))  // true -> overwrite if file exists
        return;

    if (rekey) // encrypt or reset encryption
    {
        wxSQLite3Database dbx;
        dbx.Open(newFileName.GetFullPath(), password_);
        dbx.ReKey(new_password); // empty password resets encryption
        dbx.Close();
    }

    password_.clear();
    openFile(newFileName.GetFullPath(), false, new_password);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnExportToCSV(wxCommandEvent& /*event*/)
{
    mmUnivCSVDialog(this, false).ShowModal();
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
    refreshRequested_ = true;
    updateNavTreeControl();
    if (account_id > 0)
    {
        setGotoAccountID(account_id, -1);
        Model_Account::Data* account = Model_Account::instance().get(account_id);
        setAccountNavTreeSection(account->ACCOUNTNAME);
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_GOTOACCOUNT);
        this->GetEventHandler()->AddPendingEvent(evt);
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnImportUniversalCSV(wxCommandEvent& /*event*/)
{
    if (Model_Account::instance().all().empty())
    {
        wxMessageBox(_("No account available to import"),_("Universal CSV Import"), wxOK|wxICON_WARNING );
        return;
    }

    mmUnivCSVDialog univCSVDialog(this);
    univCSVDialog.ShowModal();
    if (univCSVDialog.InportCompletedSuccessfully())
    {
        Model_Account::Data* account = Model_Account::instance().get(univCSVDialog.ImportedAccountID());
        setAccountNavTreeSection(account->ACCOUNTNAME);
        createCheckingAccountPage(univCSVDialog.ImportedAccountID());
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    autoRepeatTransactionsTimer_.Stop();
//    this->Destroy();
    Close(TRUE);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnNewAccount(wxCommandEvent& /*event*/)
{
    mmAddAccountWizard* wizard = new mmAddAccountWizard(this);
    wizard->CenterOnParent();
    wizard->RunIt(true);

    if (wizard->acctID_ != -1)
    {
        bool firstTermAccount = !Model_Account::hasActiveTermAccount();
        Model_Account::Data* account = Model_Account::instance().get(wizard->acctID_);
        mmNewAcctDialog dlg(account, this);
        dlg.ShowModal();
        if (dlg.termAccountActivated() )
        {
            updateNavTreeControl(true);
            menuBar_->FindItem(MENU_VIEW_TERMACCOUNTS)->Check(true);
            if (firstTermAccount)
            {
            /***************Message to display *************************
                Term Account views have been temporarly turned on.
                To maintain this view, change the defaults by using:

                Tools -> Options
                View Options

                This message will not be displayed in future.
            ************************************************************/
                wxString msgStr;
                msgStr << _("Term Account views have been temporarly turned on.") << "\n"
                       << _("To maintain this view, change the defaults by using:\n\nTools -> Options\nView Options")
                       << "\n\n"
                       << _("This message will not be displayed in future.");
                wxMessageBox(msgStr, _("Initial Term Account Activation"), wxOK|wxICON_INFORMATION);
            }
        }
        else
        {
            updateNavTreeControl();
        }
     }

    if (!refreshRequested_)
    {
        refreshRequested_ = true;
        /* Currency Options might have changed so refresh */
        wxCommandEvent ev(wxEVT_COMMAND_MENU_SELECTED, MENU_ACCTLIST);
        GetEventHandler()->AddPendingEvent(ev);
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnAccountList(wxCommandEvent& /*event*/)
{
    createHomePage();
    homePageAccountSelect_ = true;
    navTreeCtrl_->SelectItem(navTreeCtrl_->GetRootItem());
    homePageAccountSelect_ = false;
}
//----------------------------------------------------------------------------

void mmGUIFrame::refreshPanelData(bool catUpdate)
{
    if (activeHomePage_)
    {
        createHomePage();
    }
    else if (activeCheckingAccountPage_)
    {
        checkingAccountPage_->RefreshList();
    }
    else
    {
        mmBillsDepositsPanel* billsdeposits_panel = dynamic_cast<mmBillsDepositsPanel*>(panelCurrent_);
        if (billsdeposits_panel)
        {
            billsdeposits_panel->RefreshList();
        }
        else if (catUpdate && activeBudgetingPage_)
        {
            budgetingPage_->RefreshList();
        }
    }
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
    mmPayeeDialog dlg(this);
    dlg.ShowModal();
    if (dlg.getRefreshRequested())
    {
        refreshPanelData(false);
    }
    createHomePage();
    updateNavTreeControl();
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
            refreshRequested_ = true;
            Model_Account::Data * account = Model_Account::instance().get(gotoAccountID_);
            if (account)
            {
                setAccountNavTreeSection(account->ACCOUNTNAME);
                createCheckingAccountPage(gotoAccountID_);
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
        createHomePage();
        updateNavTreeControl();
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnTransactionReport(wxCommandEvent& /*event*/)
{
    if (!m_db) return;

    if (Model_Account::instance().all().empty()) return;

    Model_Checking::Full_Data_Set trans;

    mmFilterTransactionsDialog* dlg= new mmFilterTransactionsDialog(this);
    if (dlg->ShowModal() == wxID_OK)
    {
        for (const auto& tran: Model_Checking::instance().all())
        {
            int fromAccountID = -1;
            if (dlg->getAmountRangeCheckBoxMin() && tran.TRANSAMOUNT < dlg->getAmountMin())
                    continue; // skip
            if (dlg->getAmountRangeCheckBoxMax() && tran.TRANSAMOUNT > dlg->getAmountMax())
                    continue; // skip

            if (dlg->getAccountCheckBox())
            {
                fromAccountID = dlg->getAccountID();

                if ((tran.ACCOUNTID != fromAccountID) && (tran.TOACCOUNTID != fromAccountID))
                    continue; // skip
            }

            if (dlg->getDateRangeCheckBox())
            {
                if (!Model_Checking::TRANSDATE(tran).IsBetween(dlg->getFromDateCtrl(), dlg->getToDateControl()))
                    continue; // skip
            }

            if (!dlg->checkPayee(tran.PAYEEID))
            {
                    continue; // skip
            }

            if (dlg->getStatusCheckBox())
            {
                if (!dlg->compareStatus(tran.STATUS)) continue; //skip
            }

            if (dlg->getTypeCheckBox())
            {
                // must have account to reference to in order to distinguish between to and from transfers
                bool bSameAccount = true;
                if (dlg->getAccountCheckBox())
                {
                    if (tran.ACCOUNTID != dlg->getAccountID())
                        bSameAccount = false;
                }

                if (!dlg->allowType(tran.TRANSCODE, bSameAccount)) continue;
            }

            if (dlg->getNumberCheckBox())
            {
                const wxString& transNumber = dlg->getNumber().Trim().Lower();
                const wxString& orig = tran.TRANSACTIONNUMBER.Lower();
                if (!orig.Matches(transNumber))
                    continue;
            }

            if (dlg->getNotesCheckBox())
            {
                wxString filter_notes = dlg->getNotes().Trim().Lower();
                wxString trx_notes = tran.NOTES.Lower();

                if (!trx_notes.Matches(filter_notes))
                    continue;
            }

            if (dlg->getCategoryCheckBox())
            {
                if (tran.CATEGID != -1)
                {
                    if (dlg->getCategoryID() != tran.CATEGID)
                        continue; // Skip
                    if (dlg->getSubCategoryID() != tran.SUBCATEGID)
                        continue; // Skip
                }
                else
                {
                    bool bMatching = false;
                    for (const Model_Splittransaction::Data split : Model_Checking::splittransaction(tran))
                    {
                        if (dlg->getCategoryID() != split.CATEGID)
                            continue;
                        if (dlg->getSubCategoryID() != split.SUBCATEGID)
                            continue;

                        bMatching = true;
                        break;
                    }
                    if (!bMatching)
                        continue;
                }
            }

            Model_Checking::Full_Data full_tran(tran);
            Model_Account::Data *account = Model_Account::instance().get(full_tran.ACCOUNTID);
            if (account) full_tran.ACCOUNTNAME = account->ACCOUNTNAME;
            if (Model_Checking::TRANSFER == Model_Checking::type(tran))
            {
                bool transfer_to = (fromAccountID < 0 || full_tran.TOACCOUNTID == fromAccountID);
                account = Model_Account::instance().get(transfer_to
                    ? full_tran.TOACCOUNTID : full_tran.ACCOUNTID);
                if (account) full_tran.PAYEENAME = account->ACCOUNTNAME;
            }
            else
            {
                const Model_Payee::Data* payee = Model_Payee::instance().get(tran.PAYEEID);
                if (payee) full_tran.PAYEENAME = payee->PAYEENAME;
            }

            if (Model_Checking::splittransaction(tran).empty())
                full_tran.CATEGNAME = Model_Category::instance().full_name(tran.CATEGID, tran.SUBCATEGID);
            else
            {
                for (const auto& entry : Model_Checking::splittransaction(full_tran))
                    full_tran.CATEGNAME += Model_Category::full_name(entry.CATEGID, entry.SUBCATEGID)
                    + " = "
                    + Model_Currency::toString(entry.SPLITTRANSAMOUNT) + "<br>";
            }

            trans.push_back(full_tran);
        }

        mmReportTransactions* rs = new mmReportTransactions(trans, dlg->getAccountID(), dlg);
        createReportsPage(rs, true);
    }
}

void mmGUIFrame::OnGeneralReportManager(wxCommandEvent& /*event*/)
{
    if (!m_db) return;

    mmGeneralReportManager dlg(this);
    dlg.ShowModal();
    updateNavTreeControl();
}

//----------------------------------------------------------------------------

void mmGUIFrame::OnOptions(wxCommandEvent& /*event*/)
{
    if (!m_db.get()) return;

    mmOptionsDialog systemOptions(this);
    if (systemOptions.ShowModal() == wxID_OK && systemOptions.AppliedChanges())
    {
        systemOptions.SaveNewSystemSettings();

        //set the View Menu Option items the same as the options saved.
        menuBar_->FindItem(MENU_VIEW_BANKACCOUNTS)->Check(mmIniOptions::instance().expandBankHome_);
        menuBar_->FindItem(MENU_VIEW_TERMACCOUNTS)->Check(mmIniOptions::instance().expandTermHome_);
        menuBar_->FindItem(MENU_VIEW_STOCKACCOUNTS)->Check(mmIniOptions::instance().expandStocksHome_);
        menuBar_->FindItem(MENU_VIEW_BUDGET_FINANCIAL_YEARS)->Check(mmIniOptions::instance().budgetFinancialYears_);
        menuBar_->FindItem(MENU_VIEW_BUDGET_TRANSFER_TOTAL)->Check(mmIniOptions::instance().budgetIncludeTransfers_);
        menuBar_->FindItem(MENU_VIEW_BUDGET_SETUP_SUMMARY)->Check(mmIniOptions::instance().budgetSetupWithoutSummaries_);
        menuBar_->FindItem(MENU_VIEW_BUDGET_CATEGORY_SUMMARY)->Check(mmIniOptions::instance().budgetSummaryWithoutCategories_);
        menuBar_->FindItem(MENU_IGNORE_FUTURE_TRANSACTIONS)->Check(mmIniOptions::instance().ignoreFutureTransactions_);

        int messageIcon = wxOK|wxICON_INFORMATION;
        wxString sysMsg = wxString() << _("MMEX Options have been updated.") << "\n\n";
        if (systemOptions.RequiresRestart())
        {
            messageIcon = wxOK|wxICON_WARNING;
            sysMsg << _("Recommendation: Shut down and restart MMEX.") << "\n"
                   << _("This will allow all MMEX option updates to take effect.");
        }
        wxMessageBox(sysMsg, _("New MMEX Options"), messageIcon);

        createHomePage();
        updateNavTreeControl();
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnHelp(wxCommandEvent& /*event*/)
{
    helpFileIndex_ = mmex::HTML_INDEX;
    createHelpPage();
}
//----------------------------------------------------------------------------

bool mmGUIFrame::IsUpdateAvailable(const wxString& page)
{
    wxStringTokenizer tkz(page, '.', wxTOKEN_RET_EMPTY_ALL);
    if (tkz.CountTokens() != 4)
    {
        return true;

        //wxString url = mmex::getProgramWebSite();
        //wxLaunchDefaultBrowser(url);
        //return false;
    }

    int maj = wxAtoi(tkz.GetNextToken());
    int min = wxAtoi(tkz.GetNextToken());
    int cust = wxAtoi(tkz.GetNextToken());
    int build = wxAtoi(tkz.GetNextToken());

    // get current version
    wxString currentV = mmex::getProgramVersion();
    currentV = currentV.SubString(0, currentV.Find("DEV") - 1).Trim();
    wxStringTokenizer tkz1(currentV, ('.'), wxTOKEN_RET_EMPTY_ALL);

    int majC = wxAtoi(tkz1.GetNextToken());
    int minC = wxAtoi(tkz1.GetNextToken());
    int custC = wxAtoi(tkz1.GetNextToken());
    int buildC = wxAtoi(tkz1.GetNextToken());

    bool isUpdateAvailable = false;
    if (maj > majC)
        isUpdateAvailable = true;
    else if (maj == majC)
    {
        if (min > minC)
        {
            isUpdateAvailable = true;
        }
        else if (min == minC)
        {
            if (cust > custC)
            {
                isUpdateAvailable = true;
            }
            else if (cust == custC)
            {
                if (build > buildC)
                    isUpdateAvailable = true;
            }
        }
    }

    return isUpdateAvailable;
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnCheckUpdate(wxCommandEvent& /*event*/)
{
    // Set up system information
    wxString versionDetails = wxString()
        << mmex::getTitleProgramVersion() << "\n\n"
        << mmex::getProgramDescription() << "\n"
        << "\n";

    // Access current version details page
    wxString site = mmex::getProgramWebSite() + "/version.html";

    wxString page;
    int err_code = site_content(site, page);
    if (err_code != wxURL_NOERR)
    {
        versionDetails << page;
        wxMessageBox(versionDetails, _("MMEX System Information Check"));
        return;
    }

    /*************************************************************************
    Note: To allow larger digit counters and maintain backward compatability,
    the leading counters before the character [ is ignored by the version
    checking routines.

    Expected string format from the internet up to Version: 0.9.9.0
    page = "x.x.x.x - Win: w.w.w.w - Unix: u.u.u.u - Mac: m.m.m.m";
    string length = 53 characters
    **************************************************************************/
    //  Included for future testing
    //  Old format of counters
    //  page = "x.x.x.x - Win: w.w.w.w - Unix: u.u.u.u - Mac: m.m.m.m";
    //  page = "9.9.9.9 - Win: 0.9.9.0 - Unix: 0.9.9.0 - Mac: 0.9.9.0";

    //  New format to allow counters greater than 9
    //  page = "9.9.9.9 - Win: 0.9.9.0 - Unix: 0.9.9.0 - Mac: 0.9.9.0 -[ Win: 0.10.9.0 - Unix: 0.9.10.0 - Mac: 0.9.9.10";
    //  page = "9.9.9.9 - Win: 0.9.9.0 - Unix: 0.9.9.0 - Mac: 0.9.9.0 -[ Win: 0.9.9.2 - Unix: 0.9.9.2 - Mac: 0.9.9.2";
    //  page = "9.9.9.9 - Win: 0.9.9.0 - Unix: 0.9.9.0 - Mac: 0.9.9.0 -[ Mac: 0.9.9.3 - Unix: 0.9.9.3 - Win: 0.10.9.3";

    wxStringTokenizer versionTokens(page,("["));
    versionTokens.GetNextToken(); // ignore old counters
    page = versionTokens.GetNextToken(); // substrtute new counters

    page = page.SubString(page.find(wxPlatformInfo::Get().GetOperatingSystemFamilyName().substr(0, 3)), 53);
    wxString current_version = page;
    wxStringTokenizer mySysToken(page, ":");
    mySysToken.GetNextToken().Trim(false).Trim();           // skip Operating System. Already accounted for.
    page = mySysToken.GetNextToken().Trim(false).Trim();    // Get version for OS

    // set up display information.
    int style = wxOK|wxCANCEL;
    if (IsUpdateAvailable(page))
    {
        versionDetails << _("New update available: ") << current_version;
        style = wxICON_EXCLAMATION|style;
    }
    else
    {
        versionDetails << _("You have the latest version installed!");
        style = wxICON_INFORMATION|style;
    }

    wxString urlString = mmex::getProgramWebSite();
    versionDetails << "\n\n" << _("Proceed to website: ") << urlString;
    if (wxMessageBox(versionDetails, _("MMEX System Information Check"), style) == wxOK)
        wxLaunchDefaultBrowser(urlString);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnReportIssues(wxCommandEvent& /*event*/)
{
   wxString url = mmex::getProgramForum();
   wxLaunchDefaultBrowser(url);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnBeNotified(wxCommandEvent& /*event*/)
{
    // New site location
    //  wxString url = "http://groups.google.com/group/mmlist";
    wxString url = "https://groups.google.com/forum/?fromgroups#!forum/mmlist";

    wxLaunchDefaultBrowser(url);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnFacebook(wxCommandEvent& /*event*/)
{
    wxString url = "http://www.facebook.com/pages/Money-Manager-Ex/242286559144586";
    wxLaunchDefaultBrowser(url);
}

//----------------------------------------------------------------------------

void mmGUIFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    mmAboutDialog(this).ShowModal();
}
//----------------------------------------------------------------------------

void mmGUIFrame::restorePrinterValues()
{
    // Startup Default Settings
    int leftMargin      = Model_Setting::instance().GetIntSetting("PRINTER_LEFT_MARGIN", 20);
    int rightMargin     = Model_Setting::instance().GetIntSetting("PRINTER_RIGHT_MARGIN", 10);
    int topMargin       = Model_Setting::instance().GetIntSetting("PRINTER_TOP_MARGIN", 10);
    int bottomMargin    = Model_Setting::instance().GetIntSetting("PRINTER_BOTTOM_MARGIN", 10);
    int pageOrientation = Model_Setting::instance().GetIntSetting("PRINTER_PAGE_ORIENTATION", wxPORTRAIT);
    int paperID         = Model_Setting::instance().GetIntSetting("PRINTER_PAGE_ID", wxPAPER_A4);

    wxPoint topLeft(leftMargin, topMargin);
    wxPoint bottomRight(rightMargin, bottomMargin);

    wxPageSetupDialogData* pinterData = printer_->GetPageSetupData();
    pinterData->SetMarginTopLeft(topLeft);
    pinterData->SetMarginBottomRight(bottomRight);

    wxPrintData* printerData = printer_->GetPrintData();
    printerData->SetOrientation((wxPrintOrientation)pageOrientation);

    printerData->SetPaperId((wxPaperSize) paperID);
}

void mmGUIFrame::OnPrintPageSetup(wxCommandEvent& WXUNUSED(event))
{
    if (printer_)
    {
        printer_->PageSetup();

        wxPageSetupDialogData* printerDialogData = printer_->GetPageSetupData();
        wxPoint topLeft = printerDialogData->GetMarginTopLeft();
        wxPoint bottomRight = printerDialogData->GetMarginBottomRight();

        wxPrintData* printerData = printer_->GetPrintData();
        int pageOrientation = printerData->GetOrientation();
        wxPaperSize paperID = printerData->GetPaperId();

        Model_Setting::instance().Set("PRINTER_LEFT_MARGIN", topLeft.x);
        Model_Setting::instance().Set("PRINTER_RIGHT_MARGIN", bottomRight.x);
        Model_Setting::instance().Set("PRINTER_TOP_MARGIN", topLeft.y);
        Model_Setting::instance().Set("PRINTER_BOTTOM_MARGIN", bottomRight.y);
        Model_Setting::instance().Set("PRINTER_PAGE_ORIENTATION", pageOrientation);
        Model_Setting::instance().Set("PRINTER_PAGE_ID", paperID);
    }
}

//----------------------------------------------------------------------------

void mmGUIFrame::OnPrintPage(bool preview)
{
    if (!printer_) return;

    mmReportsPanel* report_panel = dynamic_cast<mmReportsPanel*>(panelCurrent_);
    if (report_panel)
    {
        if (preview)
            printer_ ->PreviewText(report_panel->getReportText());
        else
            printer_ ->PrintText(report_panel->getReportText());
    }
    else if (activeHomePage_)
    {
        mmHomePagePanel* home_page = dynamic_cast<mmHomePagePanel*>(panelCurrent_);
        if (home_page)
        {
            if (preview)
                printer_ ->PreviewText(home_page->GetHomePageText());
            else
                printer_ ->PrintText(home_page->GetHomePageText());
        }
    }
    else
    {
        mmHelpPanel* help_page = dynamic_cast<mmHelpPanel*>(panelCurrent_);
        if (help_page)
        {
            if (preview)
                printer_ ->PreviewFile(mmex::getPathDoc((mmex::EDocFile)helpFileIndex_));
            else
                printer_ ->PrintFile(mmex::getPathDoc((mmex::EDocFile)helpFileIndex_));
        }
    }
}

void mmGUIFrame::OnPrintPageReport(wxCommandEvent& WXUNUSED(event))
{
    this->OnPrintPage(false);
}

void mmGUIFrame::OnPrintPagePreview(wxCommandEvent& WXUNUSED(event))
{
    this->OnPrintPage(true);
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
    mmReportsPanel* report_panel = dynamic_cast<mmReportsPanel*>(panelCurrent_);
    if (report_panel)
    {
        wxString fileName = wxFileSelector("Choose HTML file to Export",
            wxEmptyString, wxEmptyString, wxEmptyString, "*.html", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if ( !fileName.empty() )
        {
            correctEmptyFileExt("html",fileName);
            wxFileOutputStream output( fileName );
            wxTextOutputStream text( output );
            text << report_panel->getReportText();
            output.Close();
        }
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnBillsDeposits(wxCommandEvent& WXUNUSED(event))
{
    wxSizer *sizer = cleanupHomePanel();

    panelCurrent_ = new mmBillsDepositsPanel(homePanel_, wxID_STATIC,
        wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

    sizer->Add(panelCurrent_, 1, wxGROW|wxALL, 1);

    homePanel_->Layout();
}
//----------------------------------------------------------------------------

void mmGUIFrame::createStocksAccountPage(int accountID)
{
    wxSizer *sizer = cleanupHomePanel();

    panelCurrent_ = new mmStocksPanel(accountID, homePanel_, wxID_STATIC,
                                      wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL
                                      );

    sizer->Add(panelCurrent_, 1, wxGROW|wxALL, 1);
    homePanel_->Layout();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnGotoStocksAccount(wxCommandEvent& WXUNUSED(event))
{
    if (gotoAccountID_ != -1)
        createStocksAccountPage(gotoAccountID_);
}
//----------------------------------------------------------------------------

void mmGUIFrame::SetCheckingAccountPageInactive()
{
    activeCheckingAccountPage_ = false;
}
//----------------------------------------------------------------------------

void mmGUIFrame::createCheckingAccountPage(int accountID)
{
    if (activeCheckingAccountPage_)
    {
        checkingAccountPage_->DisplayAccountDetails(accountID);
    }
    else
    {
        wxSizer *sizer = cleanupHomePanel();

        checkingAccountPage_ = new mmCheckingPanel(
                                   accountID, homePanel_);
        panelCurrent_ = checkingAccountPage_;
        activeCheckingAccountPage_ = true;
        if (gotoTransID_ > 0)
        {
            checkingAccountPage_->SetSelectedTransaction(gotoTransID_);
        }

        sizer->Add(panelCurrent_, 1, wxGROW|wxALL, 1);
        homePanel_->Layout();
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnGotoAccount(wxCommandEvent& WXUNUSED(event))
{
    if (gotoAccountID_ != -1)
    {
        createCheckingAccountPage(gotoAccountID_);
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnAssets(wxCommandEvent& /*event*/)
{
    wxSizer *sizer = cleanupHomePanel();
    panelCurrent_ = new mmAssetsPanel(homePanel_);
    sizer->Add(panelCurrent_, 1, wxGROW|wxALL, 1);
    homePanel_->Layout();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnCurrency(wxCommandEvent& /*event*/)
{
    mmMainCurrencyDialog(this, false).ShowModal();
    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_ACCOUNT_LIST);
    OnAccountList(evt);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnEditAccount(wxCommandEvent& /*event*/)
{
    Model_Account::Data_Set accounts = Model_Account::instance().all();
    if (accounts.empty())
    {
        wxMessageBox(_("No account available to edit!"), _("Accounts"), wxOK|wxICON_WARNING);
        return;
    }

    wxArrayString as;
    wxArrayInt arrAcctID;
    for (const auto& account: accounts)
    {
        as.Add(account.ACCOUNTNAME);
        arrAcctID.Add(account.ACCOUNTID);
    }

    wxSingleChoiceDialog scd(this, _("Choose Account to Edit"), _("Accounts"), as);
    if (scd.ShowModal() == wxID_OK)
    {
        int choice = scd.GetSelection();
        int acctID = arrAcctID[choice];
        Model_Account::Data* account = Model_Account::instance().get(acctID);
        mmNewAcctDialog dlg(account, this);
        if ( dlg.ShowModal() == wxID_OK )
        {
            updateNavTreeControl();
            if (!refreshRequested_)
            {
                refreshRequested_ = true;
                wxCommandEvent ev(wxEVT_COMMAND_MENU_SELECTED, MENU_ACCTLIST);
                GetEventHandler()->AddPendingEvent(ev);
            }
        }
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnDeleteAccount(wxCommandEvent& /*event*/)
{
    if (Model_Account::instance().all().empty())
    {
        wxMessageBox(_("No account available to delete!"), _("Accounts"), wxOK|wxICON_WARNING);
        return;
    }

    wxArrayString as;
    wxArrayInt arrAcctID;

    for (const auto& account: Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME))
    {
        as.Add(account.ACCOUNTNAME);
        arrAcctID.Add(account.ACCOUNTID);
    }

    wxSingleChoiceDialog scd (this, _("Choose Account to Delete"), _("Accounts"), as);
    if (scd.ShowModal() == wxID_OK)
    {
        int acctID = arrAcctID[scd.GetSelection()];

        Model_Account::Data* account = Model_Account::instance().get(acctID);
        wxString deletingAccountName = wxString::Format(
            _("Are you sure you want to delete\n %s account: %s ?")
            , wxGetTranslation(account->ACCOUNTTYPE)
            , account->ACCOUNTNAME);
        wxMessageDialog msgDlg(this, deletingAccountName, _("Confirm Account Deletion"),
            wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION);
        if (msgDlg.ShowModal() == wxID_YES)
        {
            Model_Account::instance().remove(acctID);
        }
    }
    updateNavTreeControl();
    createHomePage();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnViewToolbar(wxCommandEvent &event)
{
    m_mgr.GetPane("toolbar").Show(event.IsChecked());
    m_mgr.Update();
    Model_Setting::instance().Set("SHOWTOOLBAR", event.IsChecked());
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnViewLinks(wxCommandEvent &event)
{
    m_mgr.GetPane("Navigation").Show(event.IsChecked());
    m_mgr.Update();
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnViewToolbarUpdateUI(wxUpdateUIEvent &event)
{
    if(m_mgr.GetPane("toolbar").IsShown())
        event.Check(true);
    else
        event.Check(false);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnViewLinksUpdateUI(wxUpdateUIEvent &event)
{
    if(m_mgr.GetPane("Navigation").IsShown())
        event.Check(true);
    else
        event.Check(false);
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnViewBankAccounts(wxCommandEvent &event)
{
    m_mgr.GetPane("Bank Accounts").Show(event.IsChecked());
    m_mgr.Update();

    if (!refreshRequested_)
    {
        refreshRequested_ = true;
        createHomePage();
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnViewTermAccounts(wxCommandEvent &event)
{
    m_mgr.GetPane("Term Accounts").Show(event.IsChecked());
    m_mgr.Update();

    if (!refreshRequested_)
    {
        refreshRequested_ = true;
        createHomePage();
        updateNavTreeControl(menuBar_->IsChecked(MENU_VIEW_TERMACCOUNTS));
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnViewStockAccounts(wxCommandEvent &event)
{
    m_mgr.GetPane("Stock Accounts").Show(event.IsChecked());
    m_mgr.Update();

    if (!refreshRequested_)
    {
        refreshRequested_ = true;
        createHomePage();
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnIgnoreFutureTransactions(wxCommandEvent &event)
{
    m_mgr.GetPane("Ignore Future Transactions").Show(event.IsChecked());
    mmIniOptions::instance().ignoreFutureTransactions_ = !mmIniOptions::instance().ignoreFutureTransactions_;
    if (!refreshRequested_)
    {
        refreshRequested_ = true;
        createHomePage();
        updateNavTreeControl(menuBar_->IsChecked(MENU_VIEW_TERMACCOUNTS));
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnCategoryRelocation(wxCommandEvent& /*event*/)
{
    relocateCategoryDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxString msgStr;
        msgStr << _("Category Relocation Completed.") << "\n\n"
               << wxString::Format( _("Records have been updated in the database: %i"),
                    dlg.updatedCategoriesCount());
        wxMessageBox(msgStr,_("Category Relocation Result"));
        refreshPanelData();
    }
    homePanel_->Layout();
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
        refreshPanelData(false);
    }
    homePanel_->Layout();
}
//----------------------------------------------------------------------------

wxSizer* mmGUIFrame::cleanupHomePanel(bool new_sizer)
{
    wxASSERT(homePanel_);

    if (panelCurrent_) delete panelCurrent_;
    homePanel_->DestroyChildren();
    homePanel_->SetSizer(new_sizer ? new wxBoxSizer(wxHORIZONTAL) : 0);

    return homePanel_->GetSizer();
}
//----------------------------------------------------------------------------

void mmGUIFrame::SetDatabaseFile(const wxString& dbFileName, bool newDatabase)
{
    autoRepeatTransactionsTimer_.Stop();

    // Ensure database is in a steady state first
    if (m_db && !activeHomePage_)
    {
        refreshRequested_ = true;
        createHomePage();
    }

    if (openFile(dbFileName, newDatabase))
    {
        recentFiles_->updateRecentList(dbFileName);
    }
    else
    {
        createHomePage();
        updateNavTreeControl();
        showBeginAppDialog(true);
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::BackupDatabase(const wxString& filename, bool updateRequired)
{
    wxFileName fn(filename);
    if (!fn.IsOk()) return;

    wxString backupType = "_start_";
    if (updateRequired) backupType = "_update_";

    wxString backupName = filename + backupType + wxDateTime().Today().FormatISODate() + "." + fn.GetExt();
    if (updateRequired) // Create or update the backup file.
    {
        wxCopyFile(filename, backupName, true);
    }
    else                // create the backup if it does not exist
    {
        wxFileName fnBak(backupName);
        if (!fnBak.FileExists())
        {
            wxCopyFile(filename, backupName, true);
        }
    }

    // Get the list of created backup files for the given filename.
    wxArrayString backupFileArray;
    wxString fileSearch = filename + backupType + "*." + fn.GetExt();
    wxString backupFile = wxFindFirstFile(fileSearch);
    while (!backupFile.empty())
    {
        backupFileArray.Add(backupFile);
        backupFile = wxFindNextFile();
    }

    int max =  Model_Setting::instance().GetIntSetting("MAX_BACKUP_FILES", 4);
    if (backupFileArray.Count() > (size_t)max)
    {
        backupFileArray.Sort(true);
        // ensure file is not read only before deleting file.
        wxFileName fnLastFile(backupFileArray.Last());
        if (fnLastFile.IsFileWritable()) wxRemoveFile(backupFileArray.Last());
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnRecentFiles(wxCommandEvent& event)
{
    int fileNum = event.GetId() - wxID_FILE1 + 1;
    wxString file_name = recentFiles_->getRecentFile(fileNum);
    wxFileName file(file_name);
    if (file.FileExists())
        SetDatabaseFile(file_name);
    else
    {
        wxMessageBox(wxString::Format(_("File %s not found"), file_name), _("Error"), wxOK|wxICON_ERROR);
        recentFiles_->removeRecentFile(fileNum);
    }
}
//----------------------------------------------------------------------------

void mmGUIFrame::OnClearRecentFiles(wxCommandEvent& /*event*/)
{
     recentFiles_->clearRecentList();
}

void mmGUIFrame::setGotoAccountID(int account_id, long transID)
{
    gotoAccountID_ = account_id;
    gotoTransID_   = transID;
}
